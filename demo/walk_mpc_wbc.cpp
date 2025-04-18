/*
This is part of OpenLoong Dynamics Control, an open project for the control of biped robot,
Copyright (C) 2024 Humanoid Robot (Shanghai) Co., Ltd, under Apache 2.0.
Feel free to use in any purpose, and cite OpenLoong-Dynamics-Control in any style, to contribute to the advancement of the community.
 <https://atomgit.com/openloong/openloong-dyn-control.git>
 <web@openloong.org.cn>
*/
#include <mujoco/mujoco.h>
#include <GLFW/glfw3.h>
#include "GLFW_callbacks.h"
#include "MJ_interface.h"
#include "PVT_ctrl.h"
#include "data_logger.h"
#include "data_bus.h"
#include "pino_kin_dyn.h"
#include "useful_math.h"
#include "wbc_priority.h"
#include "mpc.h"
#include "gait_scheduler.h"
#include "foot_placement.h"
#include "joystick_interpreter.h"
#include <string>
#include <iostream>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/resource.h>

// 定义与主进程相同的共享内存结构
struct SharedRobotData {
    double simTime;
    double lF_pos[3];
    double lF_acc[3];
    double lF_rpy[3];
    double lF_angular_vel[3];
    double lF_linear_vel[3];
    double hip_joint_pos;
    double knee_joint_pos;
    double Contactforce;
    // ...其他需要的数据
    int dataReady;  // 数据就绪标志
};
// 创建共享内存的函数
SharedRobotData* setupSharedMemory() {
    // 确保键值文件存在
    FILE* fp = fopen("/tmp/openloong_shm_key", "w");
    if (fp) fclose(fp);
    // 创建共享内存
    key_t key = ftok("/tmp/openloong_shm_key", 'R');
    if (key == -1) {
        perror("ftok failed");
        return nullptr;
    }

    // // 检查是否存在旧的共享内存段，如果存在则移除
    // int existing_shmid = shmget(key, 0, 0666);
    // if (existing_shmid >= 0) {
    //     std::cout << "发现已存在的共享内存段，正在移除..." << std::endl;
    //     if (shmctl(existing_shmid, IPC_RMID, NULL) == -1) {
    //         perror("移除共享内存段失败");
    //     } else {
    //         std::cout << "已成功移除旧共享内存段" << std::endl;
    //     }
    // }
    
    size_t shm_size = sizeof(SharedRobotData);
    std::cout << "共享内存大小: " << shm_size << " 字节" << std::endl;
    std::cout << "共享内存键值: " << key << std::endl;

    int shmid = shmget(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
    if (shmid < 0) {
        if (errno == EEXIST) {
            // 共享内存已存在，尝试获取它
            shmid = shmget(key, shm_size, 0666);
            if (shmid < 0) {
                perror("shmget (existing) failed");
                return nullptr;
            }
            std::cout << "使用现有共享内存段: " << shmid << std::endl;
        } else {
            perror("shmget failed");
            std::cout << "错误号: " << errno << std::endl;
            return nullptr;
        }
    } else {
        std::cout << "创建新共享内存段: " << shmid << std::endl;
    }
    
    // 附加共享内存
    SharedRobotData* data = (SharedRobotData*)shmat(shmid, NULL, 0);
    if (data == (void*)-1) {
        perror("shmat failed");
        return nullptr;
    }
    // 初始化数据
    memset(data, 0, shm_size);
    // 初始化数据
    data->dataReady = 0;
    return data;
}

const   double  dt = 0.001;
const   double  dt_200Hz = 0.005;
// MuJoCo load and compile model
char error[1000] = "Could not load binary model";
mjModel* mj_model = mj_loadXML("../models/scene.xml", 0, error, 1000);
mjData* mj_data = mj_makeData(mj_model);

int main(int argc, char **argv) {
    
    // 在main()函数内创建共享内存
    SharedRobotData* sharedData;
    sharedData = setupSharedMemory();

    // initialize classes
    UIctr uiController(mj_model,mj_data);   // UI control for Mujoco
    MJ_Interface mj_interface(mj_model, mj_data); // data interface for Mujoco
    Pin_KinDyn kinDynSolver("../models/AzureLoong.urdf"); // kinematics and dynamics solver
    DataBus RobotState(kinDynSolver.model_nv); // data bus
    WBC_priority WBC_solv(kinDynSolver.model_nv, 18, 22, 0.7, mj_model->opt.timestep); // WBC solver
    MPC MPC_solv(dt_200Hz);  // mpc controller
    GaitScheduler gaitScheduler(0.25, mj_model->opt.timestep); // gait scheduler
    PVT_Ctr pvtCtr(mj_model->opt.timestep,"../common/joint_ctrl_config.json");// PVT joint control
    FootPlacement footPlacement; // foot-placement planner
    JoyStickInterpreter jsInterp(mj_model->opt.timestep); // desired baselink velocity generator
    DataLogger logger("../record/datalog.log"); // data logger

    // initialize UI: GLFW
    uiController.iniGLFW();
    uiController.enableTracking(); // enable viewpoint tracking of the body 1 of the robot
    uiController.createWindow("Demo",false); // NOTE: if the saveVideo is set to true, the raw recorded file could be 2.5 GB for 15 seconds!

    // initialize variables
    double stand_legLength = 1.01;//0.97;// desired baselink height
    double foot_height =0.07; // distance between the foot ankel joint and the bottom
    double xv_des = 0.8;  // desired velocity in x direction
	int model_nv=kinDynSolver.model_nv;

    RobotState.width_hips = 0.229;
    footPlacement.kp_vx = 0.03;
    footPlacement.kp_vy = 0.03;
    footPlacement.kp_wz = 0.03;
    footPlacement.stepHeight = 0.2;
    footPlacement.legLength=stand_legLength;

    mju_copy(mj_data->qpos, mj_model->key_qpos, mj_model->nq*1); // set ini pos in Mujoco

    std::vector<double> motors_pos_des(model_nv - 6, 0);
    std::vector<double> motors_pos_cur(model_nv - 6, 0);
    std::vector<double> motors_vel_des(model_nv - 6, 0);
    std::vector<double> motors_vel_cur(model_nv - 6, 0);
    std::vector<double> motors_tau_des(model_nv - 6, 0);
    std::vector<double> motors_tau_cur(model_nv - 6, 0);

    // ini position and posture for foot-end and hand
    Eigen::Vector3d fe_l_pos_L_des={-0.018, 0.113, -stand_legLength};
    Eigen::Vector3d fe_r_pos_L_des={-0.018, -0.116, -stand_legLength};
    Eigen::Vector3d fe_l_eul_L_des={-0.000, -0.008, -0.000};
    Eigen::Vector3d fe_r_eul_L_des={0.000, -0.008, 0.000};
    Eigen::Matrix3d fe_l_rot_des= eul2Rot(fe_l_eul_L_des(0),fe_l_eul_L_des(1),fe_l_eul_L_des(2));
    Eigen::Matrix3d fe_r_rot_des= eul2Rot(fe_r_eul_L_des(0),fe_r_eul_L_des(1),fe_r_eul_L_des(2));

    Eigen::Vector3d hd_l_pos_L_des={-0.02, 0.32, -0.159};
    Eigen::Vector3d hd_r_pos_L_des={-0.02, -0.32, -0.159};
    Eigen::Vector3d hd_l_eul_L_des={-1.253, 0.122, -1.732};
    Eigen::Vector3d hd_r_eul_L_des={1.253, 0.122, 1.732};
    Eigen::Matrix3d hd_l_rot_des= eul2Rot(hd_l_eul_L_des(0),hd_l_eul_L_des(1),hd_l_eul_L_des(2));
    Eigen::Matrix3d hd_r_rot_des= eul2Rot(hd_r_eul_L_des(0),hd_r_eul_L_des(1),hd_r_eul_L_des(2));

    auto resLeg=kinDynSolver.computeInK_Leg(fe_l_rot_des,fe_l_pos_L_des,fe_r_rot_des,fe_r_pos_L_des);
    auto resHand=kinDynSolver.computeInK_Hand(hd_l_rot_des,hd_l_pos_L_des,hd_r_rot_des,hd_r_pos_L_des);
    Eigen::VectorXd qIniDes=Eigen::VectorXd::Zero(mj_model->nq, 1);
    qIniDes.block(7, 0, mj_model->nq - 7, 1)= resLeg.jointPosRes + resHand.jointPosRes;
    WBC_solv.setQini(qIniDes,RobotState.q);

    // register variable name for data logger
    logger.addIterm("simTime",1);
    //logger.addIterm("motor_pos_des",model_nv - 6);
    logger.addIterm("motor_pos_cur",model_nv - 6);
    //logger.addIterm("motor_vel_des",model_nv - 6);
    logger.addIterm("motor_vel_cur",model_nv - 6);
    //logger.addIterm("motor_tor_des",model_nv - 6);
    //logger.addIterm("rpyVal",3);
    //logger.addIterm("base_omega_W",3);
    //logger.addIterm("gpsVal",3);
    //logger.addIterm("base_vel",3);
	//logger.addIterm("dX_cal",12);
	//logger.addIterm("Ufe",12);
	//logger.addIterm("Xd",12);
	//logger.addIterm("X_cur",12);
	//logger.addIterm("X_cal",12);
    //--添加自己的数据--
    logger.addIterm("lFgpsVal",3);
    logger.addIterm("lFrpyVal",3);
    logger.addIterm("lF_AngVel",3);
    logger.addIterm("lF_vel",3);
    logger.addIterm("lF_acc",3);

    logger.addIterm("rFgpsVal",3);
    logger.addIterm("rFrpyVal",3);
    logger.addIterm("rF_AngVel",3);
    logger.addIterm("rF_vel",3);
    logger.addIterm("rF_acc",3);

    logger.addIterm("lFcontact",4);
    logger.addIterm("rFcontact",4);

    logger.addIterm("lFtouch",1);
    logger.addIterm("rFtouch",1);

    logger.finishItermAdding();

    //// -------------------------- main loop --------------------------------

    int  MPC_count = 0; // count for controlling the mpc running period

    double startSteppingTime=3;
    double startWalkingTime=5;
    double simEndTime=30;

    mjtNum simstart = mj_data->time;
    double simTime = mj_data->time;

    while (!glfwWindowShouldClose(uiController.window)) {
        simstart = mj_data->time;
        while (mj_data->time - simstart < 1.0 / 60.0 && uiController.runSim) { // press "1" to pause and resume, "2" to step the simulation
            mj_step(mj_model, mj_data);
            simTime=mj_data->time;
            // Read the sensors:
            mj_interface.updateSensorValues();
            mj_interface.dataBusWrite(RobotState);

            // update kinematics and dynamics info
            kinDynSolver.dataBusRead(RobotState);
            kinDynSolver.computeJ_dJ();
            kinDynSolver.computeDyn();
            kinDynSolver.dataBusWrite(RobotState);

            // joint number: arm-l: 0-6, arm-r: 7-13, head: 14, waist: 15-17, leg-l: 18-23, leg-r: 24-29

            if (simTime > startWalkingTime) {
                jsInterp.setWzDesLPara(0, 1);
                jsInterp.setVxDesLPara(xv_des, 2.0); // jsInterp.setVxDesLPara(0.9,1);
				RobotState.motionState = DataBus::Walk; // start walking
            } else
                jsInterp.setIniPos(RobotState.q(0), RobotState.q(1), RobotState.base_rpy(2));
            jsInterp.step();
            RobotState.js_pos_des(2) = stand_legLength + foot_height; // pos z is not assigned in jyInterp
            jsInterp.dataBusWrite(RobotState); // only pos x, pos y, theta z, vel x, vel y , omega z are rewrote.

            if (simTime >= startSteppingTime) {
                // gait scheduler
                gaitScheduler.dataBusRead(RobotState);
                gaitScheduler.step();
                gaitScheduler.dataBusWrite(RobotState);

                footPlacement.dataBusRead(RobotState);
                footPlacement.getSwingPos();
                footPlacement.dataBusWrite(RobotState);
            }

            // ------------- MPC ------------
			MPC_count = MPC_count + 1;
            if (MPC_count > (dt_200Hz / dt-1)) {
                MPC_solv.dataBusRead(RobotState);
                MPC_solv.cal();
                MPC_solv.dataBusWrite(RobotState);
                MPC_count = 0;
            }

            // ------------- WBC ------------
            // WBC Calculation
            WBC_solv.dataBusRead(RobotState);
            WBC_solv.computeDdq(kinDynSolver);
            WBC_solv.computeTau();
            WBC_solv.dataBusWrite(RobotState);
            // get the final joint command
            if (simTime <= startSteppingTime) {
                RobotState.motors_pos_des = eigen2std(resLeg.jointPosRes + resHand.jointPosRes);
                RobotState.motors_vel_des = motors_vel_des;
                RobotState.motors_tor_des = motors_tau_des;
            } else {
                MPC_solv.enable();
                Eigen::Matrix<double, 1, nx>  L_diag;
                Eigen::Matrix<double, 1, nu>  K_diag;
                L_diag <<
                        1.0, 1.0, 1.0,//eul
                        1.0, 200.0,  1.0,//pCoM
                        1e-7, 1e-7, 1e-7,//w
                        100.0, 10.0, 1.0;//vCoM
                K_diag <<
                        1.0, 1.0, 1.0,//fl
                        1.0, 1.0, 1.0,
                        1.0, 1.0, 1.0,//fr
                        1.0, 1.0, 1.0,1.0;
                MPC_solv.set_weight(1e-6, L_diag, K_diag);

                Eigen::VectorXd pos_des = kinDynSolver.integrateDIY(RobotState.q, RobotState.wbc_delta_q_final);
                RobotState.motors_pos_des = eigen2std(pos_des.block(7, 0, model_nv - 6, 1));
                RobotState.motors_vel_des = eigen2std(RobotState.wbc_dq_final);
                RobotState.motors_tor_des = eigen2std(RobotState.wbc_tauJointRes);
            }

            // joint PVT controller
            pvtCtr.dataBusRead(RobotState);
            if (simTime <= 3) {
                pvtCtr.calMotorsPVT(100.0 / 1000.0 / 180.0 * 3.1415);
            } else {
                pvtCtr.setJointPD(100,10,"J_ankle_l_pitch");
                pvtCtr.setJointPD(100,10,"J_ankle_l_roll");
                pvtCtr.setJointPD(100,10,"J_ankle_r_pitch");
                pvtCtr.setJointPD(100,10,"J_ankle_r_roll");
                pvtCtr.setJointPD(1000,100,"J_knee_l_pitch");
                pvtCtr.setJointPD(1000,100,"J_knee_r_pitch");
                pvtCtr.calMotorsPVT();
            }
            pvtCtr.dataBusWrite(RobotState);

            // give the joint torque command to Webots
            mj_interface.setMotorsTorque(RobotState.motors_tor_out);

            // print info to the console
//            printf("f_L=[%.3f, %.3f, %.3f]\n", RobotState.fL[0], RobotState.fL[1], RobotState.fL[2]);
//            printf("f_R=[%.3f, %.3f, %.3f]\n", RobotState.fR[0], RobotState.fR[1], RobotState.fR[2]);
//
//            printf("rpyVal=[%.5f, %.5f, %.5f]\n", RobotState.rpy[0], RobotState.rpy[1], RobotState.rpy[2]);
//            printf("basePos=[%.5f, %.5f, %.5f]\n", RobotState.basePos[0], RobotState.basePos[1], RobotState.basePos[2]);

            // data save
            logger.startNewLine();
            logger.recItermData("simTime", simTime);
            //logger.recItermData("motor_pos_des", RobotState.motors_pos_des);
            logger.recItermData("motor_pos_cur", RobotState.motors_pos_cur);
            //logger.recItermData("motor_vel_des", RobotState.motors_vel_des);
            logger.recItermData("motor_vel_cur", RobotState.motors_vel_cur);
            //logger.recItermData("motor_tor_des", RobotState.motors_tor_des);
            //logger.recItermData("rpyVal", RobotState.rpy);
            //logger.recItermData("base_omega_W", RobotState.base_omega_W);
            //logger.recItermData("gpsVal", RobotState.basePos);
            //logger.recItermData("base_vel", RobotState.dq.block<3, 1>(0, 0));
			//logger.recItermData("dX_cal",RobotState.dX_cal);
			//logger.recItermData("Ufe",RobotState.Fr_ff);
			//logger.recItermData("Xd",RobotState.Xd);
			//logger.recItermData("X_cur",RobotState.X_cur);
			//logger.recItermData("X_cal",RobotState.X_cal);
            
            logger.recItermData("lFgpsVal", RobotState.fLPos);
            logger.recItermData("lFrpyVal", RobotState.fLrpy);
            logger.recItermData("lF_AngVel", RobotState.fLAngVel);
            logger.recItermData("lF_vel", RobotState.fLLinVel);
            logger.recItermData("lF_acc", RobotState.fLAcc);

            logger.recItermData("rFgpsVal", RobotState.fRPos);
            logger.recItermData("rFrpyVal", RobotState.fRrpy);
            logger.recItermData("rF_AngVel", RobotState.fRAngVel);
            logger.recItermData("rF_vel", RobotState.fRLinVel);
            logger.recItermData("rF_acc", RobotState.fRAcc);

            logger.recItermData("lFcontact", RobotState.fLcontact);
            logger.recItermData("rFcontact", RobotState.fRcontact);

            logger.recItermData("lFtouch", RobotState.fLtouch);
            logger.recItermData("rFtouch", RobotState.fRtouch);
            
			logger.finishLine();

            // Sharememory update
            if (sharedData) {
                sharedData->simTime = simTime;
                sharedData->lF_pos[0] = RobotState.fLPos[0];
                sharedData->lF_pos[1] = RobotState.fLPos[1];
                sharedData->lF_pos[2] = RobotState.fLPos[2];
                sharedData->lF_acc[0] = RobotState.fLAcc[0];
                sharedData->lF_acc[1] = RobotState.fLAcc[1];
                sharedData->lF_acc[2] = RobotState.fLAcc[2];
                sharedData->lF_rpy[0] = RobotState.fLrpy[0];
                sharedData->lF_rpy[1] = RobotState.fLrpy[1];
                sharedData->lF_rpy[2] = RobotState.fLrpy[2];
                sharedData->lF_angular_vel[0] = RobotState.fLAngVel[0];
                sharedData->lF_angular_vel[1] = RobotState.fLAngVel[1];
                sharedData->lF_angular_vel[2] = RobotState.fLAngVel[2];
                sharedData->lF_linear_vel[0] = RobotState.fLLinVel[0];
                sharedData->lF_linear_vel[1] = RobotState.fLLinVel[1];
                sharedData->lF_linear_vel[2] = RobotState.fLLinVel[2];
                sharedData->hip_joint_pos = RobotState.q(7);
                sharedData->knee_joint_pos = RobotState.q(18);
                sharedData->Contactforce = RobotState.fLtouch;
                // // ...其他需要共享的数据
                sharedData->dataReady = 1;  // 标记数据已就绪
            }

        }

        if (mj_data->time>=simEndTime)
            break;

        uiController.updateScene();
    };
    // free visualization storage
    uiController.Close();

    return 0;
}

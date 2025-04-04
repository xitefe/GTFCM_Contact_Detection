/*
This is part of OpenLoong Dynamics Control, an open project for the control of biped robot,
Copyright (C) 2024 Humanoid Robot (Shanghai) Co., Ltd, under Apache 2.0.
Feel free to use in any purpose, and cite OpenLoong-Dynamics-Control in any style, to contribute to the advancement of the community.
 <https://atomgit.com/openloong/openloong-dyn-control.git>
 <web@openloong.org.cn>
*/
#include "MJ_interface.h"

MJ_Interface::MJ_Interface(mjModel *mj_modelIn, mjData *mj_dataIn) {
    mj_model=mj_modelIn;
    mj_data=mj_dataIn;
    timeStep=mj_model->opt.timestep;
    jointNum=JointName.size();
    jntId_qpos.assign(jointNum,0);
    jntId_qvel.assign(jointNum,0);
    jntId_dctl.assign(jointNum,0);
    motor_pos.assign(jointNum,0);
    motor_vel.assign(jointNum,0);
    motor_pos_Old.assign(jointNum,0);
    for (int i=0;i<jointNum;i++)
    {
        int tmpId= mj_name2id(mj_model,mjOBJ_JOINT,JointName[i].c_str());
        if (tmpId==-1)
        {
            std::cerr <<JointName[i]<< " not found in the XML file!" << std::endl;
            std::terminate();
        }
        jntId_qpos[i]=mj_model->jnt_qposadr[tmpId];
        jntId_qvel[i]=mj_model->jnt_dofadr[tmpId];
        std::string motorName=JointName[i];
        motorName="M"+motorName.substr(1);
        tmpId= mj_name2id(mj_model,mjOBJ_ACTUATOR,motorName.c_str());
        if (tmpId==-1)
        {
            std::cerr <<motorName<< " not found in the XML file!" << std::endl;
            std::terminate();
        }
        jntId_dctl[i]=tmpId;
    }
//    int adr = m->sensor_adr[sensorId];
//    int dim = m->sensor_dim[sensorId];
//    mjtNum sensor_data[dim];
//    mju_copy(sensor_data, &d->sensordata[adr], dim);
    baseBodyId= mj_name2id(mj_model,mjOBJ_BODY, baseName.c_str());
    orientataionSensorId= mj_name2id(mj_model, mjOBJ_SENSOR, orientationSensorName.c_str());
    velSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,velSensorName.c_str());
    gyroSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,gyroSensorName.c_str());
    accSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,accSensorName.c_str());
    //左右脚ID定义
    LfootBodyId= mj_name2id(mj_model,mjOBJ_BODY, LfootName.c_str());
    LfootOrientataionSensorId= mj_name2id(mj_model, mjOBJ_SENSOR, LfootOrientationSensorName.c_str());
    LfootVelSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootVelSensorName.c_str());
    LfootGyroSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootGyroSensorName.c_str());
    LfootAccSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootAccSensorName.c_str());

    RfootBodyId= mj_name2id(mj_model,mjOBJ_BODY, RfootName.c_str());
    RfootOrientataionSensorId= mj_name2id(mj_model, mjOBJ_SENSOR, RfootOrientationSensorName.c_str());
    RfootVelSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootVelSensorName.c_str());
    RfootGyroSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootGyroSensorName.c_str());
    RfootAccSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootAccSensorName.c_str());

    LfootflContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootflContactSensorName.c_str());
    LfootfrContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootfrContactSensorName.c_str());
    LfootblContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootblContactSensorName.c_str());
    LfootbrContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootbrContactSensorName.c_str());

    RfootflContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootflContactSensorName.c_str());
    RfootfrContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootfrContactSensorName.c_str());
    RfootblContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootblContactSensorName.c_str());
    RfootbrContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootbrContactSensorName.c_str());

    LfootContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,LfootContactSensorName.c_str());
    RfootContactSensorId= mj_name2id(mj_model,mjOBJ_SENSOR,RfootContactSensorName.c_str());

}

void MJ_Interface::updateSensorValues() {
    for (int i=0;i<jointNum;i++){
        motor_pos_Old[i]=motor_pos[i];
        motor_pos[i]=mj_data->qpos[jntId_qpos[i]];
        motor_vel[i]=mj_data->qvel[jntId_qvel[i]];
    }
    for (int i=0;i<4;i++)
        baseQuat[i]=mj_data->sensordata[mj_model->sensor_adr[orientataionSensorId]+i];
    double tmp=baseQuat[0];
    baseQuat[0]=baseQuat[1];
    baseQuat[1]=baseQuat[2];
    baseQuat[2]=baseQuat[3];
    baseQuat[3]=tmp;

    rpy[0]= atan2(2*(baseQuat[3]*baseQuat[0]+baseQuat[1]*baseQuat[2]),1-2*(baseQuat[0]*baseQuat[0]+baseQuat[1]*baseQuat[1]));
    rpy[1]= asin(2*(baseQuat[3]*baseQuat[1]-baseQuat[0]*baseQuat[2]));
    rpy[2]= atan2(2*(baseQuat[3]*baseQuat[2]+baseQuat[0]*baseQuat[1]),1-2*(baseQuat[1]*baseQuat[1]+baseQuat[2]*baseQuat[2]));

    for (int i=0;i<3;i++)
    {
        double posOld=basePos[i];
        basePos[i]=mj_data->xpos[3*baseBodyId+i];
        baseAcc[i]=mj_data->sensordata[mj_model->sensor_adr[accSensorId]+i];
        baseAngVel[i]=mj_data->sensordata[mj_model->sensor_adr[gyroSensorId]+i];
        baseLinVel[i]=(basePos[i]-posOld)/(mj_model->opt.timestep);
    }
    //左右脚IMU数据处理
    for(int i=0;i<4;i++)
    {
        LfQuat[i]=mj_data->sensordata[mj_model->sensor_adr[LfootOrientataionSensorId]+i];
        RfQuat[i]=mj_data->sensordata[mj_model->sensor_adr[RfootOrientataionSensorId]+i];
    }
    double ltmp=LfQuat[0];
    LfQuat[0]=LfQuat[1];
    LfQuat[1]=LfQuat[2];
    LfQuat[2]=LfQuat[3];
    LfQuat[3]=ltmp;
    double rtmp=RfQuat[0];
    RfQuat[0]=RfQuat[1];
    RfQuat[1]=RfQuat[2];
    RfQuat[2]=RfQuat[3];
    RfQuat[3]=rtmp;
    Lfrpy[0]=atan2(2*(LfQuat[3]*LfQuat[0]+LfQuat[1]*LfQuat[2]),1-2*(LfQuat[0]*LfQuat[0]+LfQuat[1]*LfQuat[1]));
    Lfrpy[1]=asin(2*(LfQuat[3]*LfQuat[1]-LfQuat[0]*LfQuat[2]));
    Lfrpy[2]=atan2(2*(LfQuat[3]*LfQuat[2]+LfQuat[0]*LfQuat[1]),1-2*(LfQuat[1]*LfQuat[1]+LfQuat[2]*LfQuat[2]));

    Rfrpy[0]=atan2(2*(RfQuat[3]*RfQuat[0]+RfQuat[1]*RfQuat[2]),1-2*(RfQuat[0]*RfQuat[0]+RfQuat[1]*RfQuat[1]));
    Rfrpy[1]=asin(2*(RfQuat[3]*RfQuat[1]-RfQuat[0]*RfQuat[2]));
    Rfrpy[2]=atan2(2*(RfQuat[3]*RfQuat[2]+RfQuat[0]*RfQuat[1]),1-2*(RfQuat[1]*RfQuat[1]+RfQuat[2]*RfQuat[2]));

    for (int i=0;i<3;i++)
    {
        double LfposOld=LfPos[i];
        LfPos[i]=mj_data->xpos[3*LfootBodyId+i];
        LfAcc[i]=mj_data->sensordata[mj_model->sensor_adr[LfootAccSensorId]+i];
        LfAngVel[i]=mj_data->sensordata[mj_model->sensor_adr[LfootGyroSensorId]+i];
        LfLinVel[i]=(LfPos[i]-LfposOld)/(mj_model->opt.timestep);

        double RfposOld=RfPos[i];
        RfPos[i]=mj_data->xpos[3*RfootBodyId+i];
        RfAcc[i]=mj_data->sensordata[mj_model->sensor_adr[RfootAccSensorId]+i];
        RfAngVel[i]=mj_data->sensordata[mj_model->sensor_adr[RfootGyroSensorId]+i];
        RfLinVel[i]=(RfPos[i]-RfposOld)/(mj_model->opt.timestep);
    }

    Lfcontact[0]=mj_data->sensordata[mj_model->sensor_adr[LfootflContactSensorId]];
    Lfcontact[1]=mj_data->sensordata[mj_model->sensor_adr[LfootfrContactSensorId]];
    Lfcontact[2]=mj_data->sensordata[mj_model->sensor_adr[LfootblContactSensorId]];
    Lfcontact[3]=mj_data->sensordata[mj_model->sensor_adr[LfootbrContactSensorId]];

    Rfcontact[0]=mj_data->sensordata[mj_model->sensor_adr[RfootflContactSensorId]];
    Rfcontact[1]=mj_data->sensordata[mj_model->sensor_adr[RfootfrContactSensorId]];
    Rfcontact[2]=mj_data->sensordata[mj_model->sensor_adr[RfootblContactSensorId]];
    Rfcontact[3]=mj_data->sensordata[mj_model->sensor_adr[RfootbrContactSensorId]];

    Lftouch=mj_data->sensordata[mj_model->sensor_adr[LfootContactSensorId]];
    Rftouch=mj_data->sensordata[mj_model->sensor_adr[RfootContactSensorId]];
}

void MJ_Interface::setMotorsTorque(std::vector<double> &tauIn) {
    for (int i=0;i<jointNum;i++)
        mj_data->ctrl[i]=tauIn.at(i);
}

void MJ_Interface::dataBusWrite(DataBus &busIn) {
    busIn.motors_pos_cur=motor_pos;
    busIn.motors_vel_cur=motor_vel;
    busIn.rpy[0]=rpy[0];
    busIn.rpy[1]=rpy[1];
    busIn.rpy[2]=rpy[2];
    busIn.fL[0]=f3d[0][0];
    busIn.fL[1]=f3d[1][0];
    busIn.fL[2]=f3d[2][0];
    busIn.fR[0]=f3d[0][1];
    busIn.fR[1]=f3d[1][1];
    busIn.fR[2]=f3d[2][1];
    busIn.basePos[0]=basePos[0];
    busIn.basePos[1]=basePos[1];
    busIn.basePos[2]=basePos[2];
    busIn.baseLinVel[0]=baseLinVel[0];
    busIn.baseLinVel[1]=baseLinVel[1];
    busIn.baseLinVel[2]=baseLinVel[2];
    busIn.baseAcc[0]=baseAcc[0];
    busIn.baseAcc[1]=baseAcc[1];
    busIn.baseAcc[2]=baseAcc[2];
    busIn.baseAngVel[0]=baseAngVel[0];
    busIn.baseAngVel[1]=baseAngVel[1];
    busIn.baseAngVel[2]=baseAngVel[2];
    busIn.updateQ();

    busIn.fLrpy[0]=Lfrpy[0];
    busIn.fLrpy[1]=Lfrpy[1];
    busIn.fLrpy[2]=Lfrpy[2];
    busIn.fLPos[0]=LfPos[0];
    busIn.fLPos[1]=LfPos[1];
    busIn.fLPos[2]=LfPos[2];
    busIn.fLLinVel[0]=LfLinVel[0];
    busIn.fLLinVel[1]=LfLinVel[1];
    busIn.fLLinVel[2]=LfLinVel[2];
    busIn.fLAcc[0]=LfAcc[0];
    busIn.fLAcc[1]=LfAcc[1];
    busIn.fLAcc[2]=LfAcc[2];
    busIn.fLAngVel[0]=LfAngVel[0];
    busIn.fLAngVel[1]=LfAngVel[1];
    busIn.fLAngVel[2]=LfAngVel[2];

    busIn.fRrpy[0]=Rfrpy[0];
    busIn.fRrpy[1]=Rfrpy[1];
    busIn.fRrpy[2]=Rfrpy[2];
    busIn.fRPos[0]=RfPos[0];
    busIn.fRPos[1]=RfPos[1];
    busIn.fRPos[2]=RfPos[2];
    busIn.fRLinVel[0]=RfLinVel[0];
    busIn.fRLinVel[1]=RfLinVel[1];
    busIn.fRLinVel[2]=RfLinVel[2];
    busIn.fRAcc[0]=RfAcc[0];
    busIn.fRAcc[1]=RfAcc[1];
    busIn.fRAcc[2]=RfAcc[2];
    busIn.fRAngVel[0]=RfAngVel[0];
    busIn.fRAngVel[1]=RfAngVel[1];
    busIn.fRAngVel[2]=RfAngVel[2];

    busIn.fLcontact[0]=Lfcontact[0];
    busIn.fLcontact[1]=Lfcontact[1];
    busIn.fLcontact[2]=Lfcontact[2];
    busIn.fLcontact[3]=Lfcontact[3];

    busIn.fRcontact[0]=Rfcontact[0];
    busIn.fRcontact[1]=Rfcontact[1];
    busIn.fRcontact[2]=Rfcontact[2];
    busIn.fRcontact[3]=Rfcontact[3];

    busIn.fLtouch=Lftouch;
    busIn.fRtouch=Rftouch;
}










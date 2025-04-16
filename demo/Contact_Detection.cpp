#include <iostream>
#include "GT2FIS_Contact.h"
#include "Data_Filter.h"
#include <thread>
#include <chrono>
#include <sys/shm.h>
#include <sys/stat.h>
#include <string.h>
#include <csignal>
#include <unistd.h>

#include "foxglove/websocket/base64.hpp"
#include "foxglove/websocket/server_factory.hpp"
#include "foxglove/websocket/websocket_notls.hpp"
#include "foxglove/websocket/websocket_server.hpp"
#include <nlohmann/json.hpp>

#include "evaluateMyFIS.h"

// 定义与主进程相同的共享内存结构
struct SharedRobotData {
    double simTime;
    double lF_acc[3];
    double lF_rpy[3];
    double lF_vel_z;
    double hip_joint_pos;
    double knee_joint_pos;
    double Contactforce;
    // ...其他需要的数据
    int dataReady;  // 数据就绪标志
};

// 全局变量用于处理信号
volatile bool running = true;

// 信号处理函数
void signalHandler(int signum) {
    std::cout << "接收到信号: " << signum << std::endl;
    running = false;
}

// 连接到共享内存
SharedRobotData* connectToSharedMemory() {
    // 确保键值文件存在
    FILE* fp = fopen("/tmp/openloong_shm_key", "w");
    if (fp) fclose(fp);
    
    // 修改此行，使用与发送端相同的参数
    key_t key = ftok("/tmp/openloong_shm_key", 'R');
    
    // 打印键值进行调试
    std::cout << "使用共享内存键值: " << key << std::endl;
    
    int shmid = shmget(key, sizeof(SharedRobotData), 0666);
    if (shmid < 0) {
        perror("shmget failed, 请确保主程序已启动");
        return nullptr;
    }
    
    SharedRobotData* data = (SharedRobotData*)shmat(shmid, NULL, 0);
    if (data == (void*)-1) {
        perror("shmat failed");
        return nullptr;
    }
    
    return data;
}


//Set the websocket comuunication time epoch
static uint64_t nanosecondsSinceEpoch() {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count());
  }


int main() {

    const std::vector<std::vector<std::vector<double>>> R_matrix = 
    {
        {
            {-0.0199, 0.0177, 0.7093, -0.6695, 0.0770, 1.0000},
            {-0.0112, 0.0160, 0.4229, -0.6385, -0.0545, 1.0000},
            {0.1964, 0.0305, 0.9050, -0.7113, -0.4908, 1.0000},
            {0.1605, -0.5783, 0.9932, -0.8006, 0.1505, 0},
            {0.1319, 0.0234, 0.2200, -0.6127, 0.4663, 1.0000},
            {-0.3286, -0.5748, 0.8578, -0.9700, 0.6676, 0},
            {-0.8266, -0.2527, 0.6666, -0.9984, 0.4624, 0},
            {-0.9700, 0.3211, 0.4535, -0.9462, -0.1658, 0},
            {-0.7129, 0.7046, 0.3298, -0.8656, -0.6248, 0},
            {-0.0006, -0.6419, 0.9555, -0.9064, 0.4128, 0},
            {-0.2148, 0.9261, 0.2509, -0.7647, -0.9297, 0},
            {0.3762, -0.3536, 0.9696, -0.7003, 0.5154, 1.0000},
            {-0.9776, 0.0226, 0.5501, -0.9814, 0.1531, 0}
        },
        {
            {-0.0371, 0.0289, 0.7413, -0.6760, 0.0700, 1.0000},
            {-0.0198, 0.0177, 0.4022, -0.6362, -0.0732, 1.0000},
            {0.2275, 0.0229, 0.9123, -0.7120, -0.5148, 1.0000},
            {0.1444, -0.5938, 0.9981, -0.8156, 0.1365, 0},
            {0.1504, -0.0264, 0.2113, -0.6107, 0.4982, 1.0000},
            {-0.2936, -0.5808, 0.8693, -0.9650, 0.6503, 0},
            {-0.8171, -0.2262, 0.6687, -0.9979, 0.4396, 0},
            {-0.9865, 0.2951, 0.4536, -0.9490, -0.1453, 0},
            {-0.6895, 0.7284, 0.3204, -0.8581, -0.6555, 0},
            {0.0154, -0.6402, 0.9596, -0.8998, 0.3884, 0},
            {-0.1796, 0.9331, 0.2671, -0.7619, -0.9643, 0},
            {0.4215, -0.3529, 0.9700, -0.6997, 0.5114, 1.0000},
            {-0.9872, 0.0397, 0.5404, -0.9796, 0.1307, 0}
         
        },
        {
            {-0.0438, 0.0320, 0.7591, -0.6785, 0.0494, 1.0000},
            {0.0055, 0.0144, 0.4602, -0.6423, -0.0852, 1.0000},
            {0.2560, 0.0160, 0.9147, -0.7128, -0.5338, 1.0000},
            {0.1404, -0.6000, 0.9947, -0.8239, 0.1695, 0},
            {0.0824, 0.0558, 0.2334, -0.6129, 0.3836, 1.0000},
            {-0.2691, -0.6042, 0.8774, -0.9638, 0.6738, 0},
            {-0.8705, -0.2158, 0.6450, -0.9976, 0.4224, 0},
            {-0.9793, 0.2754, 0.4684, -0.9529, -0.1171, 0},
            {-0.6667, 0.7379, 0.3178, -0.8550, -0.6741, 0},
            {-0.0315, -0.6403, 0.9454, -0.9153, 0.4551, 0},
            {-0.3165, 0.8975, 0.2616, -0.7847, -0.8935, 0},
            {0.4215, -0.3552, 0.9645, -0.6984, 0.4553, 1.0000},
            {-0.9848, 0.0566, 0.5364, -0.9781, 0.1091, 0}
        
        },
        {
            {0.0327, 0.0052, 0.6439, -0.6608, 0.0939, 1.0000},
            {-0.0309, 0.0149, 0.3575, -0.6310, -0.0213, 1.0000},
            {0.3020, 0.0048, 0.9243, -0.7127, -0.5146, 1.0000},
            {0.1963, -0.5613, 0.9966, -0.7625, 0.1392, 0},
            {0.1941, -0.0516, 0.1992, -0.6076, 0.5167, 1.0000},
            {-0.2403, -0.6155, 0.8849, -0.9601, 0.6548, 0},
            {-0.8813, -0.1994, 0.6382, -0.9970, 0.4014, 0},
            {-0.9943, 0.2486, 0.4691, -0.9557, -0.0958, 0},
            {-0.6446, 0.7615, 0.3097, -0.8478, -0.7056, 0},
            {-0.0449, -0.6347, 0.9401, -0.9176, 0.4723, 0},
            {-0.0634, 0.9612, 0.2284, -0.7323, -0.9488, 0},
            {0.3360, -0.2803, 0.9773, -0.7032, 0.5872, 1.0000},
            {-0.9660, -0.0353, 0.5693, -0.9864, 0.2141, 0}
         
        },
        {
            {0.0564, 0.0077, 0.6347, -0.6594, 0.0719, 1.0000},
            {-0.0285, 0.0124, 0.3595, -0.6327, 0.0243, 1.0000},
            {0.3371, -0.0058, 0.9295, -0.7129, -0.4682, 1.0000},
            {0.2066, -0.5483, 0.9943, -0.7518, 0.1441, 0},
            {0.2602, 0.0072, 0.1881, -0.6069, 0.4448, 1.0000},
            {-0.4484, -0.5444, 0.8209, -0.9842, 0.7082, 0},
            {-0.7691, -0.3366, 0.6972, -0.9996, 0.5560, 0},
            {-0.9318, 0.4139, 0.4418, -0.9343, -0.2582, 0},
            {-0.6116, 0.7762, 0.3112, -0.8431, -0.7314, 0},
            {0.0501, -0.6378, 0.9712, -0.8866, 0.3251, 0},
            {-0.0135, 0.9632, 0.2246, -0.7236, -0.9273, 0},
            {0.2933, -0.2998, 0.9753, -0.7050, 0.6175, 1.0000},
            {-0.9302, -0.0304, 0.5900, -0.9878, 0.2343, 0}
        },
        {
            {-0.0526, 0.0369, 0.7964, -0.6857, -0.0102, 1.0000},
            {-0.0283, 0.0121, 0.3251, -0.6269, 0.0316, 1.0000},
            {0.0609, 0.0447, 0.8738, -0.7044, -0.3592, 1.0000},
            {0.1101, -0.6116, 0.9863, -0.8496, 0.1993, 0},
            {0.2794, 0.0157, 0.1769, -0.6049, 0.3938, 1.0000},
            {-0.1827, -0.6265, 0.9013, -0.9499, 0.6148, 0},
            {-0.9105, -0.1576, 0.6173, -0.9945, 0.3553, 0},
            {-0.9958, 0.2011, 0.4855, -0.9618, -0.0457, 0},
            {-0.5919, 0.7904, 0.2996, -0.8370, -0.7511, 0},
            {-0.0751, -0.6403, 0.9333, -0.9281, 0.5153, 0},
            {-0.4448, 0.8586, 0.2774, -0.8081, -0.8421, 0},
            {0.2511, -0.4727, 0.9815, -0.7093, 0.5230, 1.0000},
            {-0.9485, -0.0673, 0.5848, -0.9890, 0.2536, 0}
        },
        {
            {0.0409, 0.0021, 0.5963, -0.6557, -0.0126, 1.0000},
            {-0.0499, 0.0065, 0.5685, -0.6527, -0.0852, 1.0000},
            {0.3994, -0.0309, 0.9403, -0.7120, -0.3946, 1.0000},
            {0.2112, -0.5254, 0.9872, -0.7224, 0.1290, 0},
            {0.0339, 0.0021, 0.2537, -0.6157, 0.2805, 1.0000},
            {-0.5039, -0.4857, 0.8021, -0.9877, 0.6589, 0},
            {-0.7270, -0.3710, 0.7184, -0.9994, 0.5900, 0},
            {-0.9126, 0.4569, 0.4289, -0.9267, -0.3089, 0},
            {-0.5582, 0.8052, 0.2980, -0.8311, -0.7746, 0},
            {0.0709, -0.6327, 0.9777, -0.8760, 0.2837, 0},
            {0.1049, 0.9975, 0.2092, -0.6939, -0.8274, 0},
            {0.4504, -0.2979, 0.9619, -0.6984, 0.3042, 1.0000},
            {-0.9933, 0.1356, 0.5102, -0.9701, 0.0244, 0}

        }
    };

    const std::vector<std::vector<double>> SM_matrix = 
    {
        {0.301633557257089, 0.273374157692937, 0.203512394410977, 0.124445694025718, 0.0625064002012367, 0.0257884183206521, 0.00873937809138969},
        {0.303786141156295,	0.274851766046195, 0.203558975070884, 0.123407947223601, 0.0612430978138081, 0.0248789656896422, 0.00827310699957505},
        {0.254617355080081,	0.238753440735543, 0.196849605488350, 0.142706163778695, 0.0909649714656407, 0.0509834143553069, 0.0251250490963832},
        {0.325583142986359, 0.289214614960524, 0.202719351470849, 0.112120965596159, 0.0489322401899912, 0.0168507803216177, 0.00457890447449891},
        {0.277601363565471, 0.256212366341638, 0.201435293277397, 0.134904994289851, 0.0769622003490928, 0.0374010597976764, 0.0154827223788740},
        {0.271701064738276, 0.251822613977835, 0.200495620130255, 0.137126637468415, 0.0805648380146968, 0.0406608098712636, 0.0176284157992583},
        {0.267206301501883, 0.248434823780491, 0.199668612697234, 0.138719872578576, 0.0833103797821029, 0.0432504967243994, 0.0194095129353146},
        {0.265855415949925, 0.247409403781808, 0.199401432758870, 0.139181568855527, 0.0841349734088108, 0.0440466392336426, 0.0199705660114168},
        {0.230278132669013, 0.219289903169965, 0.189372296508827, 0.148301699974520, 0.105319232654601, 0.0678267669693977, 0.0396119680536757},
        {0.306180631091075, 0.276483330671549, 0.203583543241366, 0.122235924701225, 0.0598463314094796, 0.0238923397399286, 0.00777789914537628},
        {0.168674459801529, 0.166347952669204, 0.159559202411990, 0.148854688319980, 0.135063948879872, 0.119193516537331, 0.102306231380094},
        {0.250776079420920, 0.235744954362594, 0.195844464134775, 0.143778116318372, 0.0932796428853188, 0.0534802656452357, 0.0270964772327851},
        {0.293926332046742, 0.268000991601066, 0.203156217619861, 0.128032329622660, 0.0670818434581694, 0.0292203989806746, 0.0105818866708269}
    };

    // Example dimensions
    const int numRules = R_matrix[0].size();    // Number of rules (r)
    const int inputDim = R_matrix[0][0].size()-1;    // Input dimension (d)
    const int outputDim = 1;   // Output dimension (always 1)
    const int numMF = R_matrix.size();       // Number of membership functions (q)

    /*************** websocket server begin *************/
    const auto logHandler = [](foxglove::WebSocketLogLevel, char const* msg) {
        std::cout << "WebSocket: " << msg << std::endl;
      };
      foxglove::ServerOptions serverOptions;
      auto server = foxglove::ServerFactory::createServer<websocketpp::connection_hdl>(
        "OpenLoong Contact Detection Server", logHandler, serverOptions);

    foxglove::ServerHandlers<foxglove::ConnHandle> hdlrs;
    hdlrs.subscribeHandler = [&](foxglove::ChannelId chanId, foxglove::ConnHandle clientHandle) {
        const auto clientStr = server->remoteEndpointString(clientHandle);
        std::cout << "Client " << clientStr << " subscribed to " << chanId << std::endl;
    };
    hdlrs.unsubscribeHandler = [&](foxglove::ChannelId chanId, foxglove::ConnHandle clientHandle) {
        const auto clientStr = server->remoteEndpointString(clientHandle);
        std::cout << "Client " << clientStr << " unsubscribed from " << chanId << std::endl;
    };

    server->setHandlers(std::move(hdlrs));
    server->start("0.0.0.0", 8765);

    const auto channelIds = server->addChannels({
        {
          .topic = "contact_state",
          .encoding = "json",
          .schemaName = "ContactState",
          .schema = nlohmann::json{
            {"type", "object"},
            {"properties", {
              {"timestamp", {{"type", "number"}}},
              {"is_contact", {{"type", "boolean"}}},
              {"probability", {{"type", "number"}}},
              {"contact_truth", {{"type", "boolean"}}}
            }}
          }.dump(),
        },
        {
          .topic = "input_data",
          .encoding = "json",
          .schemaName = "InputData",
          .schema = nlohmann::json{
            {"type", "object"},
            {"properties", {
              {"timestamp", {{"type", "number"}}},           
              {"lF_accz_normalized", {{"type", "number"}}},
              {"lF_velz_normalized", {{"type", "number"}}},
              {"hip_joint_normalized", {{"type", "number"}}},
              {"knee_joint_normalized", {{"type", "number"}}},
              {"lF_accz_diff_normalized", {{"type", "number"}}},
            }}
          }.dump(),
        },
        {
          .topic = "sensor_data",
          .encoding = "json",
          .schemaName = "SensorData",
          .schema = nlohmann::json{
            {"type", "object"},
            {"properties", {
              {"timestamp", {{"type", "number"}}},
              {"acc_x", {{"type", "number"}}},
              {"acc_y", {{"type", "number"}}},
              {"acc_z", {{"type", "number"}}},
              {"rpy_x", {{"type", "number"}}},
              {"rpy_y", {{"type", "number"}}},
              {"rpy_z", {{"type", "number"}}},
              {"vel_z", {{"type", "number"}}},
              {"hip_joint_pos", {{"type", "number"}}},
              {"knee_joint_pos", {{"type", "number"}}},
            }}
          }.dump(),
        },
        {
          .topic = "debug_data",
          .encoding = "json",
          .schemaName = "DebugData",
            .schema = nlohmann::json{
                {"type", "object"},
                {"properties", {
                {"DebugData1", {{"type", "number"}}},
                {"DebugData2", {{"type", "number"}}},
                {"DebugData3", {{"type", "number"}}},
                {"DebugData4", {{"type", "number"}}},
                {"DebugData5", {{"type", "number"}}},
                {"DebugData6", {{"type", "number"}}},
                }}
            }.dump(),  
        }
      });
    /**************** websocket server end *************/
    // 设置信号处理
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "正在连接到OpenLoong控制程序..." << std::endl;

    // 连接到共享内存
    SharedRobotData* robotData = connectToSharedMemory();
    if (!robotData) {
        std::cerr << "无法连接到机器人数据。请确保主控制程序正在运行。" << std::endl;
        return 1;
    }

    std::cout << "已连接到机器人数据共享内存" << std::endl;


    // Initialize rule base matrix (r x (d+1) x q)
    std::vector<std::vector<std::vector<double>>> ruleBase(
        numMF, 
        std::vector<std::vector<double>>(
            numRules, 
            std::vector<double>(inputDim + outputDim, 0.0)
        )
    );
    
    // Initialize uncertainty weights matrix (r x q)
    std::vector<std::vector<double>> uncertaintyWeights(
        numRules, 
        std::vector<double>(numMF, 0.0)
    );
    
    // Populate rule base and uncertainty weights with example values
    // These would normally be loaded from external data or learned
    ruleBase = R_matrix;
    uncertaintyWeights = SM_matrix;
    
    // Create GT2FCM instance
    GT2FCM fuzzyModel(ruleBase, uncertaintyWeights, numRules, inputDim, numMF);
    // Create DataFilterNormalizer instance
    DataFilterNormalizer dataFilter;
    // Initialize data filter with example parameters
    std::vector<double> inputData(inputDim, 0.0);
    std::vector<double> debugData(6, 0.0);
    // 转换C风格数组到std::array
    std::array<double, 3> lF_acc_array;
    std::array<double, 3> lF_rpy_array;
    uint8_t beta_low = 10;   // 低值区域的压缩系数（较小）
    uint8_t beta_high = 12; // 高值区域的压缩系数（较大）
    bool b_output; // Example output flag
    bool b_contact_truth;
    while(running)
    {
        // 手动复制数据
        for (int i = 0; i < 3; i++) {
            lF_acc_array[i] = robotData->lF_acc[i];
            lF_rpy_array[i] = robotData->lF_rpy[i];
        }
        if (robotData->dataReady) {
            // 实现数据滤波和归一化
            //debugData = dataFilter.processData(lF_acc_array, lF_rpy_array, robotData->lF_vel_z, robotData->hip_joint_pos, robotData->knee_joint_pos);
            inputData = dataFilter.processData(lF_acc_array, lF_rpy_array, robotData->lF_vel_z, robotData->hip_joint_pos, robotData->knee_joint_pos);
            
        }
        // Calculate output
        double output = fuzzyModel.calculate(inputData);

        // Check for errors
        //output = 0.5 + 0.5 * tanh(beta * (output - 0.5));
        if (output < 0.65) {
            // 对低值区域使用较小的beta
            output = 0.5 + 0.5 * tanh(beta_low * (output - 0.65));
        } else {
            // 对高值区域使用较大的beta
            output = 0.5 + 0.5 * tanh(beta_high * (output - 0.65));
        }
        // Convert output to binary with thresholdlF_rpy
        if (output > 0.75){
            b_output = 1;
        }
        else{
            b_output = 0;
        }

        if (robotData->Contactforce >= 5){
            b_contact_truth = 1;
        }
        else{
            b_contact_truth = 0;
        }
        // 区间二型模糊推理系统
        

        // 创建并发布传感器数据消息
        nlohmann::json sensorMsg = {
            {"timestamp", robotData->simTime},
            {"acc_x", robotData->lF_acc[0]},
            {"acc_y", robotData->lF_acc[1]},
            {"acc_z", robotData->lF_acc[2]},
            {"rpy_x", robotData->lF_rpy[0]},
            {"rpy_y", robotData->lF_rpy[1]},
            {"rpy_z", robotData->lF_rpy[2]},
            {"vel_z", robotData->lF_vel_z},
            {"hip_joint_pos", robotData->hip_joint_pos},
            {"knee_joint_pos", robotData->knee_joint_pos}
        };
        nlohmann::json contactMsg = {
            {"timestamp", robotData->simTime},
            {"is_contact", b_output},
            {"probability", output},
            {"contact_truth", b_contact_truth}
        };
        nlohmann::json inputMsg = {
            {"timestamp", robotData->simTime},
            {"lF_accz_normalized", inputData[0]},
            {"lF_velz_normalized", inputData[1]},
            {"hip_joint_normalized", inputData[2]},
            {"knee_joint_normalized", inputData[3]},
            {"lF_accz_diff_normalized", inputData[4]}
        };
        nlohmann::json debugMsg = {
            {"timestamp", robotData->simTime},
            {"DebugData1", debugData[0]},
            {"DebugData2", debugData[1]},
            {"DebugData3", debugData[2]},
            {"DebugData4", debugData[3]},
            {"DebugData5", debugData[4]},
            {"DebugData6", debugData[5]}
        };



        // 获取当前时间戳
        const auto now = nanosecondsSinceEpoch();

        // 发布传感器数据
        std::string contactStr = contactMsg.dump();
        server->broadcastMessage(channelIds[0], now, 
                                reinterpret_cast<const uint8_t*>(contactStr.data()),
                                contactStr.size());
        std::string sensorStr = sensorMsg.dump();
        server->broadcastMessage(channelIds[2], now, 
                                reinterpret_cast<const uint8_t*>(sensorStr.data()),
                                sensorStr.size());
        std::string inputStr = inputMsg.dump();
        server->broadcastMessage(channelIds[1], now,
                                reinterpret_cast<const uint8_t*>(inputStr.data()),
                                inputStr.size()); 
        std::string debugStr = debugMsg.dump();
        server->broadcastMessage(channelIds[3], now,
                                reinterpret_cast<const uint8_t*>(debugStr.data()),
                                debugStr.size());        

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    server->removeChannels(channelIds);
    server->stop();
    
    // 分离共享内存
    shmdt(robotData);
    std::cout << "程序已终止" << std::endl;
    return 0;
}
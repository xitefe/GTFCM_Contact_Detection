#include "IT2FIS_Stable_Contact.h"

void StableContactDetector::addFrame(bool b_output, const std::array<double, 3>& position, 
    const std::array<double, 3>& velocity,
    const std::array<double, 3>& angular_vel,
    double contact_probability) {
    if(b_output){
        // 添加新数据到历史队列
        position_history.push_back(position);
        velocity_history.push_back(velocity);
        angular_vel_history.push_back(angular_vel);
        contact_prob_history.push_back(contact_probability);

        // 限制队列大小
        if (position_history.size() > window_size) {
            position_history.pop_front();
            velocity_history.pop_front();
            angular_vel_history.pop_front();
            contact_prob_history.pop_front();
        }
    }
    else{
        // 添加新数据到历史队列
        position_history.push_back(position);
        velocity_history.push_back(velocity);
        angular_vel_history.push_back(angular_vel);
        contact_prob_history.push_back(contact_probability);
        // 如果没有接触，则清空历史数据
        position_history.clear();
        // 限制队列大小
        if (position_history.size() > window_size) {
            position_history.pop_front();
            velocity_history.pop_front();
            angular_vel_history.pop_front();
            contact_prob_history.pop_front();
        }
    }
}

//
std::array<double, 3> StableContactDetector::transformToWorldFrame(
const std::array<double, 3>& vec, const std::array<double, 3>& rpy) {

    // 获取roll, pitch, yaw角度
    double roll = rpy[0];
    double pitch = rpy[1];
    double yaw = rpy[2];

    // 创建旋转矩阵
    // 绕X轴(roll)
    double Rx[3][3] = {
                    {1, 0, 0},
                    {0, cos(roll), -sin(roll)},
                    {0, sin(roll), cos(roll)}};

    // 绕Y轴(pitch)
    double Ry[3][3] = {
                    {cos(pitch), 0, sin(pitch)},
                    {0, 1, 0},
                    {-sin(pitch), 0, cos(pitch)}};

    // 绕Z轴(yaw)
    double Rz[3][3] = {
                    {cos(yaw), -sin(yaw), 0},
                    {sin(yaw), cos(yaw), 0},
                    {0, 0, 1}};

    // 临时数组用于计算
    double temp1[3] = {0};
    double temp2[3] = {0};
    std::array<double, 3> result = {0};

    // 按ZYX顺序应用旋转（先Z，再Y，最后X）
    // Ry * vec
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            temp1[i] += Ry[i][j] * vec[j];
        }
    }

    // Rz * (Ry * vec)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            temp2[i] += Rz[i][j] * temp1[j];
        }
    }

    // Rx * (Rz * Ry * vec)
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            result[i] += Rx[i][j] * temp2[j];
        }
    }

    return result;
}

double StableContactDetector::calculateHorizontalDisplacement(void){
    if (position_history.size() < 2) {
        return 0.0;  // 不够数据
    }

    // 计算窗口起点和终点的水平位移
    double dx = position_history.back()[0] - position_history.front()[0];
    double dy = position_history.back()[1] - position_history.front()[1];

    // 计算水平平面的欧几里得距离
    double displacement = sqrt(dx*dx + dy*dy);
    return displacement;
}

std::array<double, 3> StableContactDetector::calculateWorldTruthVelocity(const std::array<double, 3>& world_vel){
    std::array<double, 3> world_truth_velocity;
    if(position_history.size() > 1)
    {
        world_truth_velocity[0] = (position_history.back()[0] - position_history[position_history.size()-2][0]) / dt;
        world_truth_velocity[1] = (position_history.back()[1] - position_history[position_history.size()-2][1]) / dt;
        world_truth_velocity[2] = (position_history.back()[2] - position_history[position_history.size()-2][2]) / dt;
    }
    else
    {
        world_truth_velocity[0] = 0;
        world_truth_velocity[1] = 0;
        world_truth_velocity[2] = 0;
    }
    return world_truth_velocity;
}
#ifndef IT2FIS_STABLE_CONTACT_H
#define IT2FIS_STABLE_CONTACT_H

#include <vector>
#include <array>
#include <deque>
#include <cmath>
#include <algorithm>
#include <numeric>

// 在DataFilterNormalizer类中添加或在新类StableContactDetector中添加
class StableContactDetector {
    private:
        // 历史数据窗口
        std::deque<std::array<double, 3>> position_history;  // 位置历史
        std::deque<std::array<double, 3>> velocity_history;  // 速度历史
        std::deque<std::array<double, 3>> angular_vel_history; // 角速度历史
        std::deque<double> contact_prob_history;  // 接触概率历史
        
        // 窗口大小
        size_t window_size;
        double dt = 0.001;
        
        // 归一化参数
        double max_horizontal_disp;
        double max_horizontal_vel;
        double max_angular_vel;
        
    public:
        StableContactDetector(size_t window_size = 50) : 
            window_size(window_size),
            max_horizontal_disp(0.3),
            max_horizontal_vel(1.0),
            max_angular_vel(10.0) {
            // 初始化
        }
        
        // 添加当前帧数据
        void addFrame(bool b_output, const std::array<double, 3>& position, 
                     const std::array<double, 3>& velocity,
                     const std::array<double, 3>& angular_vel,
                     double contact_probability);
                     
        
        // 世界坐标系转换
        std::array<double, 3> transformToWorldFrame(const std::array<double, 3>& vec, 
                                                   const std::array<double, 3>& rpy);
                                                   
        // 计算水平位移
        double calculateHorizontalDisplacement(void);

        std::array<double, 3> calculateWorldTruthVelocity(const std::array<double, 3>& world_vel);
        
        // 计算水平速度大小
        double calculateHorizontalVelocityMagnitude(const std::array<double, 3>& world_vel);
        
        // 计算角速度大小
        double calculateAngularVelocityMagnitude(const std::array<double, 3>& world_ang_vel);
    };




#endif
#include "Data_Filter.h"
#include <numeric>
#include <cmath>
#include <algorithm>

DataFilterNormalizer::DataFilterNormalizer() {
    // Initialize filter coefficients
    designButterLowPass();
    initialize();
}

void DataFilterNormalizer::initialize() {

    // Initialize histories with appropriate size and zeros
    lF_acc_history.clear();
    lF_acc_vertical_history.clear();
    lF_accz_diff_history.clear();
    lF_vel_z_history.clear();
    hip_joint_pos_history.clear();
    knee_joint_pos_history.clear();
    
    // Fill histories with zeros for initial state
    for (int i = 0; i < std::max(window_size, 3*order); i++) {
        lF_acc_history.push_back({0.0, 0.0, 0.0});
        lF_acc_vertical_history.push_back(0.0);
        lF_accz_diff_history.push_back(0.0);
        lF_vel_z_history.push_back(0.0);
        hip_joint_pos_history.push_back(0.0);
        knee_joint_pos_history.push_back(0.0);
    }
    
    // Reset max values for normalization
    max_lF_acc_vertical = 1.0;
    max_lF_vel_z = 1.0;
    max_hip_joint_pos = 1.0;
    max_knee_joint_pos = 1.0;
    max_lF_accz_diff = 1.0;
    
    // Reset previous values
    prev_lF_acc_vertical = {0.0, 0.0, 0.0};
    prev_lF_accz = 0.0;
    
    lF_acc_output_history.clear();
    for (int i = 0; i < 3*order; i++) {
        lF_acc_output_history.push_back({0.0, 0.0, 0.0});
    }
}

void DataFilterNormalizer::reset() {
    initialize();
}

/* Design Butterworth low-pass filter coefficients
    * This is a simplified implementation based on common filter coefficients
    * For a more accurate implementation, additional digital filter design libraries may be needed
    */

void DataFilterNormalizer::designButterLowPass() {
    // Design 2nd-order Butterworth low-pass filter
    // This is a simplified implementation based on common filter coefficients
    // For a more accurate implementation, additional digital filter design libraries may be needed
    
    double Wn = 2.0 * cutoff_freq / fs;  // Normalized cutoff frequency
    
    // For a 2nd-order Butterworth low-pass filter:
    // These are pre-calculated coefficients for a normalized cutoff of Wn
    double ita = tan(M_PI * Wn / 2.0);
    double q = sqrt(2.0);
    
    // Filter coefficients calculation
    double b0 = ita * ita / (1 + q * ita + ita * ita);
    double b1 = 2 * b0;
    double b2 = b0;
    double a1 = 2 * (ita * ita - 1) / (1 + q * ita + ita * ita);
    double a2 = (1 - q * ita + ita * ita) / (1 + q * ita + ita * ita);
    
    // Store coefficients
    b = {b0, b1, b2};
    a = {1.0, a1, a2};  // a0 is assumed to be 1
}

/*  Apply median filter to a value using history
    * This function applies a median filter to the input value using the history of previous values
    * The history is maintained as a deque for efficient insertion and removal
    */
double DataFilterNormalizer::applyMedianFilter(double value, std::deque<double>& history) {
    // Add new value to history
    history.push_back(value);
    if (history.size() > window_size) {
        history.pop_front();
    }
    
    // Copy values for sorting
    std::vector<double> values(history.begin(), history.end());
    std::sort(values.begin(), values.end());
    
    // Return median value
    if (values.size() % 2 == 0) {
        return (values[values.size()/2 - 1] + values[values.size()/2]) / 2.0;
    } else {
        return values[values.size()/2];
    }
}

std::array<double, 3> DataFilterNormalizer::applyButterworthFilter(
    const std::array<double, 3>& input, 
    std::deque<std::array<double, 3>>& history) {
    
    // Add new input to history
    history.push_back(input);
    if (history.size() > 3*order) {
        history.pop_front();
    }
    
    // Apply filter to each axis
    std::array<double, 3> output = {0.0, 0.0, 0.0};
    
    if (history.size() >= 3) {  // Need at least 3 samples for 2nd order filter
        for (int axis = 0; axis < 3; axis++) { 
            // Apply filter formula: y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
            size_t h_size = history.size();
            
            // Input components
            double x0 = history[h_size-1][axis];
            double x1 = history[h_size-2][axis];
            double x2 = history[h_size-3][axis];
            
            // Previous output components
            // For simplicity, we're getting these from the history directly
            // In a more advanced implementation, we would maintain separate output history
            // 前一个输出分量 - 使用输出历史而非输入历史
            double y1 = lF_acc_output_history[lF_acc_output_history.size()-1][axis];
            double y2 = lF_acc_output_history[lF_acc_output_history.size()-2][axis];
            
            // Calculate output
            output[axis] = b[0]*x0 + b[1]*x1 + b[2]*x2 - a[1]*y1 - a[2]*y2;
        }
    } else {
        // Not enough history, return the input
        output = input;
    }
    // 保存输出到历史
    lF_acc_output_history.push_back(output);
    if (lF_acc_output_history.size() > 3*order) {
        lF_acc_output_history.pop_front();
    }
    return output;
}

double DataFilterNormalizer::normalizeWithSign(double value, double max_value) {
    // Update max value if needed
    if (std::abs(value) > max_value) {
        max_value = std::abs(value);
    }
    
    // Sign-preserving normalization
    return (value / max_value);
}

std::vector<double> DataFilterNormalizer::processData(
    const std::array<double, 3>& lF_acc,
    const std::array<double, 3>& lF_rpy,
    double lF_vel_z,
    double hip_joint_pos,
    double knee_joint_pos) {
    
    // Step 1: Apply threshold to acceleration data
    std::array<double, 3> lF_acc_thresholded = {
        std::min(std::max(lF_acc[0], -acc_threshold), acc_threshold),
        std::min(std::max(lF_acc[1], -acc_threshold), acc_threshold),
        std::min(std::max(lF_acc[2], -acc_threshold), acc_threshold)
    }; 
    
    // Step 2: Apply median filter
    // For simplicity, we'll only apply median filter to z-component
    // In a more comprehensive implementation, we would apply to all axes
    // lF_acc_thresholded[0] = applyMedianFilter(lF_acc_thresholded[0], lF_acc_x_history);
    // lF_acc_thresholded[1] = applyMedianFilter(lF_acc_thresholded[1], lF_acc_y_history);
    // lF_acc_thresholded[2] = applyMedianFilter(lF_acc_thresholded[2], lF_acc_z_history);

    
    // Step 3: Apply Butterworth low-pass filter
    lF_acc_filtered = applyButterworthFilter(lF_acc_thresholded, lF_acc_history);
    
    // Step 4: Transform accelerations to world frame using rotation matrices
    // Convert angles from degrees to radians if needed
    double lF_roll = lF_rpy[0];
    double lF_pitch = lF_rpy[1];
    // Calculate rotation matrices
    // Left foot rotation matrix (from sensor frame to world frame)
    double Rx_l[3][3] = {
        {1, 0, 0},
        {0, cos(lF_roll), -sin(lF_roll)},
        {0, sin(lF_roll), cos(lF_roll)}
    };
    
    double Ry_l[3][3] = {
        {cos(lF_pitch), 0, sin(lF_pitch)},
        {0, 1, 0},
        {-sin(lF_pitch), 0, cos(lF_pitch)}
    };
    
    // Combined rotation matrix for left foot
    double R_l[3][3] = {0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            for (int k = 0; k < 3; k++) {
                R_l[i][j] += Ry_l[i][k] * Rx_l[k][j];
            }
        }
    }
    
    // Transform filtered accelerations to world frame
    std::array<double, 3> lF_acc_world = {0, 0, 0};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            lF_acc_world[i] += R_l[i][j] * lF_acc_filtered[j];
        }
    }
    
    // Remove gravity from vertical component
    lF_acc_world[2] -= g;
    
    // Step 5: Calculate vertical acceleration derivative
    double lF_accz_diff = (lF_acc_world[2] - prev_lF_accz) / dt;
    prev_lF_accz = lF_acc_world[2];
    
    // Store in history for future filtering
    lF_accz_diff_history.push_back(lF_accz_diff);
    if (lF_accz_diff_history.size() > window_size) {
        lF_accz_diff_history.pop_front();
    }
    
    // Also store velocity and joint positions in history
    lF_vel_z_history.push_back(lF_vel_z);
    hip_joint_pos_history.push_back(hip_joint_pos);
    knee_joint_pos_history.push_back(knee_joint_pos);
    
    if (lF_vel_z_history.size() > window_size) lF_vel_z_history.pop_front();
    if (hip_joint_pos_history.size() > window_size) hip_joint_pos_history.pop_front();
    if (knee_joint_pos_history.size() > window_size) knee_joint_pos_history.pop_front();
    
    //Step 6: Update max values for normalization
    if (std::abs(lF_acc_world[2]) > max_lF_acc_vertical) {
        max_lF_acc_vertical = std::abs(lF_acc_world[2]);
    }
    
    if (std::abs(lF_vel_z) > max_lF_vel_z) {
        max_lF_vel_z = std::abs(lF_vel_z);
    }
    
    if (std::abs(hip_joint_pos) > max_hip_joint_pos) {
        max_hip_joint_pos = std::abs(hip_joint_pos);
    }
    
    if (std::abs(knee_joint_pos) > max_knee_joint_pos) {
        max_knee_joint_pos = std::abs(knee_joint_pos);
    }
    
    if (std::abs(lF_accz_diff) > max_lF_accz_diff) {
        max_lF_accz_diff = std::abs(lF_accz_diff);
    }
    
    // Step 7: Calculate normalized values
    double lF_accz_normalized = normalizeWithSign(lF_acc_world[2], max_lF_acc_vertical);
    double lF_velz_normalized = normalizeWithSign(lF_vel_z, max_lF_vel_z);
    double hip_joint_normalized = normalizeWithSign(hip_joint_pos, max_hip_joint_pos);
    double knee_joint_normalized = normalizeWithSign(knee_joint_pos, max_knee_joint_pos);
    double lF_accz_diff_normalized = normalizeWithSign(lF_accz_diff, max_lF_accz_diff);
    
    //Return the five normalized values
    return {
        lF_accz_normalized,
        lF_velz_normalized,
        hip_joint_normalized,
        knee_joint_normalized,
        lF_accz_diff_normalized,
    };
    // debug
    // return {
    //     lF_acc_filtered[0],
    //     lF_acc_filtered[1],
    //     lF_acc_filtered[2],
    //     lF_accz_diff,
    //     lF_acc_world[1],
    //     lF_acc_world[2]
    // };
}
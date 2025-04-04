#ifndef DATA_FILTER_H
#define DATA_FILTER_H

#include <vector>
#include <deque>
#include <cmath>
#include <algorithm>
#include <array>

class DataFilterNormalizer {
private:
    // Constants
    const double g = 9.81;           // gravitational acceleration in m/s^2
    const double fs = 1000.0;        // sampling frequency in Hz
    const double dt = 0.001;         // time step
    const double acc_threshold = 25.0; // acceleration threshold
    const int window_size = 8;       // median filter window size
    const double cutoff_freq = 10.0; // cutoff frequency for low-pass filter
    const int order = 2;             // filter order

    // Filter coefficients
    std::vector<double> b;  // numerator coefficients
    std::vector<double> a;  // denominator coefficients

    // State variables for filtering
    std::deque<std::array<double, 3>> lF_acc_history;  // Left foot acceleration history
    std::deque<double> lF_acc_vertical_history;        // Left foot vertical acceleration history
    std::deque<double> lF_accz_diff_history;           // Left foot vertical acceleration derivative history
    std::deque<double> lF_vel_z_history;               // Left foot vertical velocity history
    std::deque<double> hip_joint_pos_history;          // Hip joint position history
    std::deque<double> knee_joint_pos_history;         // Knee joint position history

    // History for max values (for normalization)
    double max_lF_acc_vertical = 1.0;
    double max_lF_vel_z = 1.0;
    double max_hip_joint_pos = 1.0;
    double max_knee_joint_pos = 1.0;
    double max_lF_accz_diff = 1.0;

    // Previous values for derivatives
    std::array<double, 3> prev_lF_acc_vertical = {0.0, 0.0, 0.0};
    double prev_lF_accz = 0.0;

    // Filtered data
    std::array<double, 3> lF_acc_filtered = {0.0, 0.0, 0.0};
    std::array<double, 3> rF_acc_filtered = {0.0, 0.0, 0.0};
    
    // Design Butterworth filter coefficients
    void designButterLowPass();
    
    // Apply median filter to a value using history
    double applyMedianFilter(double value, std::deque<double>& history);
    
    // Apply Butterworth filter to each axis
    std::array<double, 3> applyButterworthFilter(const std::array<double, 3>& input, 
                                                std::deque<std::array<double, 3>>& history);
    
    // Calculate the sign-preserving normalized value
    double normalizeWithSign(double value, double max_value);

public:
    DataFilterNormalizer();
    
    // Initialize the filter
    void initialize();
    
    // Process a single data point in real-time
    std::array<double, 5> processData(
        const std::array<double, 3>& lF_acc,           // Left foot acceleration [x,y,z]
        const std::array<double, 2>& lF_rpy,           // Left foot roll and pitch
        const std::array<double, 2>& rF_rpy,           // Right foot roll and pitch
        double lF_vel_z,                               // Left foot vertical velocity
        double hip_joint_pos,                          // Hip joint position
        double knee_joint_pos                          // Knee joint position
    );
    
    // Reset the filter states
    void reset();
};

#endif // DATA_FILTER_NORMALIZER_H
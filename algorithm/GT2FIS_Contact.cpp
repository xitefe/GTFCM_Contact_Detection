#include "GT2FIS_Contact.h"

// Default constructor
GT2FCM::GT2FCM() : r(0), d(0), q(0), m(2.0) {
}

// Constructor with parameters
GT2FCM::GT2FCM(const std::vector<std::vector<std::vector<double>>>& ruleBase,
               const std::vector<std::vector<double>>& uncertaintyWeights,
               int numRules, int inputDim, int numMF) 
    : R(ruleBase), SM(uncertaintyWeights), r(numRules), d(inputDim), q(numMF), m(2.0) {
    
    // Initialize the UNR matrix with proper dimensions (r x (d+1))
    UNR.resize(r, std::vector<double>(d+1, 0.0));
    
    // Compute the uncertainty rule base matrix
    computeUNR();
}

// Destructor
GT2FCM::~GT2FCM() {
    // Nothing special to clean up
}

// Set the rule base matrix
void GT2FCM::setRuleBase(const std::vector<std::vector<std::vector<double>>>& ruleBase,
                         int numRules, int inputDim, int numMF) {
    R = ruleBase;
    r = numRules;
    d = inputDim;
    q = numMF;
    
    // Resize the UNR matrix when dimensions change
    UNR.resize(r, std::vector<double>(d+1, 0.0));
    
    // Recompute UNR if SM is already set
    if (!SM.empty()) {
        computeUNR();
    }
}

// Set the uncertainty weights matrix
void GT2FCM::setUncertaintyWeights(const std::vector<std::vector<double>>& uncertaintyWeights) {
    SM = uncertaintyWeights;
    
    // Recompute UNR if R is already set
    if (!R.empty()) {
        computeUNR();
    }
}

// Compute the uncertainty rule base matrix
void GT2FCM::computeUNR() {
    // Step 1: Calculate uncertainty rule base matrix UNR
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < (d+1); j++) {
            double numerator = 0.0;
            double denominator = 0.0;
            
            for (int m = 0; m < q; m++) {
                numerator += SM[i][m] * R[i][j][m];
                denominator += SM[i][m];
            }
            
            // Avoid division by zero
            if (denominator < 1e-10) {
                denominator = 1e-10;
            }
            
            UNR[i][j] = numerator / denominator;
        }
    }
}

// Compute distances between input vector and UNRin rows
std::vector<double> GT2FCM::computeDistance(const std::vector<double>& inputData) {
    // Validate input dimensions
    if (inputData.size() != d) {
        throw std::invalid_argument("Input data dimension does not match expected dimension");
    }
    
    // Step 3: Calculate distances
    std::vector<double> distances(r, 0.0);
    
    for (int i = 0; i < r; i++) {
        double sum_squared_diff = 0.0;
        
        for (int n = 0; n < d; n++) {
            double diff = inputData[n] - UNR[i][n];
            sum_squared_diff += diff * diff;
        }
        
        distances[i] = sum_squared_diff;
    }
    
    return distances;
}

// Compute FCM membership vector
std::vector<double> GT2FCM::computeMembership(const std::vector<double>& distances) {
    // Step 4: Calculate FCM membership vector μ
    std::vector<double> mu(r, 0.0);
    
    for (int i = 0; i < r; i++) {
        // Avoid division by zero
        double dist_i = (distances[i] < 1e-10) ? 1e-10 : distances[i];
        double sum_ratio = 0.0;
        
        for (int l = 0; l < r; l++) {
            // Avoid division by zero
            double dist_l = distances[l];
            sum_ratio += std::pow(dist_l / dist_i, 1.0 / (m - 1.0));
        }
        
        mu[i] = sum_ratio;
    }
    
    return mu;
}

// Normalize membership vector
std::vector<double> GT2FCM::normalizeMembers(const std::vector<double>& mu) {
    // Step 5: Normalize membership vector to C
    std::vector<double> C(r, 0.0);
    
    // Find min and max values
    double min_mu = *std::min_element(mu.begin(), mu.end());
    double max_mu = *std::max_element(mu.begin(), mu.end());
    
    if (max_mu > min_mu) {
        // Normal case - scale to [0,1]
        for (int i = 0; i < r; i++) {
            C[i] = (mu[i] - min_mu) / (max_mu - min_mu);
        }
    } else {
        // All mu values are equal
        double val = 1.0 / r;
        std::fill(C.begin(), C.end(), val);
    }
    
    return C;
}

// Calculate output for given input data
double GT2FCM::calculate(const std::vector<double>& inputData) {
    // Compute distances between input vector and UNRin rows
    std::vector<double> distances = computeDistance(inputData);
    
    // Compute FCM membership vector
    std::vector<double> mu = computeMembership(distances);
    
    // Normalize membership vector
    std::vector<double> C = normalizeMembers(mu);
    
    // Step 6: Calculate predicted output using UNR output part and C
    double numerator = 0.0;
    double denominator = 0.0;
    
    for (int i = 0; i < r; i++) {
        numerator += C[i] * UNR[i][d];  // UNR[:, d] is the output part
        denominator += C[i];
    }
    
    // Avoid division by zero
    if (denominator < 1e-10) {
        denominator = 1e-10;
    }
    
    // Calculate final predicted output
    double output = numerator / denominator;
    
    return output;
}
#ifndef GT2FCM_H
#define GT2FCM_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <stdexcept>

/**
 * @brief General Type-2 Fuzzy C-Means algorithm class
 * 
 * This class implements the GT2FCM algorithm for fuzzy inference
 * based on a rule base matrix, uncertainty weights, and input data.
 */
class GT2FCM {
public:
    /**
     * @brief Constructor with parameters
     * 
     * @param ruleBase Rule base matrix (3D)
     * @param uncertaintyWeights Uncertainty weight matrix (2D)
     * @param numRules Number of rules
     * @param inputDim Input dimension
     * @param numMF Number of membership functions
     */
    GT2FCM(const std::vector<std::vector<std::vector<double>>>& ruleBase,
           const std::vector<std::vector<double>>& uncertaintyWeights,
           int numRules, int inputDim, int numMF);

    /**
     * @brief Default constructor
     */
    GT2FCM();

    /**
     * @brief Destructor
     */
    ~GT2FCM();

    /**
     * @brief Set the rule base matrix
     * 
     * @param ruleBase Rule base matrix (3D)
     * @param numRules Number of rules
     * @param inputDim Input dimension
     * @param numMF Number of membership functions
     */
    void setRuleBase(const std::vector<std::vector<std::vector<double>>>& ruleBase,
                     int numRules, int inputDim, int numMF);

    /**
     * @brief Set the uncertainty weights matrix
     * 
     * @param uncertaintyWeights Uncertainty weight matrix (2D)
     */
    void setUncertaintyWeights(const std::vector<std::vector<double>>& uncertaintyWeights);

    /**
     * @brief Calculate output for given input data
     * 
     * @param inputData Input vector
     * @return double Predicted output value
     */
    double calculate(const std::vector<double>& inputData);

private:
    // Member variables
    std::vector<std::vector<std::vector<double>>> R;     // Rule base matrix
    std::vector<std::vector<double>> SM;                // Uncertainty weights matrix
    std::vector<std::vector<double>> UNR;               // Uncertainty rule base matrix
    int r;                                              // Number of rules
    int d;                                              // Input dimension
    int q;                                              // Number of MFs
    double m;                                           // FCM constant

    //Data normalization
    



    // Private helper methods
    void computeUNR();
    std::vector<double> computeDistance(const std::vector<double>& inputData);
    std::vector<double> computeMembership(const std::vector<double>& distances);
    std::vector<double> normalizeMembers(const std::vector<double>& mu);
};

#endif // GT2FCM_H
//
// File: centroid.cpp
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-15 21:40:45
//

// Include Files
#include "centroid.h"

// Function Definitions
//
// Arguments    : const double x[101]
//                const double membershipValueOfX[101]
//                int varargin_1
// Return Type  : double
//
namespace coder {
double centroid(const double x[101], const double membershipValueOfX[101],
                int varargin_1)
{
  double area;
  double y;
  int i;
  y = 0.0;
  area = 0.0;
  i = static_cast<unsigned char>(varargin_1);
  for (int b_i{0}; b_i < i; b_i++) {
    area += membershipValueOfX[b_i];
  }
  if (area == 0.0) {
    y = (x[0] + x[varargin_1 - 1]) / 2.0;
  } else {
    for (int b_i{0}; b_i < i; b_i++) {
      y += x[b_i] * membershipValueOfX[b_i];
    }
    y *= 1.0 / area;
  }
  return y;
}

} // namespace coder

//
// File trailer for centroid.cpp
//
// [EOF]
//

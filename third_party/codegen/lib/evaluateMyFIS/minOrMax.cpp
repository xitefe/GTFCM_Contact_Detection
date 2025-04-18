//
// File: minOrMax.cpp
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-18 17:32:54
//

// Include Files
#include "minOrMax.h"
#include <cmath>

// Function Definitions
//
// Arguments    : const double x[2]
// Return Type  : double
//
namespace coder {
namespace internal {
double maximum(const double x[2])
{
  double ex;
  if ((x[0] < x[1]) || (std::isnan(x[0]) && (!std::isnan(x[1])))) {
    ex = x[1];
  } else {
    ex = x[0];
  }
  return ex;
}

//
// Arguments    : const double x[2]
// Return Type  : double
//
double minimum(const double x[2])
{
  double ex;
  if ((x[0] > x[1]) || (std::isnan(x[0]) && (!std::isnan(x[1])))) {
    ex = x[1];
  } else {
    ex = x[0];
  }
  return ex;
}

} // namespace internal
} // namespace coder

//
// File trailer for minOrMax.cpp
//
// [EOF]
//

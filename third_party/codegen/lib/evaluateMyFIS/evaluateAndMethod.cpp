//
// File: evaluateAndMethod.cpp
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-15 21:40:45
//

// Include Files
#include "evaluateAndMethod.h"
#include <cmath>

// Function Definitions
//
// Arguments    : const double x[4]
// Return Type  : double
//
namespace coder {
namespace fuzzy {
namespace internal {
namespace codegen {
double evaluateAndMethod(const double x[4])
{
  double y;
  int idx;
  int k;
  if (!std::isnan(x[0])) {
    idx = 1;
  } else {
    boolean_T exitg1;
    idx = 0;
    k = 2;
    exitg1 = false;
    while ((!exitg1) && (k <= 4)) {
      if (!std::isnan(x[k - 1])) {
        idx = k;
        exitg1 = true;
      } else {
        k++;
      }
    }
  }
  if (idx == 0) {
    y = x[0];
  } else {
    y = x[idx - 1];
    idx++;
    for (k = idx; k < 5; k++) {
      double d;
      d = x[k - 1];
      if (y > d) {
        y = d;
      }
    }
  }
  return y;
}

} // namespace codegen
} // namespace internal
} // namespace fuzzy
} // namespace coder

//
// File trailer for evaluateAndMethod.cpp
//
// [EOF]
//

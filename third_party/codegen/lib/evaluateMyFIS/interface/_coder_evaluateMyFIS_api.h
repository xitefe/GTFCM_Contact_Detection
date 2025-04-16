//
// File: _coder_evaluateMyFIS_api.h
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-15 21:40:45
//

#ifndef _CODER_EVALUATEMYFIS_API_H
#define _CODER_EVALUATEMYFIS_API_H

// Include Files
#include "emlrt.h"
#include "mex.h"
#include "tmwtypes.h"
#include <algorithm>
#include <cstring>

// Variable Declarations
extern emlrtCTX emlrtRootTLSGlobal;
extern emlrtContext emlrtContextGlobal;

// Function Declarations
real_T evaluateMyFIS(real_T x[4]);

void evaluateMyFIS_api(const mxArray *const prhs[2], const mxArray **plhs);

void evaluateMyFIS_atexit();

void evaluateMyFIS_initialize();

void evaluateMyFIS_terminate();

void evaluateMyFIS_xil_shutdown();

void evaluateMyFIS_xil_terminate();

#endif
//
// File trailer for _coder_evaluateMyFIS_api.h
//
// [EOF]
//

//
// File: _coder_evaluateMyFIS_mex.cpp
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-15 21:40:45
//

// Include Files
#include "_coder_evaluateMyFIS_mex.h"
#include "_coder_evaluateMyFIS_api.h"

// Function Definitions
//
// Arguments    : int32_T nlhs
//                mxArray *plhs[]
//                int32_T nrhs
//                const mxArray *prhs[]
// Return Type  : void
//
void mexFunction(int32_T nlhs, mxArray *plhs[], int32_T nrhs,
                 const mxArray *prhs[])
{
  mexAtExit(&evaluateMyFIS_atexit);
  // Module initialization.
  evaluateMyFIS_initialize();
  // Dispatch the entry-point.
  unsafe_evaluateMyFIS_mexFunction(nlhs, plhs, nrhs, prhs);
  // Module termination.
  evaluateMyFIS_terminate();
}

//
// Arguments    : void
// Return Type  : emlrtCTX
//
emlrtCTX mexFunctionCreateRootTLS()
{
  emlrtCreateRootTLSR2022a(&emlrtRootTLSGlobal, &emlrtContextGlobal, nullptr, 1,
                           nullptr, "GBK", true);
  return emlrtRootTLSGlobal;
}

//
// Arguments    : int32_T nlhs
//                mxArray *plhs[1]
//                int32_T nrhs
//                const mxArray *prhs[2]
// Return Type  : void
//
void unsafe_evaluateMyFIS_mexFunction(int32_T nlhs, mxArray *plhs[1],
                                      int32_T nrhs, const mxArray *prhs[2])
{
  emlrtStack st{
      nullptr, // site
      nullptr, // tls
      nullptr  // prev
  };
  const mxArray *b_prhs[2];
  const mxArray *outputs;
  st.tls = emlrtRootTLSGlobal;
  // Check for proper number of arguments.
  if (nrhs < 2) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooFewInputsConstants", 9, 4, 13,
                        "evaluateMyFIS", 4, 13, "evaluateMyFIS", 4, 13,
                        "evaluateMyFIS");
  }
  if (nrhs != 2) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:WrongNumberOfInputs", 5, 12, 2, 4,
                        13, "evaluateMyFIS");
  }
  if (nlhs > 1) {
    emlrtErrMsgIdAndTxt(&st, "EMLRT:runTime:TooManyOutputArguments", 3, 4, 13,
                        "evaluateMyFIS");
  }
  // Call the function.
  b_prhs[0] = prhs[0];
  b_prhs[1] = prhs[1];
  evaluateMyFIS_api(b_prhs, &outputs);
  // Copy over outputs to the caller.
  emlrtReturnArrays(1, &plhs[0], &outputs);
}

//
// File trailer for _coder_evaluateMyFIS_mex.cpp
//
// [EOF]
//

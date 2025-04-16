//
// File: main.cpp
//
// MATLAB Coder version            : 24.1
// C/C++ source code generated on  : 2025-04-15 21:40:45
//

/*************************************************************************/
/* This automatically generated example C++ main file shows how to call  */
/* entry-point functions that MATLAB Coder generated. You must customize */
/* this file for your application. Do not modify this file directly.     */
/* Instead, make a copy of this file, modify it, and integrate it into   */
/* your development environment.                                         */
/*                                                                       */
/* This file initializes entry-point function arguments to a default     */
/* size and value before calling the entry-point functions. It does      */
/* not store or use any values returned from the entry-point functions.  */
/* If necessary, it does pre-allocate memory for returned values.        */
/* You can use this file as a starting point for a main function that    */
/* you can deploy in your application.                                   */
/*                                                                       */
/* After you copy the file, and before you deploy it, you must make the  */
/* following changes:                                                    */
/* * For variable-size function arguments, change the example sizes to   */
/* the sizes that your application requires.                             */
/* * Change the example values of function arguments to the values that  */
/* your application requires.                                            */
/* * If the entry-point functions return values, store these values or   */
/* otherwise use them as required by your application.                   */
/*                                                                       */
/*************************************************************************/

// Include Files
#include "main.h"
#include "evaluateMyFIS.h"
#include "evaluateMyFIS_terminate.h"

// Function Declarations
static void argInit_1x4_real_T(double result[4]);

static double argInit_real_T();

// Function Definitions
//
// Arguments    : double result[4]
// Return Type  : void
//
static void argInit_1x4_real_T(double result[4])
{
  // Loop over the array to initialize each element.
  for (int idx1{0}; idx1 < 4; idx1++) {
    // Set the value of the array element.
    // Change this value to the value that the application requires.
    result[idx1] = argInit_real_T();
  }
}

//
// Arguments    : void
// Return Type  : double
//
static double argInit_real_T()
{
  return 0.0;
}

//
// Arguments    : int argc
//                char **argv
// Return Type  : int
//
int main(int, char **)
{
  // The initialize function is being called automatically from your entry-point
  // function. So, a call to initialize is not included here. Invoke the
  // entry-point functions.
  // You can call entry-point functions multiple times.
  main_evaluateMyFIS();
  // Terminate the application.
  // You do not need to do this more than one time.
  evaluateMyFIS_terminate();
  return 0;
}

//
// Arguments    : void
// Return Type  : void
//
void main_evaluateMyFIS()
{
  double dv[4];
  double y;
  // Initialize function 'evaluateMyFIS' input arguments.
  // Initialize function input argument 'x'.
  // Call the entry-point 'evaluateMyFIS'.
  argInit_1x4_real_T(dv);
  y = evaluateMyFIS(dv);
}

//
// File trailer for main.cpp
//
// [EOF]
//

#ifndef TMWTYPES_H
#define TMWTYPES_H

#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

/* 基本类型定义 */
typedef double real_T;
typedef float real32_T;
typedef int int_T;
typedef unsigned int uint_T;
typedef unsigned char boolean_T;
typedef char char_T;
typedef unsigned char uchar_T;
typedef short int16_T;
typedef unsigned short uint16_T;
typedef int int32_T;
typedef unsigned int uint32_T;
typedef long long int64_T;
typedef unsigned long long uint64_T;
typedef unsigned char uint8_T;
typedef signed char int8_T;

/* 复数类型 */
typedef struct {
  real_T re;
  real_T im;
} creal_T;

typedef struct {
  real32_T re;
  real32_T im;
} creal32_T;

/* 指针类型 */
typedef void * pointer_T;

#endif /* TMWTYPES_H */
#ifndef _OCL_DEBUG_
#define _OCL_DEBUG_

#include <stdio.h>

//#define OCL_DEBUG_ON

#ifdef OCL_DEBUG_ON
#define ocl_debug(x)      printf("[OCL] " x "\n")
#define ocl_debug1(x,y)   printf("[OCL] " x "\n", y)
#define ocl_debug2(x,y,z) printf("[OCL] " x "\n", y, z)
#else
#define ocl_debug(x) 
#define ocl_debug1(x,y) 
#define ocl_debug2(x,y,z) 
#endif

#endif //_OCL_DEBUG_

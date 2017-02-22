#ifndef _HOST_ASSIST_H_
#define _HOST_ASSIST_H_

#include <CL/cl.hpp>
#include <stdint.h>

/*-----------------------------------------------------------------------------
* Prototypes
*----------------------------------------------------------------------------*/
int  assert_am57x         (cl::Device &device);
void print_completion_code(uint32_t completion_code);

#endif // _HOST_ASSIST_H_

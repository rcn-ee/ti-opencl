/* pocl.h - global pocl declarations.

   Copyright (c) 2011 Universidad Rey Juan Carlos
                 2011-2014 Pekka Jääskeläinen / Tampere University of Technology
   Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
  
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

/**
 * @file pocl.h
 * 
 * The declarations in this file are such that are used both in the
 * libpocl implementation CL and the kernel compiler. Others should be
 * moved to pocl_cl.h of lib/CL or under the kernel compiler dir. 
 * @todo Check if there are extra declarations here that could be moved.
 */
#ifndef POCL_H
#define POCL_H

/*
 * During pocl kernel compiler transformations we use the fixed address 
 * space ids of clang's -ffake-address-space-map to mark the different 
 * address spaces to keep the processing target-independent. These
 * are converted to the target's address space map (if any), in a final
 * kernel compiler pass.
 */
#define POCL_ADDRESS_SPACE_PRIVATE 0
#define POCL_ADDRESS_SPACE_GLOBAL 1
#define POCL_ADDRESS_SPACE_LOCAL 2
#define POCL_ADDRESS_SPACE_CONSTANT 3

#endif /* POCL_H */

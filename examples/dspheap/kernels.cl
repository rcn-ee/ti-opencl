/******************************************************************************
 * Copyright (c) 2014, Texas Instruments Incorporated - http://www.ti.com/
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions are met:
 *       * Redistributions of source code must retain the above copyright
 *         notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *         notice, this list of conditions and the following disclaimer in the
 *         documentation and/or other materials provided with the distribution.
 *       * Neither the name of Texas Instruments Incorporated nor the
 *         names of its contributors may be used to endorse or promote products
 *         derived from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

/*-----------------------------------------------------------------------------
* These kernels initialize user controlled heaps,  they do not have to be 
* separate kernels.  The call to __heap_init_xxx can be rolled into an existing
* kernel and called before any __malloc_xxx calls are made.
*
* These heaps can be persistent across kernel boundaries as long as the 
* underlying memory (aka buffers pointed to by p are not deallocated.
*----------------------------------------------------------------------------*/
kernel void heap_init_ddr(__global void *p, size_t bytes) 
{
    printf("DDR  heap pointer is 0x%08x\n", p);
    __heap_init_ddr(p,bytes);
}

kernel void heap_init_msmc(__global void *p, size_t bytes) 
{
    printf("MSMC heap pointer is 0x%08x\n", p);
    __heap_init_msmc(p,bytes);
}

/*-----------------------------------------------------------------------------
* This kernel will allocate from tthe heaps and then free them memory.
*----------------------------------------------------------------------------*/
kernel void alloc_and_free(int ddr_bytes, int msmc_bytes)
{
    if(ddr_bytes > 0)
    {
        char *p1 = __malloc_ddr(ddr_bytes);

        if(p1)
            printf("DDR  alloc+free pointer is 0x%08x\n", p1);
        else
            printf("DDR  alloc+free ERROR FAILED ALLOCATION\n");

        __free_ddr(p1);
    }

    if(msmc_bytes > 0)
    {
        char *p2 = __malloc_msmc(msmc_bytes);

        if(p2)
            printf("MSMC alloc+free pointer is 0x%08x\n", p2);
        else
            printf("MSMC alloc+free ERROR FAILED ALLOCATION\n");

        __free_msmc(p2);
    }
}

/*-----------------------------------------------------------------------------
* This kernel will allocate from the heaps and the memory is not freed. The 
* active pointers p1 and p2 could be returned to the host application, via 
* output arguments to the kernel.  They could then subsequently be passed to 
* other kernels.  However the values should not be dereferenced on the host,
* because the DSP addresses are not valid linux system addresses.
*
* Additionally, if you do maintain a malloced block across kernel boundaries,
* depending on how the kernels are enqueued you may not know which core will 
* subsequently be passed the pointer and access the memory, therefore you
* will need to manage cache coherency manually.  This method of communicating 
* across kernels is not recommended for this reason.  Passing a buffer from 
* the host to kernel 1 that populates the buffer and then passing the buffer 
* to kernel 2 where the buffer is read is the preferred method for
* communicated across kernels, because it is portable and cache operations 
* are managed automatically.
*----------------------------------------------------------------------------*/
kernel void alloc_only(int ddr_bytes, int msmc_bytes)
{
    if(ddr_bytes > 0)
    {
        char *p1 = __malloc_ddr(ddr_bytes);

        if(p1)
            printf("DDR  alloc pointer is 0x%08x\n", p1);
        else
            printf("DDR  alloc ERROR FAILED ALLOCATION\n");
    }

    if(msmc_bytes > 0)
    {
        char *p2 = __malloc_msmc(msmc_bytes);

        if(p2)
            printf("MSMC alloc pointer is 0x%08x\n", p2);
        else
            printf("MSMC alloc ERROR FAILED ALLOCATION\n");
    }
}

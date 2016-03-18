/**
 *   @file  mmap_resource.c
 *
 *   @brief
 *      Source file for the KEYSTONE_MMAP component which relevent to the allocation of 
 *      resources such as logical addresses and MPAX registers.
 *
 *  \par
 *  NOTE:
 *      (C) Copyright 2013 Texas Instruments, Inc.
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#if defined(DEVICE_K2X) || defined(DEVICE_K2G)

extern "C" {

#include <stdint.h>
#include <string.h>
#include <stdio.h>


/*-----------------------------------------------------------------------------
* The file ti/runtime/mmap/mmap.h has a missing '}' when compiled with C++.  
* We do this as a workaraound for now.
*----------------------------------------------------------------------------*/
#ifdef __cplusplus
#undef __cplusplus
#include <ti/runtime/mmap/mmap.h>
#define __cplusplus
#else
#include <ti/runtime/mmap/mmap.h>
#endif

#include <ti/runtime/mmap/include/mmap_resource.h>

#define KEYSTONE_MMAP_MIN_LENGTH        0x1000   //This can be reduced to 4K, but then cache handling needs enhancement.
#define KEYSTONE_MMAP_LENGTH_THRESHOLD  0x1000000

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

/* Round up to the nearest power of 2 value */
static inline uint32_t power2_ceil(uint32_t in)
{
  uint32_t out = 1;

  in--;

  /* Round up size to next power of 2 */
  while ( in )
  {
    in  >>= 1;
    out <<= 1;
  }
  return (out);
}


/* Heap subroutines */

/**
 *  @b  Initialize the heap.
 *
 *  @n  This function beings by finding the best power of 2 size and aligned 
 *      address space which contains the requested heap space. Then the excess 
 *      reserved from the heap.
 */
static void     heap_init ( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t addr, uint32_t size);

/**
 *  @b  Allocates a chunk of the address space.
 *
 *  @n  Recursively searches the heap to find a free element. This sub routine 
 *      requires that the length be a power of 2.
 */
static uint32_t heap_alloc( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t size, uint32_t protections);

/**
 *  @b  Deallocates a chunk of the address space.
 *
 *  @n  Searches the leaf nodes of the heap to find the element which contains 
 *      addr.
 */
static uint32_t heap_free ( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t addr);

/**
 *  @b  Prints the current state of the heap.
 *
 *  @n  This function is intended for debugging and it will print out the 
 *      information of the all of the leaf nodes of the heap tree.
 */
static void     heap_status(keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, int32_t depth);

/**
 *  @b  Prints out all valid mappings maintained by KEYSTONE_MMAP.
 */
static void     map_status(keystone_mmap_mapping_t *mapping);



/**
 *  @b  Description
 *
 *  @n
 *      This function prints out debug information on the KEYSTONE_MMAP resources.
 *
 *  @param[in]    function_name    Name of the calling function.
 *
 *  @param[in]    resources        KEYSTONE_MMAP resource structure.
 */
void keystone_mmap_resource_debug(const char *function_name, keystone_mmap_resources_t *resources)
{
  int32_t i, num_heap_elems_used = 0;

  printf("\n\n----------KEYSTONE_MMAP_RESOURCE_DEBUG Begin----------\n");
  printf("\n%s\n\n", function_name);

  for ( i = 0; i < resources->num_heaps; i++)
  {
    printf("Heap %d: \n", i);
    heap_status(resources->heap, i, 0);
    printf("\n");
  }
  printf("\n");

  for ( i = 0; i < KEYSTONE_MMAP_MAX_NUM_HEAP_ELEMS; i++)
  {
    if ( resources->heap[i].in_use )
      num_heap_elems_used++;
  }
  printf("Using %d / %d heap elements..\n\n\n", num_heap_elems_used, KEYSTONE_MMAP_MAX_NUM_HEAP_ELEMS);

  map_status(resources->mapping);
  printf("\n----------KEYSTONE_MMAP_RESOURCE_DEBUG End------------\n\n");

} /* keystone_mmap_resource_debug() */



/**
 *  @b  Description
 *
 *  @n
 *      This function initializes the data structures which manage the resources
 *      used for mapping. These resources are the address space heaps and MPAX 
 *      registers used for mapping.
 *
 *  @param[in]  num_regs   Number of XMC and SES register pairs available for 
 *                         mapping.
 *
 *  @param[in]  xmc_regs   Array of XMC register indexes which are available for 
 *                         mapping. XMC registers allow the DSP core to access 
 *                         the mapped memory.
 *
 *  @param[in]  ses_regs   Array of SES register indexes which are available for
 *                         mapping. SES registers allows EDMA to access the 
 *                         mapped memory.
 *
 *  @param[in]  num_heaps  Number of contiguous address space ranges which are 
 *                         available for mapping.
 *
 *  @param[in]  heap_base  Array of base addresses of each contiguous address 
 *                         space range.
 *
 *  @param[in]  heap_size  Array of sizes of each contiguous address space range.
 *
 *  @param[out] resources  KEYSTONE_MMAP resource strucutre which manages the resources 
 *                         used for mapping.
 *
 *  @retval     KEYSTONE_MMAP_RESOURCE_NOERR on success. Other values correspond to an 
 *              error condition. (@sa )
 */
int32_t keystone_mmap_resource_init
(
  uint32_t            num_regs, 
  uint32_t           *xmc_regs, 
  uint32_t           *ses_regs, 
  uint32_t            num_heaps, 
  uint32_t           *heap_base, 
  uint32_t           *heap_size, 
  keystone_mmap_resources_t   *resources
)
{
  int32_t i;

  if ( num_regs > KEYSTONE_MMAP_MAX_NUM_MAPS)
    return KEYSTONE_MMAP_RESOURCE_INIT_ERROR;

  memset(resources, 0, sizeof(keystone_mmap_resources_t));

  /* Pair XMC and SES registers for MPAX */
  for ( i = 0; i < num_regs; i++)
  {
    resources->mapping[i].xmc_idx = xmc_regs[i];
    resources->mapping[i].ses_idx = ses_regs[i];
  }
  for ( ; i < KEYSTONE_MMAP_MAX_NUM_MAPS; i++)
  {
    resources->mapping[i].xmc_idx = -1;
    resources->mapping[i].ses_idx = -1;
  }

  /* Claim first 'num_heap' heap elements for the root of each heap */
  for ( i = 0; i < num_heaps; i++)
  {
    resources->heap[i].in_use = 1;
  }
  resources->num_heaps = num_heaps;

  for ( i = 0; i < num_heaps; i++)
  {
    /* Verify heap spaces are aligned to MAR windows. */
    if ( ((heap_base[i] & (KEYSTONE_MMAP_HEAP_ALIGN - 1)) != 0) ||
         ((heap_size[i] & (KEYSTONE_MMAP_HEAP_ALIGN - 1)) != 0) )
    {
      break;
    }

    /* Initialize the heap. */
    heap_init( resources->heap, i, heap_base[i], heap_size[i]);
  }
  if ( i < num_heaps )
  {
    /* Clean up if needed */

    return KEYSTONE_MMAP_RESOURCE_INIT_ERROR;
  }

#ifdef KEYSTONE_MMAP_RESOURCE_DEBUG
  keystone_mmap_resource_debug("keystone_mmap_resource_init()", resources);
#endif
  return KEYSTONE_MMAP_RESOURCE_NOERR;
} /* keystone_mmap_resource_inint() */



/**
 *  @b  Description
 *
 *  @n  
 *      This function frees resources associated to mapped physical buffers, The 
 *      virtual addresses and lengths are supplied to this function and the 
 *      corresponding mapping entries are removed.
 *
 *  @param[in]     num_bufs   number of buffers which are to be unmapped.
 *
 *  @param[in]     virt_addrs Array of virtual address of the buffers which are 
 *                            to be unmapped.
 *
 *  @param[in]     lengths    Array of lengths of the buffers which are to be 
 *                            unmapped.
 *
 *  @param[in,out] resources  Resource structure managing the resources used for 
 *                            mapping.
 *
 *  @retval        KEYSTONE_MMAP_RESOURCE_NOERR on success. Other values correspond to an 
 *                 error condition. (@sa )
 */
int32_t keystone_mmap_resource_free
( 
  uint32_t            num_bufs, 
  uint32_t           *virt_addrs, 
  uint32_t           *lengths, 
  keystone_mmap_resources_t   *resources
)
{
  int32_t buf_idx, heap_idx, map_idx, num_unmap;

  for ( buf_idx = 0; buf_idx < num_bufs; buf_idx++)
  {
    /* Free the virtual address back to the heap. */
    for ( heap_idx = 0; heap_idx < resources->num_heaps; heap_idx++)
    {
      if ( heap_free(resources->heap, heap_idx, virt_addrs[buf_idx]) > 0 )
        break;
    }
    if ( heap_idx >= resources->num_heaps )
    {
      /* Could not free */
      break;
    }

    /* Remove mappings from map table */
    num_unmap = 0;
    for ( map_idx = 0; map_idx < KEYSTONE_MMAP_MAX_NUM_MAPS; map_idx++ )
    {
      keystone_mmap_mapping_t *mapping = &resources->mapping[map_idx];

      /**
       *   A single buffer may require 2 mappings to save address space resource. 
       *
       *   Check if beginning or end of mapped region is within the actual mapping. 
       */
      if ( 
           ( 
             (virt_addrs[buf_idx] >= mapping->baddr) &&
             (virt_addrs[buf_idx] <  mapping->baddr + (1 << mapping->segsize_power2)) 
           ) 
           ||
           ( 
             (virt_addrs[buf_idx] + lengths[buf_idx] > mapping->baddr) &&
             (virt_addrs[buf_idx] + lengths[buf_idx] <= mapping->baddr + (1 << mapping->segsize_power2)) 
           ) 
         )
      {
        /* Reset mapping. */
        mapping->raddr           = 0;
        mapping->baddr           = 0;
        mapping->segsize_power2  = 0;
        mapping->prot            = 0;

        num_unmap++;
      }
    }
    if ( num_unmap == 0 || num_unmap > 2 )
    {
      /* Error unmapping */
      break;
    }
  }
  if ( buf_idx < num_bufs )
  {
    /* Error freeing one of the mappings. */
    return KEYSTONE_MMAP_RESOURCE_FREE_ERROR;
  }

#ifdef KEYSTONE_MMAP_RESOURCE_DEBUG
  keystone_mmap_resource_debug("keystone_mmap_resource_free()", resources);
#endif
  return KEYSTONE_MMAP_RESOURCE_NOERR;
} /* keystone_mmap_resource_free() */



/**
 *  @b  Description
 *
 *  @n
 *      This functions takes physical buffers and lengths and allocates the 
 *      resources required to map all of the buffer. If the allocation is 
 *      successful, the array of addressable virtual address is populated along 
 *      with the physical mapping configuration.
 *
 *  @param[in]     num_bufs     Number of buffers for which to allocate mapping 
 *                              resources.
 *
 *  @param[in]     phys_addrs   Array of physical addresses of the base of each 
 *                              buffer.
 *
 *  @param[in]     lengths      Array of lengths for each buffer.
 *
 *  @param[in]     protections  Array containing the read/write/execute 
 *                              permissions along with cacheability.
 *
 *  @param[out]    virt_addrs   An array which, upon succesful resource 
 *                              allocation, will be populated with the virtual 
 *                              addresses corresponding to the physical buffers.
 * 
 *  @param[in,out] resources    Resource structure which manages the mapping 
 *                              resources. This structure contains a 
 *                              keystone_mmap_mapping_t structure which is the table of 
 *                              mapping information which should be passed to 
 *                              keystone_mmap_do_map() to perform the actual mapping.
 *
 *  @retval        KEYSTONE_MMAP_RESOURCE_NOERR on success. Other values correspond to an 
 *                 error condition. (@sa )
 */
int32_t keystone_mmap_resource_alloc
( 
  uint32_t            num_bufs, 
  uint64_t           *phys_addrs, 
  uint32_t           *lengths, 
  uint32_t           *protections, 
  uint32_t           *virt_addrs, 
  keystone_mmap_resources_t   *resources
)
{
  int32_t buf_idx, heap_idx, map_idx, k;

  for ( buf_idx = 0; buf_idx < num_bufs; buf_idx++)
  {
    uint64_t aligned_phy;
    uint32_t aligned_len, tmp_aligned_len;
    int32_t  num_mappings = 1;
    int8_t   segsize_power2 = 0;

    /* Enforce alignment requirements */
    aligned_len = power2_ceil( phys_addrs[buf_idx] ^ (phys_addrs[buf_idx] + lengths[buf_idx] - 1) );
    aligned_len = MAX(KEYSTONE_MMAP_MIN_LENGTH, aligned_len);
    aligned_phy = phys_addrs[buf_idx] & (~((uint64_t)aligned_len - 1));

    if ( (aligned_len > KEYSTONE_MMAP_LENGTH_THRESHOLD) && (aligned_len / lengths[buf_idx] > 1) )
    {
      /* Mapping will be more efficient on resources if 2 mappings are used. */
      aligned_phy  += (aligned_len >> 1);
      aligned_len   = aligned_phy - phys_addrs[buf_idx];
      aligned_len   = MAX( aligned_len, (phys_addrs[buf_idx] + lengths[buf_idx] - aligned_phy));
      aligned_len   = power2_ceil(aligned_len);
      aligned_len   = MAX(KEYSTONE_MMAP_MIN_LENGTH,aligned_len);
      aligned_phy  -= aligned_len;

      num_mappings  = 2;
    }

    /* Allocate address space for mapping. */
    virt_addrs[buf_idx] = 0;
    for ( heap_idx = 0; heap_idx < resources->num_heaps; heap_idx++)
    {
      virt_addrs[buf_idx] = heap_alloc(resources->heap, heap_idx, num_mappings*aligned_len, protections[buf_idx]);
      if ( virt_addrs[buf_idx] != 0 )
        break;
    }
    if ( heap_idx >= resources->num_heaps )
      break;

    /* Get the log2 of the size of the mapping. */
    tmp_aligned_len = aligned_len - 1;
    while (tmp_aligned_len )
    {
      tmp_aligned_len >>= 1;
      segsize_power2++;
    }

    /* Allocate and fill entries in the mapping table. */
    k = 0;
    for ( map_idx = 0; map_idx < num_mappings; map_idx++)
    {
      for ( ; k < KEYSTONE_MMAP_MAX_NUM_MAPS; k++)
      {
        if ( resources->mapping[k].xmc_idx >= 0 && 
             resources->mapping[k].segsize_power2 == 0 )
        {
          break;
        }
      }
      if ( k >= KEYSTONE_MMAP_MAX_NUM_MAPS )
      {
        /* No available mappings */
        break;
      }

      resources->mapping[k].raddr           = aligned_phy  + map_idx*aligned_len;
      resources->mapping[k].baddr           = virt_addrs[buf_idx] + map_idx*aligned_len;
      resources->mapping[k].segsize_power2  = segsize_power2;
      resources->mapping[k].prot            = protections[buf_idx];
    }
    if ( map_idx < num_mappings )
    {
      /* Not enough entries in mapping table, free address space */
      heap_free( resources->heap, heap_idx, virt_addrs[buf_idx]);
      break;
    }
    virt_addrs[buf_idx] += (phys_addrs[buf_idx] - aligned_phy);
  }
  if ( buf_idx < num_bufs )
  {
    /* Could not allocate all of the required resources. Free the ones that were successful. */
    keystone_mmap_resource_free(buf_idx,virt_addrs,lengths,resources);
    return KEYSTONE_MMAP_RESOURCE_ALLOC_ERROR;
  }

#ifdef KEYSTONE_MMAP_RESOURCE_DEBUG
  keystone_mmap_resource_debug("keystone_mmap_resource_alloc()", resources);
#endif
  return KEYSTONE_MMAP_RESOURCE_NOERR;
} /* keystone_mmap_resource_alloc() */


/* Begin heap subroutines */

/**
 *  @b Allocate heap element frmo the heap element bank.  
 */
static unsigned int get_heap_element(keystone_mmap_heap_element_t *elem_bank)
{
  int i;

  for ( i = 0; i < KEYSTONE_MMAP_MAX_NUM_HEAP_ELEMS; i++)
  {
    if ( elem_bank[i].in_use == 0)
    {
      elem_bank[i].in_use = 1;
      break;
    }
  }
  if ( i >= KEYSTONE_MMAP_MAX_NUM_HEAP_ELEMS ) {
    printf("\n\tNo more heap elems...\n\n");
    getchar();
    return 0;
  }
  return (i);
} /* get_heap_element() */


/**
 *  @b  Free heap element back to the heap element bank. 
 */
static void free_heap_element( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];

  memset(elem,0,sizeof(keystone_mmap_heap_element_t));
} /* free_heap_element() */


/**
 *  @b  Split an element into 2 sub elements, each with half the size. 
 */
static void heap_split_elem( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];
  keystone_mmap_heap_element_t *sub_elem;

  if ( elem->sub_elem[0] == 0 )
  {
    uint32_t sub_size = elem->length >> 1;

    elem->sub_elem[0] = get_heap_element(elem_bank);
    sub_elem = &elem_bank[elem->sub_elem[0]];
    sub_elem->base_addr = elem->base_addr;
    sub_elem->length    = sub_size;

    elem->sub_elem[1] = get_heap_element(elem_bank);
    sub_elem = &elem_bank[elem->sub_elem[1]];
    sub_elem->base_addr = elem->base_addr + sub_size;
    sub_elem->length    = sub_size;
  }
} /* heap_split_elem() */


/**
 *  @b  Reserve parts of the address space from the heap. 
 *
 *  @n  Recursively parses the heap to mark all elements which are contained in 
 *      the reserved region as allocated.
 */
static uint32_t heap_reserve( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t addr, uint32_t size)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];
  keystone_mmap_heap_element_t *sub_elem;
  uint32_t total_reserved_size = 0, reserved_size;

  if ( size == 0 )
    return 0;

  if ( (elem->base_addr == addr) && (elem->length <= size)  )
  {
    /* Mark elem as allocated. */
    elem->allocated = 1;
    total_reserved_size = elem->length;
  }
  else if ( (elem->base_addr <= addr) && ((elem->base_addr + elem->length - 1) >= (addr)) )
  {
    heap_split_elem(elem_bank, elem_idx);

    /* Try to reserve from first sub element. */
    reserved_size = heap_reserve( elem_bank, elem->sub_elem[0], addr, size);
    if ( reserved_size > 0 )
    {
      sub_elem = &elem_bank[elem->sub_elem[0]];

      addr += reserved_size;
      size -= reserved_size;
      total_reserved_size += reserved_size;
      elem->allocated += sub_elem->allocated;
    }

    /* Try to reserve from second sub element. */
    reserved_size = heap_reserve( elem_bank, elem->sub_elem[1], addr, size);
    if ( reserved_size > 0 )
    {
      sub_elem = &elem_bank[elem->sub_elem[1]];

      addr += reserved_size;
      size -= reserved_size;
      total_reserved_size += reserved_size;
      elem->allocated += sub_elem->allocated;
    }
  }
  /* Return total amount of size successfully reserved. */
  return ( total_reserved_size);
} /* heap_reserve() */


/**
 *  @b  Initialize the heap.
 *
 *  @n  This function beings by finding the best power of 2 size and aligned 
 *      address space which contains the requested heap space. Then the excess 
 *      reserved from the heap.
 */
static void heap_init(keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t addr, uint32_t size)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];
  uint32_t base_addr, norm_size = power2_ceil(size);

  /* Enforce power of 2 size and alignment */
  base_addr = addr;
  norm_size = size;

  norm_size = power2_ceil(addr ^ (addr + size - 1));
  base_addr = addr & (~(norm_size - 1));

  elem->base_addr = base_addr;
  elem->length    = norm_size;

  heap_reserve( elem_bank, elem_idx, base_addr, addr - base_addr);

  heap_reserve( elem_bank, elem_idx, addr + size, (base_addr + norm_size) - (addr + size));

} /* heap_init() */


/**
 *  @b  Allocates a chunk of the address space.
 *
 *  @n  Recursively searches the heap to find a free element. This sub routine 
 *      requires that the length be a power of 2.
 */
static uint32_t heap_alloc( keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t size, uint32_t protections)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];
  uint32_t addr = 0;

  /* Cache can be configured for 16 MB windows */
  if (elem->length <= 0x1000000 )
  {
    if ( (elem->allocated > 0) && ((protections & KEYSTONE_MMAP_PROT_NOCACHE) != (elem->prot & KEYSTONE_MMAP_PROT_NOCACHE)) )
      return (addr);
  }

  if ( size == elem->length )
  {
    /* Reached the farthest depth */
    if ( elem->allocated == 0 )
    {
      elem->allocated++;
      elem->prot = protections;
      addr = elem->base_addr;
    }
  }
  else if ( (size < elem->length) && (elem->sub_elem[0] != 0 || elem->allocated == 0) )
  {
    heap_split_elem(elem_bank, elem_idx);

    if ( ( (addr = heap_alloc( elem_bank, elem->sub_elem[0], size, protections)) != 0) ||
         ( (addr = heap_alloc( elem_bank, elem->sub_elem[1], size, protections)) != 0) )
    {
      elem->allocated++;
      if ( elem->length <= 0x1000000 )
      {
        elem->prot = protections & KEYSTONE_MMAP_PROT_NOCACHE;
      }
    }
  }

  return (addr);
} /* heap_alloc() */


/**
 *  @b  Deallocates a chunk of the address space.
 *
 *  @n  Searches the leaf nodes of the heap to find the element which contains 
 *      addr.
 */
static uint32_t heap_free(keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, uint32_t addr)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];
  keystone_mmap_heap_element_t *sub_elem;
  uint32_t ret_val = 0;

  if (elem->sub_elem[0] == 0 )
  {
//    if ( elem->base_addr == addr )
    if ( (elem->base_addr <= addr) && (elem->base_addr + elem->length - 1 >= addr) )
    {
//      printf("\theap_free() : Found element with size 0x%08X\n", elem->length);
      elem->allocated--;
      elem->prot = KEYSTONE_MMAP_PROT_NONE;

      ret_val = 1;
    }
  }
  else
  {
    sub_elem = &elem_bank[elem->sub_elem[1]];
    if ( (sub_elem->base_addr & addr) == sub_elem->base_addr )
    {
      ret_val = heap_free( elem_bank, elem->sub_elem[1], addr);
    }
    else
    {
      ret_val = heap_free( elem_bank, elem->sub_elem[0], addr);
    }
    elem->allocated -= ret_val;

    if ( elem->allocated == 0 )
    {
      free_heap_element(elem_bank, elem->sub_elem[0]);
      elem->sub_elem[0] = 0;
      free_heap_element(elem_bank, elem->sub_elem[1]);
      elem->sub_elem[1] = 0;

      elem->prot = KEYSTONE_MMAP_PROT_NONE;
    }
  }
  return (ret_val);
} /* heap_free() */


/**
 *  @b  Prints the current state of the heap.
 *
 *  @n  This function is intended for debugging and it will print out the 
 *      information of the all of the leaf nodes of the heap tree.
 */
static void heap_status(keystone_mmap_heap_element_t *elem_bank, uint32_t elem_idx, int32_t depth)
{
  keystone_mmap_heap_element_t *elem = &elem_bank[elem_idx];

  if ( elem->sub_elem[0] == 0 )
  {
    if ( elem->allocated > 0 && elem->sub_elem[0] == 0 )
      printf("+ ");
    else
      printf("- ");

    printf("Base : %08X, Size %08X, Num Alloc: %3d, Protections: %02X ", elem->base_addr, elem->length, elem->allocated, elem->prot);

    if ( elem->allocated > 0 && (elem->length <= 0x1000000 || elem->sub_elem[0] == 0) )
    {
      if ( (elem->prot & KEYSTONE_MMAP_PROT_NOCACHE) == KEYSTONE_MMAP_PROT_NOCACHE )
        printf("NON");

      printf("CACHED");
    }
    printf("\n");
  }
  else
  {
    heap_status(elem_bank, elem->sub_elem[0], depth+1);
    heap_status(elem_bank, elem->sub_elem[1], depth+1);
  }
} /* heap_status() */


/**
 *  @b  Prints out all valid mappings maintained by KEYSTONE_MMAP.
 */
static void map_status(keystone_mmap_mapping_t *mapping)
{
  uint32_t length;
  int32_t map_idx;

  for ( map_idx = 0; map_idx < KEYSTONE_MMAP_MAX_NUM_MAPS; map_idx++)
  {
    if ( mapping[map_idx].xmc_idx >= 0 && mapping[map_idx].ses_idx >= 0 )
    {
      if ( mapping[map_idx].segsize_power2 ) {
        length = (1 << mapping[map_idx].segsize_power2);

        printf("Map %d:\n", map_idx);
        printf("\tPhys Addr   : 0x%016llX\n", mapping[map_idx].raddr);
        printf("\tBase Addr   : 0x%08X\n", mapping[map_idx].baddr);
        printf("\tLength      : 0x%08X\n", length);
        printf("\tProtections : ");
        if ( (mapping[map_idx].prot & KEYSTONE_MMAP_PROT_READ) == KEYSTONE_MMAP_PROT_READ )
          printf("R");
        if ( (mapping[map_idx].prot & KEYSTONE_MMAP_PROT_WRITE) == KEYSTONE_MMAP_PROT_WRITE )
          printf("W");
        if ( (mapping[map_idx].prot & KEYSTONE_MMAP_PROT_EXEC) == KEYSTONE_MMAP_PROT_EXEC )
          printf("X");
        printf("\n");
      }
    }
  }
} /* map_status() */

}

#endif  // #if defined(DEVICE_K2X) || defined(DEVICE_K2G)


/******************************************************************************
 * Copyright (c) 2016, Texas Instruments Incorporated - http://www.ti.com/
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

#include "dload_impl.h"
#include "device.h"
#include "driver.h"
#include "program.h"
#include "../../shared_memory_interface.h"

extern "C" {
#include "dload_api.h"
}


using Coal::DSPDevice;
using Coal::DSPProgram;

using namespace tiocl;

DLOAD::DLOAD(DSPProgram *program):
                programLoadAddress(0), ph(-1)
{
    Lock lock(this);
    dloadHandle = DLOAD_create((void*)program);
}

DLOAD::~DLOAD()
{
    DLOAD_destroy(dloadHandle);
    dloadHandle = 0;
}

bool DLOAD::LoadProgram(const std::string &fileName)
{
    Lock lock(this);

    FILE *fp = fopen(fileName.c_str(), "rb");

    //TODO: Can we propagate the error to DeviceProgram instead of exiting?
    if (!fp) { printf("can't open OpenCL Program file\n"); exit(1); }

    ph = DLOAD_load(dloadHandle, fp);

    fclose(fp);

    // DLOAD_load returns 0 if it fails
    if (ph == 0)
        return false;

    return true;
}

bool DLOAD::UnloadProgram()
{
    if (dloadHandle)
    {
        assert (ph != 0);
        Lock lock(this);
        bool retval = DLOAD_unload(dloadHandle, ph);

        return retval;
    }
    return false;
}

DSPDevicePtr DLOAD::QuerySymbol(const std::string &symName) const
{
    if (!dloadHandle || !ph)
        return 0;

    DSPDevicePtr addr = 0;

    bool found = DLOAD_query_symbol(dloadHandle, ph,
                                    symName.c_str(), &addr);

    return (found) ? addr : 0;
}

DSPDevicePtr DLOAD::GetDataPagePointer() const
{
    if (!dloadHandle || !ph)
        return 0;

    DSPDevicePtr p = 0;
    DLOAD_get_static_base(dloadHandle, ph,  &p);
    return p;
}

DSPDevicePtr DLOAD::GetProgramLoadAddress() const
{
    return programLoadAddress;
}

void DLOAD::SetProgramLoadAddress(DSPDevicePtr address)
{
    programLoadAddress = address;
}

/******************************************************************************
* Call back functions from the target loader
******************************************************************************/
extern "C"
{

static bool load_kernels_onchip()
{
   return (getenv("TI_OCL_LOAD_KERNELS_ONCHIP") != NULL);
}

/*****************************************************************************/
/* DLIF_ALLOCATE() - Return the load address of the segment/section          */
/*      described in its parameters and record the run address in            */
/*      run_address field of DLOAD_MEMORY_REQUEST.                           */
/*****************************************************************************/
BOOL DLIF_allocate(void* client_handle, struct DLOAD_MEMORY_REQUEST *targ_req)
{
   DSPDevice* device = ((DSPProgram*) client_handle)->GetDevice();
   SharedMemory* shm = device->GetSHMHandler();

   /*------------------------------------------------------------------------*/
   /* Get pointers to API segment and file descriptors.                      */
   /*------------------------------------------------------------------------*/
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;

   uint32_t addr;

   if (device->addr_is_l2(obj_desc->target_address))
        addr = obj_desc->target_address; // do not allocate L2
   else if (device->addr_is_msmc(obj_desc->target_address) || load_kernels_onchip())
        addr = (uint32_t)shm->AllocateMSMC(obj_desc->memsz_in_bytes);
   else addr = (uint32_t)shm->AllocateGlobal(obj_desc->memsz_in_bytes, true);

#if DEBUG
   printf("DLIF_allocate: %d bytes starting at 0x%x (relocated from 0x%x)\n",
                      obj_desc->memsz_in_bytes, (uint32_t)addr,
                      (uint32_t)obj_desc->target_address);
#endif

   obj_desc->target_address = (TARGET_ADDRESS) addr;

   /*------------------------------------------------------------------------*/
   /* Target memory request was successful.                                  */
   /*------------------------------------------------------------------------*/
   return addr == 0 ? 0 : 1;
}

/*****************************************************************************/
/* DLIF_RELEASE() - Unmap or free target memory that was previously          */
/*      allocated by DLIF_allocate().                                        */
/*****************************************************************************/
BOOL DLIF_release(void* client_handle, struct DLOAD_MEMORY_SEGMENT* ptr)
{
   DSPDevice* device = ((DSPProgram*) client_handle)->GetDevice();
   SharedMemory* shm = device->GetSHMHandler();

   if (device->addr_is_l2(ptr->target_address))
        ; // local was not allocated
   else if (device->addr_is_msmc(ptr->target_address) || load_kernels_onchip())
        shm->FreeMSMC((DSPDevicePtr)ptr->target_address);
   else shm->FreeGlobal((DSPDevicePtr)ptr->target_address);

#if DEBUG
   printf("DLIF_free: %d bytes starting at 0x%x\n",
                      ptr->memsz_in_bytes, (uint32_t)ptr->target_address);
#endif

   return 1;
}

/*****************************************************************************/
/* DLIF_WRITE() - Write updated (relocated) segment contents to target       */
/*      memory.                                                              */
/*****************************************************************************/
BOOL DLIF_write(void* client_handle, struct DLOAD_MEMORY_REQUEST* req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;

   DSPProgram *program = static_cast<DSPProgram *>(client_handle);
   DSPDevice  *device = program->GetDevice();
   DLOAD *dl = dynamic_cast<DLOAD *>(program->GetDynamicLoader());

   assert (dl != 0);

   int dsp_id = device->dspID();

   SharedMemory*shm = device->GetSHMHandler();

   if (device->addr_is_l2(obj_desc->target_address))
   {
       if (req->host_address)
           printf("Warning: Initialized data for objects in .mem_l2 sections will be ignored.\n");
   }
   else if (req->host_address)
       shm->WriteToShmem ((uint32_t)obj_desc->target_address,
                             (uint8_t*)req->host_address,
                             obj_desc->memsz_in_bytes);

    if (req->flags & DLOAD_SF_executable)
        dl->SetProgramLoadAddress((DSPDevicePtr)obj_desc->target_address);

#if DEBUG
    printf("DLIF_write (dsp:%d): %d bytes starting at 0x%x\n",
               dsp_id, obj_desc->memsz_in_bytes,
               (uint32_t)obj_desc->target_address);
#endif

    if (program->IsPrintInfoEnabled())
    {
        uint32_t flags = req->flags & (DLOAD_SF_executable | DLOAD_SF_writable);

        const char *seg_desc;
        switch (flags)
        {
            case 0:                   seg_desc = "Read Only"; break;
            case DLOAD_SF_executable: seg_desc = "Executable"; break;
            case DLOAD_SF_writable:   seg_desc = "Writable"; break;
            default:                  seg_desc = "Writable & Executable"; break;
        }

        printf("\t%s segment loaded to 0x%08x with size 0x%x\n",
                       seg_desc, obj_desc->target_address,
                       obj_desc->memsz_in_bytes);
    }

    return 1;
}

/*****************************************************************************/
/* DLIF_COPY() - Copy data from file to host-accessible memory.              */
/*      Returns a host pointer to the data in the host_address field of the  */
/*      DLOAD_MEMORY_REQUEST object.                                         */
/*****************************************************************************/
BOOL DLIF_copy(void* client_handle, struct DLOAD_MEMORY_REQUEST* targ_req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;
   LOADER_FILE_DESC* f = targ_req->fp;
   DSPDevice* device = ((DSPProgram*) client_handle)->GetDevice();
   void *buf = NULL;

   int result = 1;
   if (obj_desc->objsz_in_bytes)
   {
       buf = calloc(obj_desc->memsz_in_bytes, 1);
       fseek(f, targ_req->offset, SEEK_SET);
       result = fread(buf, obj_desc->objsz_in_bytes, 1, f);
   }
   else  if (!device->addr_is_l2(obj_desc->target_address))
       buf = calloc(obj_desc->memsz_in_bytes, 1);

   assert(result == 1);

   targ_req->host_address = buf;

   return 1;
}


/******************************************************************************
* DLIF_LOAD_DEPENDENT()
******************************************************************************/
int DLIF_load_dependent(void* client_handle, const char* so_name)
{
   DSPProgram *program = static_cast<DSPProgram *>(client_handle);
   DLOAD *dl = dynamic_cast<DLOAD *>(program->GetDynamicLoader());

   assert (dl != 0);

   FILE* fp = fopen(so_name, "rb");

   if (!fp)
   {
      DLIF_error(DLET_FILE, "Can't open dependent file '%s'.\n", so_name);
      return 0;
   }

   int to_ret = DLOAD_load(dl->GetDloadHandle(), fp);

   if (!to_ret)
       DLIF_error(DLET_MISC, "Failed load of dependent file '%s'.\n", so_name);

   fclose(fp);
   return to_ret;
}

/******************************************************************************
* DLIF_UNLOAD_DEPENDENT()
******************************************************************************/
void DLIF_unload_dependent(void* client_handle, uint32_t file_handle)
{
   DSPProgram *program = static_cast<DSPProgram *>(client_handle);
   DLOAD *dl = dynamic_cast<DLOAD *>(program->GetDynamicLoader());

   assert (dl != 0);

   DLOAD_unload(dl->GetDloadHandle(), file_handle);
}

}

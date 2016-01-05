/******************************************************************************
 * Copyright (c) 2013-2014, Texas Instruments Incorporated - http://www.ti.com/
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
#ifdef _SYS_BIOS
#include <ti/sysbios/posix/pthread.h>
#include <xdc/std.h>
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dload_api.h"

#define TYPE_STACK_DEFINITION(t, t_name)
#define TYPE_STACK_IMPLEMENTATION(t, t_name) 

int debugging_on = FALSE;
int profiling_on = FALSE;

int global_argc;
char **global_argv;

int DLIF_fseek(LOADER_FILE_DESC *stream, int32_t offset, int origin)
#ifndef _SYS_BIOS
    { return fseek(stream, offset, origin); }
#else
{
	if(stream->mode == 0)
	      return fseek(stream->fp, offset, origin);

	else
	{
	switch(origin){
	  case SEEK_SET:
		  stream->cur = stream->orig;
		  stream->cur += offset;
		  stream->read_size = offset;
		  return 0;
	  case SEEK_CUR:
	  	  stream->cur += offset;
	  	  stream->read_size += offset;
	  	  return 0;
	  case SEEK_END:
		  stream->cur = stream->orig + stream->size;
	  	  stream->read_size = stream->size;
  	  	  return 0;
	  default:
		  return -1;
	}
	}
}
#endif


size_t DLIF_fread(void *ptr, size_t size, size_t nmemb,
                  LOADER_FILE_DESC *stream)
#ifndef _SYS_BIOS
    { return fread(ptr, size, nmemb, stream); }
#else
{
	if(stream->mode == 0)
       return fread(ptr, size, nmemb, stream->fp);
	else
	{
	memcpy(ptr, stream->cur, size*nmemb);
	stream->cur += size*nmemb;
	stream->read_size +=size*nmemb;
	return size*nmemb;
	}

}
#endif

int32_t DLIF_ftell (LOADER_FILE_DESC *stream)
#ifndef _SYS_BIOS
{ return ftell(stream); }
#else
{
	if(stream->mode == 0)
	  return ftell(stream->fp);
	else
	 return (int32_t)stream->cur;
}
#endif
int32_t DLIF_fclose(LOADER_FILE_DESC *fd)
#ifndef _SYS_BIOS
{ return fclose(fd); }
#else
{
return 0;
}
#endif
void*   DLIF_malloc(size_t size)              { return malloc(size); }
void    DLIF_free  (void* ptr)                { free(ptr); }

/*****************************************************************************/
/* DLIF_COPY() - Copy data from file to host-accessible memory.              */
/*      Returns a host pointer to the data in the host_address field of the  */
/*      DLOAD_MEMORY_REQUEST object.                                         */
/*****************************************************************************/
BOOL DLIF_copy(void* client_handle, struct DLOAD_MEMORY_REQUEST* targ_req)
{
#if 1
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;
   LOADER_FILE_DESC* f = targ_req->fp;
   void *buf = calloc(obj_desc->memsz_in_bytes, 1); 

   DLIF_fseek(f, targ_req->offset, SEEK_SET);

   int result = 1;
   if (obj_desc->objsz_in_bytes)
       result = DLIF_fread(buf, obj_desc->objsz_in_bytes, 1, f);

//   assert(result == 1);

   targ_req->host_address = buf;
#else
   targ_req->host_address = (void*)(targ_req->segment->target_address);
#endif
   return 1;
}

BOOL DLIF_read(void* client_handle, 
               void *ptr, size_t size, size_t nmemb, TARGET_ADDRESS src)
    { assert(0); }

BOOL DLIF_memcpy(void* client_handle, 
                 void *to, void *from, size_t size)
    { return (!memcpy(to, from, size)) ? 0 : 1; }

int32_t DLIF_execute(void* client_handle,
                     TARGET_ADDRESS exec_addr) { assert(0); return 1; }




BOOL DLIF_register_dsbt_index_request(DLOAD_HANDLE handle,
                                      const char *requestor_name,
				      int32_t     requestor_file_handle,
				      int32_t     requested_dsbt_index)
    { assert(0); }

void DLIF_assign_dsbt_indices(void) { assert(0); }

int32_t DLIF_get_dsbt_index(int32_t file_handle)
    { assert(0); return DSBT_INDEX_INVALID; }

BOOL DLIF_update_all_dsbts() { assert(0); return TRUE; }

void DLIF_warning(LOADER_WARNING_TYPE wtype, const char *fmt, ...)
{
   va_list ap;
   va_start(ap,fmt);
   printf("<< D L O A D >> WARNING: ");
   vprintf(fmt,ap);
   va_end(ap);
}

void DLIF_error(LOADER_ERROR_TYPE etype, const char *fmt, ...)
{
   va_list ap;
   va_start(ap,fmt);
   printf("<< D L O A D >> ERROR: ");
   vprintf(fmt,ap);
   va_end(ap);
}

void DLIF_trace(const char *fmt, ...)
{
    va_list ap;
    va_start(ap,fmt);
    vprintf(fmt,ap);
    va_end(ap);
}

void DLIF_exit(ecode)
{
    exit(ecode);
}


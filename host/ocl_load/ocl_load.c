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
    { return fseek(stream, offset, origin); }


size_t DLIF_fread(void *ptr, size_t size, size_t nmemb,
                  LOADER_FILE_DESC *stream)
    { return fread(ptr, size, nmemb, stream); }

int32_t DLIF_ftell (LOADER_FILE_DESC *stream) { return ftell(stream); }
int32_t DLIF_fclose(LOADER_FILE_DESC *fd)     { return fclose(fd); }
void*   DLIF_malloc(size_t size)              { return malloc(size); }
void    DLIF_free  (void* ptr)                { free(ptr); }

/*****************************************************************************/
/* DLIF_COPY() - Copy data from file to host-accessible memory.              */
/*      Returns a host pointer to the data in the host_address field of the  */
/*      DLOAD_MEMORY_REQUEST object.                                         */
/*****************************************************************************/
BOOL DLIF_copy(struct DLOAD_MEMORY_REQUEST* targ_req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;
   LOADER_FILE_DESC* f = targ_req->fp;
   void *buf = calloc(obj_desc->memsz_in_bytes, 1); 

   fseek(f, targ_req->offset, SEEK_SET);
   int result = fread(buf, obj_desc->objsz_in_bytes, 1, f);
   assert(result == 1);

   targ_req->host_address = buf;

   return 1;
}

BOOL DLIF_read(void *ptr, size_t size, size_t nmemb, TARGET_ADDRESS src)
    { assert(0); }

BOOL DLIF_memcpy(void *to, void *from, size_t size)
    { return (!memcpy(to, from, size)) ? 0 : 1; }

int32_t DLIF_execute(TARGET_ADDRESS exec_addr) { assert(0); return 1; }

int DLIF_load_dependent(void* client_handle, const char* so_name)
    { assert(0); }

void DLIF_unload_dependent(void* client_handle, uint32_t file_handle)
    { assert(0); }

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

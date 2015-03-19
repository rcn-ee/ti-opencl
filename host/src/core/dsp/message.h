#ifndef __MESSAGE_H_
#define __MESSAGE_H_

typedef struct
{
   int  number_commands;
   struct { unsigned code; unsigned addr; unsigned size; } commands[20];
} Msg_t;


typedef struct
{
   unsigned dims;
   unsigned global_size_0;
   unsigned global_size_1;
   unsigned global_size_2;

   unsigned local_size_0;
   unsigned local_size_1;
   unsigned local_size_2;

   unsigned global_offset_0;
   unsigned global_offset_1;
   unsigned global_offset_2;

   unsigned WG_gid_start_0;
   unsigned WG_gid_start_1;
   unsigned WG_gid_start_2;

   unsigned entry_point;
   unsigned data_page_ptr;
} KernelConfig;


typedef enum { READY, EXIT, NDRKERNEL, WORKGROUP, CACHEINV, SUCCESS, ERROR } command_codes;

#endif

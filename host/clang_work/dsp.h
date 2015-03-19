/*--------------------------------------------------------------------------
* num_dims = _config_packet[0]
* G        = _config_packet[1,2,3]
* S        = _config_packet[4,5,6]
* F        = _config_packet[7,8,9]
* t        = _config_packet[10,11,12]
* wg_id    = _config_packet[13]
* unused   = _config_packet[14,15]
*
* Gi = global_size[i]   is fixed and specified for i in [1..work_dim]
* Si = local_size[i]    is fixed and specified for i in [1..work_dim]
* Fi = global_offset[i] is fixed and specified for i in [1..work_dim], 
*                       defaults to [0,0,0]
*
* Wi = num_groups[i]  = Gi / Si
*
* gi  = global_id[i]  = ti + si
* si  = local_id[i]
* wi  = group_id[i]   = (gi - si - Fi) /Si
* ti  = global_id(i) of workitem 0 in workgroup  = Fi+wi*Si
*-------------------------------------------------------------------------*/

#define _cfg_addr ((int*)0x00800000) // L2 SRAM
#define _cfg(i) *(_cfg_addr + i)

#define LOOPS3 for (int s2 = 0; s2 < _cfg(6); ++s2) for (int s1 = 0; s1 < _cfg(5); ++s1) for (int s0 = 0; s0 < _cfg(4); ++s0)
#define LOOPS2 for (int s1 = 0; s1 < _cfg(5); ++s1) for (int s0 = 0; s0 < _cfg(4); ++s0)
#define LOOPS1 for (int s0 = 0; s0 < _cfg(4); ++s0)
#define LOOPS0 

/*--------------------------------------------------------------------------
* Giving a dimension outside range will surely screw up your system
*-------------------------------------------------------------------------*/
#define get_work_dim()       _cfg(0)
#define get_global_size(i)   _cfg(1+i)
#define get_global_id(i)     (_cfg(10+i) + s##i)
#define get_local_size(i)    _cfg(4+i)
#define get_local_id(i)      (s##i)
#define get_num_groups(i)    (_cfg(1+i) / _cfg(4+i))
#define get_group_id(i)      ((_cfg(10+i)-_cfg(7+i)) / _cfg(4+i))
#define get_global_offset(i) _cfg(7+i)

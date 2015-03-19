#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "dload.h"
#include "elf32.h"

#ifdef C60_TARGET
#include "c60_dynamic.h"
#include "c60_reloc.h"
#endif

#ifdef ARM_TARGET
#include "arm_dynamic.h"
#include "arm_reloc.h"
#endif

/*****************************************************************************/
/* Define a virtual target class to give access to target specific functions */
/*****************************************************************************/
typedef struct vtarget 
{
   int machine_id;
 
   BOOL (*relocate_dynamic_tag_info)(DLIMP_Dynamic_Module *dyn_module, int i);
   BOOL (*process_eiosabi)(DLIMP_Dynamic_Module* dyn_module);
   BOOL (*process_dynamic_tag)(DLIMP_Dynamic_Module *dyn_module, int i);
   void (*relocate)(DLOAD_HANDLE handle, LOADER_FILE_DESC *elf_file, 
                    DLIMP_Dynamic_Module *dyn_module);

} VIRTUAL_TARGET;



/*****************************************************************************/
/* Populate this for each target supported.                                  */
/*****************************************************************************/
VIRTUAL_TARGET vt_arr[] = {

#ifdef C60_TARGET
                 { 
                    EM_TI_C6000, 
                    DLDYN_c60_relocate_dynamic_tag_info, 
                    DLDYN_c60_process_eiosabi, 
                    DLDYN_c60_process_dynamic_tag, 
                    DLREL_c60_relocate
                 },
#endif
#ifdef ARM_TARGET
                 { 
                    EM_ARM, 
                    DLDYN_arm_relocate_dynamic_tag_info, 
                    DLDYN_arm_process_eiosabi, 
                    DLDYN_arm_process_dynamic_tag, 
                    DLREL_arm_relocate
                 },
#endif
                 { 
                    EM_NONE, 
                    0, 
                    0, 
                    0, 
                    0 
                 }
};



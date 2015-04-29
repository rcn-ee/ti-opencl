#include <ti/csl/csl_semAux.h>
#include <ti/csl/csl_xmc.h>
#include <ti/csl/csl_xmcAux.h>
#include <ti/csl/csl_cacheAux.h>


__attribute((visibility("protected")))
uint32_t __sem_lock(int idx)
{
    while (!CSL_semAcquireDirect(idx));
    return 0;
}

__attribute((visibility("protected")))
void __sem_unlock(int idx, uint32_t lvInt)
{
    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
    CSL_semReleaseSemaphore(idx);
}

__attribute((visibility("protected")))
void __inv(char*p, int sz)
{
    CACHE_invL2(p, sz, CACHE_NOWAIT);
    CSL_XMC_invalidatePrefetchBuffer();

    _mfence(); // Wait until data written to memory
    _mfence(); // Second one because of a bug
    asm(" NOP 9");
    asm(" NOP 7");
}

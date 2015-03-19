#include "u_lockable.h"
#ifndef _CORE_SCHEDULER_H
#define _CORE_SCHEDULER_H

class CoreScheduler : public Lockable
{
  public:
    CoreScheduler() : p_avail(0xff) {}

    void free(int core) 
    { 
        Lock lock(this);
        p_avail |= (1 << core);
        CV.notify_one();
    }

    int allocate()
    {
        Lock lock(this);

        /*---------------------------------------------------------------------
        * Wait in a loop in case the condvar is falsely signalled
        *--------------------------------------------------------------------*/
        while (!p_avail) CV.wait(lock.raw());

        for (int i=0, mask = 1; i < 8; ++i, mask <<= 1)
            if (p_avail & mask) { p_avail &= ~mask; return i; }
    }

  private:
     unsigned char p_avail;
     CondVar       CV;
};

#endif //_CORE_SCHEDULER_H

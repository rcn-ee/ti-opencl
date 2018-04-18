/******************************************************************************
 * Copyright (c) 2013-2018, Texas Instruments Incorporated - http://www.ti.com/
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
#include "rootdevice.h"
#include "device_info.h"
/*-----------------------------------------------------------------------------
* Add ULM memory state messages if ULM library is available
*----------------------------------------------------------------------------*/
extern "C"
{
#if defined (ULM_ENABLED)
   #include "tiulm.h"
#endif
}

using namespace Coal;
using namespace tiocl;

/*-----------------------------------------------------------------------------
* Declare our threaded dsp handler function
*----------------------------------------------------------------------------*/
void *dsp_worker_event_dispatch   (void* data);
void *dsp_worker_event_completion (void* data);

/******************************************************************************
* DSPRootDevice::DSPRootDevice(unsigned char dsp_id, SharedMemory* shm)
******************************************************************************/
DSPRootDevice::DSPRootDevice(unsigned char dsp_id, SharedMemory* shm)
    : DSPDevice              (shm),
      p_worker_dispatch      (0),
      p_worker_completion    (0),
      p_events               (),
      p_stop                 (false),
      p_exit_acked           (false),
      p_initialized          (false),
      p_complete_pending     (nullptr),
      core_scheduler_        (nullptr),
      device_manager_        (nullptr),
      p_mb                   (nullptr)
{
    p_dsp_id                      = dsp_id;
    const DeviceInfo& device_info = DeviceInfo::Instance();
    p_compute_units               = device_info.GetComputeUnits();

    /*-------------------------------------------------------------------------
    * Set possible partition types
    *------------------------------------------------------------------------*/
    if (p_compute_units.size() > 1)
    {
        p_partitions_supported[0] = CL_DEVICE_PARTITION_EQUALLY;
        p_partitions_supported[1] = CL_DEVICE_PARTITION_BY_COUNTS;
    }

    /*-------------------------------------------------------------------------
    * Reset and start DSP cores
    *------------------------------------------------------------------------*/
#if !defined(_SYS_BIOS)
    device_manager_ = DeviceManagerFactory::CreateDeviceManager(dsp_id, p_compute_units.size(),
                                                 device_info.FullyQualifiedPathToDspMonitor());
    device_manager_->Reset();
    device_manager_->Load();
    device_manager_->Run();
#endif

    /*-------------------------------------------------------------------------
    * Initialize Core Scheduler
    *------------------------------------------------------------------------*/
    core_scheduler_ = new CoreScheduler(p_compute_units, CORE_SCHEDULER_DEFAULT_DEPTH);

    /*-------------------------------------------------------------------------
    * Initialize the mailboxes on the cores, so they can receive an exit cmd
    *------------------------------------------------------------------------*/
    p_mb = MBoxFactory::CreateMailbox(this);

    /*-------------------------------------------------------------------------
    * Initialize ulm only after we establish mailboxes with cores because
    * CreateMailbox() could fail and exit from this DSPRootDevice constructor,
    * in which case ulm_term() in DSPRootDevice destructor won't be called.
    *------------------------------------------------------------------------*/
    init_ulm();

    /*-------------------------------------------------------------------------
     * Send monitor configuration
     * On AM57, the monitor is configured based on the number of cores in the
     * configuration message
     *------------------------------------------------------------------------*/
    Msg_t msg = {CONFIGURE_MONITOR};
    msg.u.configure_monitor.n_cores = p_compute_units.size();
    msg.u.configure_monitor.master_core = *p_compute_units.begin();

    mail_to(msg, p_compute_units);

    /*-------------------------------------------------------------------------
    * Query DSP frequency; monitor is in message loop task after this point.
    *------------------------------------------------------------------------*/
    setup_dsp_mhz();

    /*-------------------------------------------------------------------------
    * Allocate p_complete_pending map
    *------------------------------------------------------------------------*/
    p_complete_pending = new concurrent_map<uint32_t, class Event*>();
}

/*-----------------------------------------------------------------------------
* If ULM library was not available don't emit ULM trace messages
*----------------------------------------------------------------------------*/
#if !defined (ULM_ENABLED)
#define ulm_put_mem(a,b,c)
#define ulm_config()
#define ulm_term()
#endif

/******************************************************************************
* DSPRootDevice::init_ulm()
******************************************************************************/
void DSPRootDevice::init_ulm()
{
    ulm_config();
    uint64_t cmem_persistent_sz =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_PERSISTENT,
                                  MemoryRange::Location::OFFCHIP);
    uint64_t cmem_ondemand_sz =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_ONDEMAND,
                                  MemoryRange::Location::OFFCHIP);
    uint64_t msmc_mem_size =
        GetSHMHandler()->HeapSize(MemoryRange::Kind::CMEM_PERSISTENT,
                                  MemoryRange::Location::ONCHIP);
    ulm_put_mem(ULM_MEM_IN_DATA_ONLY,     msmc_mem_size >> 16  , 1.0f);
    ulm_put_mem(ULM_MEM_EX_DATA_ONLY,     cmem_ondemand_sz >> 16, 1.0f);
    ulm_put_mem(ULM_MEM_EX_CODE_AND_DATA, cmem_persistent_sz >> 16, 1.0f);
}

/******************************************************************************
* DSPRootDevice::~DSPRootDevice()
******************************************************************************/
DSPRootDevice::~DSPRootDevice()
{
    if (p_initialized)
    {
        /*---------------------------------------------------------------------
        * Terminate the workers and wait for them
        *--------------------------------------------------------------------*/
        pthread_mutex_lock(&p_events_mutex);
        p_stop = true;
        pthread_cond_broadcast(&p_events_cond);
        pthread_mutex_unlock(&p_events_mutex);
        pthread_join(p_worker_dispatch, 0);
        pthread_join(p_worker_completion, 0);
        pthread_mutex_destroy(&p_events_mutex);
        pthread_cond_destroy(&p_events_cond);
        pthread_mutex_destroy(&p_worker_mutex);
        pthread_cond_destroy(&p_worker_cond);
    }
    /*-------------------------------------------------------------------------
    * Inform the cores on the device to stop listening for commands
    * Send exit message after worker threads terminate to avoid a race condition
    * on p_exit_acked. The race condition is caused by a worker thread receiving
    * the exit response and setting p_exit_acked to true. It's possible that the
    * master thread can read p_exit_acked before write from the worker thread
    * lands and enter the while loop. It then is stuck at the mail_query() while
    * loop because the worker has already read the message.
    *
    * Sending the exit message after the worker threads terminate eliminates
    * the race condition.
    *------------------------------------------------------------------------*/
    mail_to(exitMsg, p_compute_units);
#if defined(DSPC868X)
    /*-------------------------------------------------------------------------
    * Wait for the EXIT acknowledgement from device
    *------------------------------------------------------------------------*/
    while (!p_exit_acked)
    {
        while (!mail_query()) usleep(1);
        mail_from(p_compute_units);
    }
#endif

    delete p_mb;
    delete p_complete_pending;

    /*-------------------------------------------------------------------------
    * Free any ulm resources used.
    *------------------------------------------------------------------------*/
    ulm_term();
    /*-------------------------------------------------------------------------
    * Only need to close the driver for one of the devices
    *------------------------------------------------------------------------*/
    delete device_manager_;
    delete core_scheduler_;
}

/******************************************************************************
* void DSPRootDevice::init()
******************************************************************************/
void DSPRootDevice::init()
{
    if (p_initialized) return;
    /*-------------------------------------------------------------------------
    * Initialize the locking machinery and create worker threads
    *------------------------------------------------------------------------*/
    /* Allocate memory for data structures */
    pthread_cond_init(&p_events_cond, 0);
    pthread_mutex_init(&p_events_mutex, 0);
    pthread_cond_init(&p_worker_cond, 0);
    pthread_mutex_init(&p_worker_mutex, 0);
#if !defined(_SYS_BIOS)
    pthread_create(&p_worker_dispatch,   0, &dsp_worker_event_dispatch,   this);
    pthread_create(&p_worker_completion, 0, &dsp_worker_event_completion, this);
#else
    pthread_attr_t attr_dispatch, attr_completion;
    pthread_attr_init(&attr_dispatch);
    pthread_attr_init(&attr_completion);
    /* Give dispatch thread +1 priority so that work can be dispatch to devices
     * immediately when available, give completion thread +2 priority so that
     * completion message from devices can be processed immediately.
     * */
    int pri = Task_getPri(Task_self());
    attr_dispatch.priority   = (pri >= 15 - 2) ? pri : pri + 1;
    attr_completion.priority = (pri >= 15 - 2) ? pri : pri + 2;
    pthread_create(&p_worker_dispatch,   &attr_dispatch,
                   &dsp_worker_event_dispatch,   this);
    pthread_create(&p_worker_completion, &attr_completion,
                   &dsp_worker_event_completion, this);
#endif
    p_initialized = true;
}

/******************************************************************************
* void DSPRootDevice::pushEvent(Event *event)
******************************************************************************/
void DSPRootDevice::pushEvent(Event* event)
{
    /*-------------------------------------------------------------------------
    * Add an event in the list
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);
    p_events.push_back(event);
    pthread_cond_broadcast(&p_events_cond);
    pthread_mutex_unlock(&p_events_mutex);
}

/******************************************************************************
* void DSPRootDevice::stop()
******************************************************************************/
bool DSPRootDevice::stop()
{
    return p_stop;
}

/******************************************************************************
* void DSPRootDevice::availableEvent()
******************************************************************************/
bool DSPRootDevice::availableEvent()
{
    return p_events.size() > 0;
}

/******************************************************************************
* Event *DSPRootDevice::getEvent(bool &stop)
******************************************************************************/
Event* DSPRootDevice::getEvent(bool& stop)
{
    /*-------------------------------------------------------------------------
    * Return the first event in the list, if any. Remove it if it is a
    * single-shot event.
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);
    while (p_events.size() == 0 && !p_stop)
        pthread_cond_wait(&p_events_cond, &p_events_mutex);
    if (p_stop)
    {
        pthread_mutex_unlock(&p_events_mutex);
        stop = true;
        return 0;
    }
    Event* event = p_events.front();
    p_events.pop_front();
    pthread_mutex_unlock(&p_events_mutex);
    return event;
}

/******************************************************************************
* bool DSPRootDevice::gotEnoughToWorkOn()
* Device's decision about whether CommandQueue should push more events over
* This number could be tuned (e.g. using ooo example).  Note that events
* are in device's queue, but not yet executed.
******************************************************************************/
bool DSPRootDevice::gotEnoughToWorkOn()
{
    return p_events.size() > 0;
}

/******************************************************************************
 * DSPRootDevice::mail_query()
******************************************************************************/
bool DSPRootDevice::mail_query()
{
    return p_mb->query();
}

/******************************************************************************
* DSPRootDevice::mail_query(int* retcode, const DSPCoreSet& compute_units)
******************************************************************************/
int DSPRootDevice::mail_from(const DSPCoreSet& compute_units, int* retcode)
{
    uint32_t size_rx;
    int32_t  trans_id_rx;
    Msg_t    rxmsg;
    uint8_t  core;
    trans_id_rx = p_mb->from((uint8_t*)&rxmsg, &size_rx, &core);
    if (rxmsg.command == ERROR)
    {
        std::cout << rxmsg.u.message;
        return -1;
    }
    if (rxmsg.command == PRINT)
    {
        std::cout << "[core " << rxmsg.u.message[0] << "] "
                  << rxmsg.u.message + 1;
        return -1;
    }
    if ((rxmsg.command == NDRKERNEL) || (rxmsg.command == TASK))
    {
        command_retcode_t* profiling_data = &(rxmsg.u.command_retcode);
        recordProfilingData(profiling_data, core);
    }
    if (rxmsg.command == EXIT)
    {
        p_exit_acked = true;
        return -1;
    }
    if (rxmsg.command == TASK && IS_OOO_TASK(rxmsg))
    {
        if (compute_units.find(core) != compute_units.end()) core_scheduler_->free(core);
    }
    if (retcode != nullptr)  *retcode = rxmsg.u.command_retcode.retcode;
    return trans_id_rx;
}

/******************************************************************************
 * DSPRootDevice::mail_to(Msg_t& msg,
 *                        unsigned int core,
 *                        const DSPCoreSet& compute_units)
******************************************************************************/
void DSPRootDevice::mail_to(Msg_t& msg,
                            const DSPCoreSet& compute_units,
                            unsigned int core)
{
    msg.pid = p_pid;
    switch (msg.command)
    {
        /*-----------------------------------------------------------------
        * Broadcast the following messages to the respective compute units
        * for this device
        *----------------------------------------------------------------*/
        case EXIT:
        case CACHEINV:
        case NDRKERNEL:
        case SETUP_DEBUG:
        case CONFIGURE_MONITOR:
            {
                /* In debug mode send message to the first compute unit */
                if (IS_DEBUG_MODE(msg))
                {
                    /* Get iterator to first element in set */
                    auto first_compute_unit = compute_units.begin();
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), *first_compute_unit);
                }
                else
                {
                    /* Send message to each compute unit in this device */
                    for (auto & compute_unit : compute_units)
                    {
                        p_mb->to((uint8_t*)&msg, sizeof(Msg_t), compute_unit);
                    }
                }
                break;
            }
        /*-----------------------------------------------------------------
        * OoO TASKs are sent to cores in round-robin order, in-order TASKs
        * are broadcast to all cores In case this is a sub device, the
        * allocation is done using the root device's core scheduler, but
        * only on the cores that this sub device is associated with
        *----------------------------------------------------------------*/
        case TASK:
            {
                if (IS_OOO_TASK(msg))
                {
                    int dsp_id = core_scheduler_->allocate(compute_units);
                    p_mb->to((uint8_t*)&msg, sizeof(Msg_t), dsp_id);
                }
                else
                {
                    /*---------------------------------------------------------
                    * note: for in-order TASKs this is a trick to enable the
                    * OpenMP runtime. OpenMP kernels run as in-order tasks.
                    *-------------------------------------------------------*/
                    for (auto & compute_unit : compute_units)
                    {
                        p_mb->to((uint8_t*)&msg, sizeof(Msg_t), compute_unit);
                    }
                }
                break;
            }
        /*-----------------------------------------------------------------
        * otherwise send it to the designated core
        *---------------------------------------------------------------*/
        default:
            {
                p_mb->to((uint8_t*)&msg, sizeof(Msg_t), core);
                break;
            }
    }
}

/******************************************************************************
 * Complete Pending access functions
******************************************************************************/
void DSPRootDevice::push_complete_pending(uint32_t idx,
                                          Event* const data,
                                          unsigned int cnt)
{ p_complete_pending->push(idx, data, cnt); }

bool DSPRootDevice::get_complete_pending(uint32_t idx, Event*& data)
{ return p_complete_pending->try_pop(idx, data); }

int  DSPRootDevice::num_complete_pending()
{ return p_complete_pending->size(); }

void DSPRootDevice::dump_complete_pending()
{ p_complete_pending->dump(); }

bool DSPRootDevice::any_complete_pending()
{ return !p_complete_pending->empty(); }

/******************************************************************************
 * DSPRootDevice::setup_dsp_mhz()
******************************************************************************/
void DSPRootDevice::setup_dsp_mhz()
{
#ifdef DSPC868X
    char* ghz1 = EnvVar::Instance().GetEnv<EnvVar::Var: TI_OCL_DSP_1_25GHZ>(
                                                                    nullptr);
    if (ghz1) p_dsp_mhz = 1250;  // 1.25 GHz
#else
    mail_to(frequencyMsg, p_compute_units);
    int ret = 0;
    do
    {
        while (!mail_query())  ;
        ret = mail_from(p_compute_units);
    }
    while (ret == -1);
    p_dsp_mhz = ret;
#endif
}

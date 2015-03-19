#include "device.h"
#include "buffer.h"
#include "kernel.h"
#include "program.h"

#include <core/config.h>
#include "../propertylist.h"
#include "../commandqueue.h"
#include "../events.h"
#include "../memobject.h"
#include "../kernel.h"
#include "../program.h"

#include "dsp_kernel_defs.h"
#include "driver.h"
#include "mailbox.h"

extern "C"
{
    #include "dload_api.h"
}

#include <cstring>
#include <cstdlib>
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <sstream>

using namespace Coal;

Mailbox* Mailbox::pInstance = 0;

/*-----------------------------------------------------------------------------
* Declare our threaded dsp handler function
*----------------------------------------------------------------------------*/
void *dsp_worker(void* data);

#define DDR_START 0x82000000
#define L2_START  0x00840000

#define DDR_LENGTH 0x3E000000 // 992 M
#define L2_LENGTH  0x00020000 // 128 K


/******************************************************************************
* DSPDevice::DSPDevice(unsigned char dsp_id)
******************************************************************************/
DSPDevice::DSPDevice(unsigned char dsp_id)
    : DeviceInterface   (), 
      p_cores           (8), 
      p_num_events      (0), 
      p_dsp_mhz         (1024), // 1.00 GHz
      p_worker          (0), 
      p_rx_mbox         (0), 
      p_tx_mbox         (0), 
      p_stop            (false),
      p_initialized     (false), 
      p_dsp_id          (dsp_id), 
      p_device_ddr_heap ((DSPDevicePtr)0x82000000, 0x3E000000),   // 992 M
      p_device_l2_heap  ((DSPDevicePtr)0x00840000, 0x00020000),   // 128 K
      p_dload_handle    (0)
{ }

/******************************************************************************
* void DSPDevice::init()
******************************************************************************/
void DSPDevice::init()
{
    if (p_initialized) return;

    /*-------------------------------------------------------------------------
    * Initialize the locking machinery
    *------------------------------------------------------------------------*/
    pthread_cond_init(&p_events_cond, 0);
    pthread_mutex_init(&p_events_mutex, 0);

    /*-------------------------------------------------------------------------
    * Create worker threads
    *------------------------------------------------------------------------*/
    Mailbox* mb_instance = Mailbox::instance();

    pthread_create(&p_worker, 0, &dsp_worker, this);

    int node_id = p_dsp_id * numDSPs();
    mb_instance->init(MAILBOX_NODE_ID_HOST, node_id);
    mb_instance->init(node_id, MAILBOX_NODE_ID_HOST);
    p_tx_mbox = mb_instance->create(MAILBOX_NODE_ID_HOST, node_id,0,0,0);
    p_rx_mbox = mb_instance->create(node_id, MAILBOX_NODE_ID_HOST,0,0,0);

    if (p_tx_mbox == -1 || p_rx_mbox == -1)
       std::cout << "Could not create mailboxes for dsp " << node_id << std::endl;

    p_initialized = true;
}

/******************************************************************************
* DSPDevice::~DSPDevice()
******************************************************************************/
DSPDevice::~DSPDevice()
{
    if (!p_initialized) return;

    /*-------------------------------------------------------------------------
    * Inform the cores on the device to stop listening for commands
    *------------------------------------------------------------------------*/
    Msg_t msg = {1, {EXIT,0,0}};
    mail_to(msg);

    /*-------------------------------------------------------------------------
    * Reset the cores
    *------------------------------------------------------------------------*/

    /*-------------------------------------------------------------------------
    * Only need to close the driver for one of the devices
    *------------------------------------------------------------------------*/
    if (p_dsp_id == 0) Driver::instance()->close(); 

    /*-------------------------------------------------------------------------
    * Terminate the workers and wait for them
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);

    p_stop = true;

    pthread_cond_broadcast(&p_events_cond);
    pthread_mutex_unlock(&p_events_mutex);

    pthread_join(p_worker, 0);

    pthread_mutex_destroy(&p_events_mutex);
    pthread_cond_destroy(&p_events_cond);

}

/******************************************************************************
* DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer)
******************************************************************************/
DeviceBuffer *DSPDevice::createDeviceBuffer(MemObject *buffer, cl_int *rs)
    { return (DeviceBuffer *)new DSPBuffer(this, buffer, rs); }

/******************************************************************************
* DeviceProgram *DSPDevice::createDeviceProgram(Program *program)
******************************************************************************/
DeviceProgram *DSPDevice::createDeviceProgram(Program *program)
    { return (DeviceProgram *)new DSPProgram(this, program); }

/******************************************************************************
* DeviceKernel *DSPDevice::createDeviceKernel(Kernel *kernel,
******************************************************************************/
DeviceKernel *DSPDevice::createDeviceKernel(Kernel *kernel,
                                llvm::Function *function)
    { return (DeviceKernel *)new DSPKernel(this, kernel); }

/******************************************************************************
* cl_int DSPDevice::initEventDeviceData(Event *event)
******************************************************************************/
cl_int DSPDevice::initEventDeviceData(Event *event)
{
    switch (event->type())
    {
        case Event::MapBuffer:
        {
            MapBufferEvent *e    = (MapBufferEvent*) event;
            DSPBuffer      *buf  = (DSPBuffer*) e->buffer()->deviceBuffer(this);
            unsigned char  *data = ((unsigned char*) buf->data()) + e->offset();

            e->setPtr((void *)data);
            break;
        }

        case Event::MapImage: break;

        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            KernelEvent *e    = (KernelEvent *)event;
            Program     *p    = (Program *)e->kernel()->parent();
            DSPProgram  *prog = (DSPProgram *)p->deviceDependentProgram(this);

            /*-----------------------------------------------------------------
            * Just in time loading 
            *----------------------------------------------------------------*/
            if (!prog->is_loaded()) prog->load();
            DSPKernel *dspkernel = (DSPKernel*)e->deviceKernel();
            dspkernel->preAllocBuffers();

            // ASW TODO do something

            // Set device-specific data
            DSPKernelEvent *dsp_e = new DSPKernelEvent(this, e);
            e->setDeviceData((void *)dsp_e);
            break;
        }
        default: break;
    }

    return CL_SUCCESS;
}

/******************************************************************************
* void DSPDevice::freeEventDeviceData(Event *event)
******************************************************************************/
void DSPDevice::freeEventDeviceData(Event *event)
{
    switch (event->type())
    {
        case Event::NDRangeKernel:
        case Event::TaskKernel:
        {
            DSPKernelEvent *dsp_e = (DSPKernelEvent *)event->deviceData();
            if (dsp_e) delete dsp_e;
        }
        default: break;
    }
}

/******************************************************************************
* void DSPDevice::pushEvent(Event *event)
******************************************************************************/
void DSPDevice::pushEvent(Event *event)
{
    /*-------------------------------------------------------------------------
    * Add an event in the list
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);

    p_events.push_back(event);
    p_num_events++;                 // Way faster than STL list::size() !

    pthread_cond_broadcast(&p_events_cond);
    pthread_mutex_unlock(&p_events_mutex);
}

/******************************************************************************
* Event *DSPDevice::getEvent(bool &stop)
******************************************************************************/
Event *DSPDevice::getEvent(bool &stop)
{
    /*-------------------------------------------------------------------------
    * Return the first event in the list, if any. Remove it if it is a
    * single-shot event.
    *------------------------------------------------------------------------*/
    pthread_mutex_lock(&p_events_mutex);

    while (p_num_events == 0 && !p_stop)
        pthread_cond_wait(&p_events_cond, &p_events_mutex);

    if (p_stop)
    {
        pthread_mutex_unlock(&p_events_mutex);
        stop = true;
        return 0;
    }

    Event *event = p_events.front();
    p_num_events--;
    p_events.pop_front();

    pthread_mutex_unlock(&p_events_mutex);

    return event;
}

/******************************************************************************
* Getter functions
******************************************************************************/
unsigned int  DSPDevice::numDSPs()      const { return p_cores;   }
float         DSPDevice::dspMhz()       const { return p_dsp_mhz; }
unsigned char DSPDevice::dspID()        const { return p_dsp_id;  }
DLOAD_HANDLE  DSPDevice::dload_handle() const { return p_dload_handle;  }

static void work_group_aggregate(std::string& test);

/******************************************************************************
* std::string DSPDevice::sourceTranslation(std::string& source) 
******************************************************************************/
std::string DSPDevice::sourceTranslation(std::string& source) 
{ 
    std::string srcc(source);
    std::string marker("/*EOH Marker*/");
    srcc.replace(srcc.find(marker),marker.length(), dsp_specific_header);
    work_group_aggregate(srcc);
    return srcc; 
}

int DSPDevice::load(const char *filename)
{ 
   if (!p_dload_handle)
   {
       p_dload_handle = DLOAD_create((void*)this);
       DLOAD_initialize(p_dload_handle);
   }

   FILE *fp = fopen(filename, "rb");
   if (!fp) { printf("can't open OpenCL Program file\n"); exit(1); }

   int prog_handle = DLOAD_load(p_dload_handle, fp);
   fclose(fp);
   return prog_handle;
}

DSPDevicePtr DSPDevice::malloc_l2(size_t size)  
    { return p_device_l2_heap.malloc(size); }

void DSPDevice::free_l2(DSPDevicePtr addr)       
    { p_device_l2_heap.free(addr); }

DSPDevicePtr DSPDevice::malloc_code(size_t size)  
    { return p_device_ddr_heap.malloc(size); }

void DSPDevice::free_code(DSPDevicePtr addr)       
    { p_device_ddr_heap.free(addr); }

DSPDevicePtr DSPDevice::malloc_ddr(size_t size)  
    { return p_device_ddr_heap.malloc(size); }

void DSPDevice::free_ddr(DSPDevicePtr addr)       
    { p_device_ddr_heap.free(addr); }

void DSPDevice::mail_to(Msg_t &msg)
{
    static unsigned trans_id = 0xC0DE0000;
    Mailbox::instance()->write(p_tx_mbox, (uint8_t*)&msg, sizeof(Msg_t), trans_id++);
}

bool DSPDevice::mail_query()
{
    unsigned read_cnt=0, write_cnt=0;
    Mailbox::instance()->query(p_rx_mbox, &read_cnt, &write_cnt);
    return (read_cnt != write_cnt);
}

int DSPDevice::mail_from()
{
    uint32_t size_rx, trans_id_rx;
    uint32_t rxmsg;

    Mailbox::instance()->read(p_rx_mbox, (uint8_t*)&rxmsg, &size_rx, &trans_id_rx);
    return rxmsg;
}

int DSPDevice::dma_write(DSPDevicePtr addr, void* host_addr, size_t size)
{
    return Driver::instance()->dma_write(dspID(), addr, 
                                (uint8_t*)host_addr, size);
}


/******************************************************************************
* cl_int DSPDevice::info
******************************************************************************/
cl_int DSPDevice::info(cl_device_info param_name,
                       size_t param_value_size,
                       void *param_value,
                       size_t *param_value_size_ret) const
{
    void *value = 0;
    size_t value_length = 0;

    union 
    {
        cl_device_type cl_device_type_var;
        cl_uint cl_uint_var;
        size_t size_t_var;
        cl_ulong cl_ulong_var;
        cl_bool cl_bool_var;
        cl_device_fp_config cl_device_fp_config_var;
        cl_device_mem_cache_type cl_device_mem_cache_type_var;
        cl_device_local_mem_type cl_device_local_mem_type_var;
        cl_device_exec_capabilities cl_device_exec_capabilities_var;
        cl_command_queue_properties cl_command_queue_properties_var;
        cl_platform_id cl_platform_id_var;
        size_t work_dims[MAX_WORK_DIMS];
    };

    switch (param_name)
    {
        case CL_DEVICE_TYPE:
            SIMPLE_ASSIGN(cl_device_type, CL_DEVICE_TYPE_ACCELERATOR);
            break;

        case CL_DEVICE_VENDOR_ID:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_MAX_COMPUTE_UNITS:
            SIMPLE_ASSIGN(cl_uint, numDSPs());
            break;

        case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
            SIMPLE_ASSIGN(cl_uint, MAX_WORK_DIMS);
            break;

        case CL_DEVICE_MAX_WORK_GROUP_SIZE:
            SIMPLE_ASSIGN(size_t, 0xffffffff);
            break;

        case CL_DEVICE_MAX_WORK_ITEM_SIZES:
            for (int i=0; i<MAX_WORK_DIMS; ++i)
            {
                work_dims[i] = 0xffffffff;
            }
            value_length = MAX_WORK_DIMS * sizeof(size_t);
            value        = &work_dims;
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 1);
            break;

        case CL_DEVICE_MAX_CLOCK_FREQUENCY:
            SIMPLE_ASSIGN(cl_uint, dspMhz() * 1000000);
            break;

        case CL_DEVICE_ADDRESS_BITS:
            SIMPLE_ASSIGN(cl_uint, 32);
            break;

        case CL_DEVICE_MAX_READ_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 0);          //images not supported
            break;

        case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
            SIMPLE_ASSIGN(cl_uint, 0);          // images not supported
            break;

        case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
            SIMPLE_ASSIGN(cl_ulong, DDR_LENGTH);
            break;

        case CL_DEVICE_IMAGE2D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 0);           // images not supported
            break;

        case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_WIDTH:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE3D_MAX_DEPTH:
            SIMPLE_ASSIGN(size_t, 0);           //images not supported
            break;

        case CL_DEVICE_IMAGE_SUPPORT:
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);   //images not supported
            break;

        case CL_DEVICE_MAX_PARAMETER_SIZE:
            SIMPLE_ASSIGN(size_t, 1024);       // ASW TODO
            break;

        case CL_DEVICE_MAX_SAMPLERS:
            SIMPLE_ASSIGN(cl_uint, 0);          //images not supported
            break;

        case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
            SIMPLE_ASSIGN(cl_uint, 64);
            break;

        case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
            SIMPLE_ASSIGN(cl_uint, 64);
            break;

        case CL_DEVICE_SINGLE_FP_CONFIG:
            // ASW TODO: 
            SIMPLE_ASSIGN(cl_device_fp_config,
                          CL_FP_DENORM |
                          CL_FP_INF_NAN |
                          CL_FP_ROUND_TO_NEAREST);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
            SIMPLE_ASSIGN(cl_device_mem_cache_type, CL_READ_WRITE_CACHE);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
            SIMPLE_ASSIGN(cl_uint, 128);
            break;

        case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
            SIMPLE_ASSIGN(cl_ulong, 256*1024);
            break;

        case CL_DEVICE_GLOBAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, DDR_LENGTH);
            break;

        case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
            SIMPLE_ASSIGN(cl_ulong, DDR_LENGTH);
            break;

        case CL_DEVICE_MAX_CONSTANT_ARGS:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_LOCAL_MEM_TYPE:
            SIMPLE_ASSIGN(cl_device_local_mem_type, CL_LOCAL);
            break;

        case CL_DEVICE_LOCAL_MEM_SIZE:
            SIMPLE_ASSIGN(cl_ulong, L2_LENGTH);
            break;

        case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
            // ASW TODO - check answer
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);
            break;

        case CL_DEVICE_HOST_UNIFIED_MEMORY:
            SIMPLE_ASSIGN(cl_bool, CL_FALSE);
            break;

        case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
            SIMPLE_ASSIGN(size_t, 1000);   // 1000 nanoseconds = 1 microsecond
            break;

        case CL_DEVICE_ENDIAN_LITTLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_COMPILER_AVAILABLE:
            SIMPLE_ASSIGN(cl_bool, CL_TRUE);
            break;

        case CL_DEVICE_EXECUTION_CAPABILITIES:
            SIMPLE_ASSIGN(cl_device_exec_capabilities, CL_EXEC_KERNEL);
            break;

        case CL_DEVICE_QUEUE_PROPERTIES:
            SIMPLE_ASSIGN(cl_command_queue_properties,
                          CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE |
                          CL_QUEUE_PROFILING_ENABLE);
            break;

        case CL_DEVICE_NAME:
            // ASW TODO add device number suffix
            STRING_ASSIGN("TMS320C6678 DSP");
            break;

        case CL_DEVICE_VENDOR:
            STRING_ASSIGN("Texas Instruments, Inc.");
            break;

        case CL_DRIVER_VERSION:
            STRING_ASSIGN("" COAL_VERSION);
            break;

        case CL_DEVICE_PROFILE:
            STRING_ASSIGN("FULL_PROFILE");
            break;

        case CL_DEVICE_VERSION:
            STRING_ASSIGN("OpenCL 1.1 TI " COAL_VERSION);
            break;

        case CL_DEVICE_EXTENSIONS:
            STRING_ASSIGN("cl_khr_byte_addressable_store"
                          " cl_khr_fp64")
            break;

        case CL_DEVICE_PLATFORM:
            SIMPLE_ASSIGN(cl_platform_id, 0);
            break;

        case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
            SIMPLE_ASSIGN(cl_uint, 8);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
            SIMPLE_ASSIGN(cl_uint, 4);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
            SIMPLE_ASSIGN(cl_uint, 2);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
            SIMPLE_ASSIGN(cl_uint, 1);
            break;

        case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
            SIMPLE_ASSIGN(cl_uint, 0);
            break;

        case CL_DEVICE_OPENCL_C_VERSION:
            STRING_ASSIGN("OpenCL C 1.1 LLVM " LLVM_VERSION);
            break;

        default:
            return CL_INVALID_VALUE;
    }

    if (param_value && param_value_size < value_length)
        return CL_INVALID_VALUE;

    if (param_value_size_ret)
        *param_value_size_ret = value_length;

    if (param_value)
        std::memcpy(param_value, value, value_length);

    return CL_SUCCESS;
}

/******************************************************************************
* find_next_dim_use
******************************************************************************/
static int find_next_dim_use(const std::string& kbody, int& start, int stop)
{
    int pos1 = kbody.find("get_global_id", start);
    int pos2 = kbody.find("get_local_id", start);
    int pos;

    if      (pos1 == std::string::npos) pos = pos2;
    else if (pos2 == std::string::npos) pos = pos1;
    else                                pos = std::min(pos1, pos2);

    if (pos == std::string::npos || pos > stop) return 0;

    pos = kbody.find("(", pos);
    pos = kbody.find_first_of("012)", pos);
    start = pos+1;

    switch (kbody.at(pos))
    {
        case '0': return 1;
        case '1': return 2;
        case '2': return 3;
        default:  return 0;
    }
}

/*-----------------------------------------------------------------------------
* Search for all get_global_id and get_local_id calls to determine what 
* dimensionality this kernel is written for.  This will be confused by comments
* contatining those strings.  ASW TODO
*----------------------------------------------------------------------------*/
static int determine_dimensionality(const std::string& kbody, int start, int stop)
{
    int dims = 0;
    int this_dim;

    /*-------------------------------------------------------------------------
    * Start is passed by reference and is updated as a side effect
    * by the call to find_next_dim_use
    *------------------------------------------------------------------------*/
    while ((this_dim = find_next_dim_use(kbody, start, stop)) != 0)
        dims = std::max(dims, this_dim);

    return dims;
}


/******************************************************************************
* static void work_group_aggregate(std::string& test)
*
* Find the opening '{' and closing '}' for all kernel functions. Replace the
* opening '{' with "{LOOPS3" and the clogin '}' with "}}".  This will add the
* three nested loop structure for work group aggregation.
******************************************************************************/
static void work_group_aggregate(std::string& test)
{
    const std::string indicator("kernel");
    int pos0 = 0;
    int pos1 = 0;
    int input_size = test.size();

    std::vector<int> kernel_open;
    std::vector<int> kernel_close;

    /*-------------------------------------------------------------------------
    * Continue as long as we continue to find the string "kernel"
    *------------------------------------------------------------------------*/
    while (pos0 <  input_size &&
          (pos1 = test.find(indicator, pos0)) != std::string::npos)
    {
        int pos2 = test.find("{", pos1 + indicator.size());
        int nest_level = 1;
        int pos3 = pos2+1;

        for ( ; nest_level != 0 && pos3 < input_size; ++pos3)
        {
            char c = test[pos3];

            if      (c == '{') nest_level++;
            else if (c == '}') nest_level--;
        }

        pos0 = pos3;

        /*---------------------------------------------------------------------
        * pos2 will be the opening brace, pos3 willbe the closing brace
        *--------------------------------------------------------------------*/
        if (test[--pos3] == '}')
        {
            kernel_open.push_back(pos2);
            kernel_close.push_back(pos3);
        }
    }

    /*-------------------------------------------------------------------------
    * We cached the positions to replace and we work backwards to prevent a
    * replacement from invalidating our other found positions.
    *------------------------------------------------------------------------*/
    for (int i = kernel_close.size()-1; i >= 0; --i)
    {
        int dims = determine_dimensionality(test, kernel_open[i], kernel_close[i]);
        std::string opening_replacement("{");
        std::string closing_replacement("}");

        if (dims > 0) closing_replacement = "}}";

        switch (dims)
        {
            case 3: opening_replacement = "{LOOPS3{"; break;
            case 2: opening_replacement = "{LOOPS2{"; break;
            case 1: opening_replacement = "{LOOPS1{"; break;
            default: break;
        }

        test.replace(kernel_close[i], 1, closing_replacement);
        test.replace(kernel_open[i], 1, opening_replacement);
    }
}



/******************************************************************************
* Call back functions from the target loader
******************************************************************************/
extern "C"
{

/*****************************************************************************/
/* DLIF_ALLOCATE() - Return the load address of the segment/section          */
/*      described in its parameters and record the run address in            */
/*      run_address field of DLOAD_MEMORY_REQUEST.                           */
/*****************************************************************************/
BOOL DLIF_allocate(void* client_handle, struct DLOAD_MEMORY_REQUEST *targ_req)
{
   DSPDevice* device = (DSPDevice*) client_handle;

   /*------------------------------------------------------------------------*/
   /* Get pointers to API segment and file descriptors.                      */
   /*------------------------------------------------------------------------*/
   struct DLOAD_MEMORY_SEGMENT* obj_desc = targ_req->segment;

   uint32_t addr = (uint32_t)device->malloc_code(obj_desc->memsz_in_bytes);
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
   DSPDevice* device = (DSPDevice*) client_handle;
   device->free_code((DSPDevicePtr)ptr->target_address);

   printf("DLIF_free: %d bytes starting at 0x%x\n",
                      ptr->memsz_in_bytes, (uint32_t)ptr->target_address);

   return 1;
}

void dump_hex(char *addr, int bytes)
{
    int cnt = 0;

    printf("\n");
    while (cnt < bytes)
    {
        for (int col = 0; col < 16; ++col)
        {
            printf("%02x ", addr[cnt++] & 0xff);
            if (cnt >= bytes) break;
        }
        printf("\n");
    }
}

/*****************************************************************************/
/* DLIF_WRITE() - Write updated (relocated) segment contents to target       */
/*      memory.                                                              */
/*****************************************************************************/
BOOL DLIF_write(void* client_handle, struct DLOAD_MEMORY_REQUEST* req)
{
   struct DLOAD_MEMORY_SEGMENT* obj_desc = req->segment;
   DSPDevice* device = (DSPDevice*) client_handle;
   int dsp_id = device->dspID();

   Driver::instance()->dma_write (dsp_id,
                         (uint32_t)obj_desc->target_address,
                         (uint8_t*)req->host_address,
                         obj_desc->memsz_in_bytes);

#if DEBUG
    printf("DLIF_write (dsp:%d): %d bytes starting at 0x%x\n",
               dsp_id, obj_desc->memsz_in_bytes, 
               (uint32_t)obj_desc->target_address);

    dump_hex((char*)req->host_address, obj_desc->memsz_in_bytes);
#endif
    
    extern DSPProgram::segment_list *segments;

    if (segments) segments->push_back
        (DSPProgram::seg_desc((DSPDevicePtr)obj_desc->target_address, obj_desc->memsz_in_bytes));

    return 1;
}

}

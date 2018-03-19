#include <assert.h>
#include "executor.h"
#include "executor_impl.h"
#include "parameters.h"
#include "util.h"
#include "trace.h"


using namespace tidl;

using std::unique_ptr;

Executor::Executor(DeviceType core_type, const DeviceIds& ids,
                   const Configuration& configuration)
{
    pimpl_m = unique_ptr<ExecutorImpl>
              { new ExecutorImpl(core_type, ids) };
    pimpl_m->Initialize(configuration);
}



// Pointer to implementation idiom: https://herbsutter.com/gotw/_100/:
// Both unique_ptr and shared_ptr can be instantiated with an incomplete type
// unique_ptr's destructor requires a complete type in order to invoke delete
// By writing it yourself in the implementation file, you force it to be
// defined in a place where impl is already defined, and this successfully
// prevents the compiler from trying to automatically generate the destructor
// on demand in the caller’s code where impl is not defined.
Executor::~Executor() = default;

uint32_t Executor::GetNumDevicesSupportingTIDL()
{
    return Device::GetNumDevicesSupportingTIDL();
}

ExecutorImpl::ExecutorImpl(DeviceType core_type, const DeviceIds& ids):
    configuration_m(),
    shared_networkparam_heap_m(nullptr, &__free_ddr),
    device_ids_m(ids),
    core_type_m(core_type)
{
    std::string name;
    if (core_type_m == DeviceType::DSP)
        name  = "../dsp/ocl_wrapper.out";
    else if (core_type_m == DeviceType::DLA)
        name = STRING(SETUP_KERNEL) ";" STRING(INIT_KERNEL) ";" STRING(PROCESS_KERNEL) ";" STRING(CLEANUP_KERNEL);

    device_m = Device::Create(core_type_m, ids, name);

}


const ExecutionObjects& Executor::GetExecutionObjects() const
{
    return pimpl_m->execution_objects_m;
}

bool ExecutorImpl::Initialize(const Configuration& configuration)
{
    configuration_m = configuration;

    // Allocate, initialize TIDL_CreateParams object
    up_malloc_ddr<TIDL_CreateParams> shared_createparam(
                                            malloc_ddr<TIDL_CreateParams>(),
                                            &__free_ddr);
    InitializeNetworkCreateParam(shared_createparam.get());

    // Read network from file into network struct in TIDL_CreateParams
    sTIDL_Network_t *net = &(shared_createparam.get())->net;

    bool status = ReadBinary(configuration_m.netBinFile,
                             reinterpret_cast<char *>(net),
                             sizeof(sTIDL_Network_t));
    assert(status != false);

    //TODO: Why is this set here?
    net->interElementSize = 4;

    // Call a setup kernel to allocate and fill network parameters
    InitializeNetworkParams(shared_createparam.get());

    const ArgInfo create_arg(shared_createparam.get(),
                             sizeof(TIDL_CreateParams));
    const ArgInfo param_heap_arg(shared_networkparam_heap_m.get(),
                                 configuration_m.PARAM_HEAP_SIZE);
    for (auto ids : device_ids_m)
    {
        uint8_t index = static_cast<uint8_t>(ids);
        execution_objects_m.push_back(
             unique_ptr<ExecutionObject>
             {new ExecutionObject(device_m.get(), index,
                                  create_arg, param_heap_arg,
                                  configuration_m.EXTMEM_HEAP_SIZE)} );
    }

    for (auto &eo : execution_objects_m)
        eo->RunAsync(ExecutionObject::CallType::INIT);

    for (auto &eo : execution_objects_m)
        eo->Wait(ExecutionObject::CallType::INIT);

    return true;
}


bool ExecutorImpl::InitializeNetworkParams(TIDL_CreateParams *cp)
{
    // Determine size of network parameters buffer, allocate it
    size_t networkparam_size =
                        GetBinaryFileSize(configuration_m.paramsBinFile);

    up_malloc_ddr<char> networkparam(malloc_ddr<char>(networkparam_size),
                                &__free_ddr);

    // Read network parameters from bin file into buffer
    bool status = ReadBinary(configuration_m.paramsBinFile,
                             networkparam.get(),
                             networkparam_size);
    assert(status != false);

    // Allocate a buffer for passing parameters to the kernel
    up_malloc_ddr<OCL_TIDL_SetupParams> setupParams(
                                            malloc_ddr<OCL_TIDL_SetupParams>(),
                                            &__free_ddr);

    setupParams->enableTrace = OCL_TIDL_TRACE_OFF;
    setupParams->networkParamHeapSize = configuration_m.PARAM_HEAP_SIZE;
    setupParams->noZeroCoeffsPercentage = configuration_m.noZeroCoeffsPercentage;
    setupParams->sizeofTIDL_CreateParams = sizeof(TIDL_CreateParams);
    setupParams->offsetofNet = offsetof(TIDL_CreateParams, net);

    // Allocate buffer for a network parameter heap. Used by the setup
    // kernel to allocate and initialize network parameters for the layers
    shared_networkparam_heap_m.reset(malloc_ddr<char>(setupParams->networkParamHeapSize));

    KernelArgs args = { ArgInfo(cp, sizeof(TIDL_CreateParams)),
                        ArgInfo(networkparam.get(), networkparam_size),
                        ArgInfo(shared_networkparam_heap_m.get(),
                                setupParams->networkParamHeapSize),
                        ArgInfo(setupParams.get(),
                                sizeof(OCL_TIDL_SetupParams)) };

    // Execute kernel on first available device in the Executor
    uint8_t id = static_cast<uint8_t>(*(device_ids_m.cbegin()));
    unique_ptr<Kernel> K {new Kernel(device_m.get(), STRING(SETUP_KERNEL),
                                     args, id)};
    K->RunAsync();
    K->Wait();

    if (setupParams->errorCode != OCL_TIDL_SUCCESS)
        throw Exception(setupParams->errorCode,
                        __FILE__, __FUNCTION__, __LINE__);

    return status;
}


void ExecutorImpl::Cleanup()
{
    for (auto &eo : execution_objects_m)
        eo->RunAsync(ExecutionObject::CallType::CLEANUP);

    for (auto &eo : execution_objects_m)
        eo->Wait(ExecutionObject::CallType::CLEANUP);
}


void ExecutorImpl::InitializeNetworkCreateParam(TIDL_CreateParams *CP)
{
    CP->currCoreId           = tidl::internal::CURR_CORE_ID;
    CP->currLayersGroupId    = tidl::internal::CURR_LAYERS_GROUP_ID;
    CP->l1MemSize            = tidl::internal::DMEM0_SIZE;
    CP->l2MemSize            = tidl::internal::DMEM1_SIZE;
    CP->l3MemSize            = tidl::internal::OCMC_SIZE;

    CP->quantHistoryParam1   = tidl::internal::QUANT_HISTORY_PARAM1;
    CP->quantHistoryParam2   = tidl::internal::QUANT_HISTORY_PARAM2;
    CP->quantMargin          = tidl::internal::QUANT_MARGIN;
    CP->optimiseExtMem       = TIDL_optimiseExtMemL1;
}

Exception::Exception(const std::string& error, const std::string& file,
                     const std::string& func, uint32_t line_no)
{

    message_m = "TIDL Error: [";
    message_m += file;
    message_m += ", ";
    message_m += func;
    message_m += ", ";
    message_m += std::to_string(line_no);
    message_m += "]: ";
    message_m += error;
}

Exception::Exception(int32_t errorCode, const std::string& file,
                     const std::string& func, uint32_t line_no)
{
    message_m = "TIDL Error: [";
    message_m += file;
    message_m += ", ";
    message_m += func;
    message_m += ", ";
    message_m += std::to_string(line_no);
    message_m += "]: ";

    if (errorCode == OCL_TIDL_ERROR)
        message_m += "";
    else if (errorCode == OCL_TIDL_ALLOC_FAIL)
        message_m += "Allocation failed on device";
    else if (errorCode == OCL_TIDL_MEMREC_ALLOC_FAIL)
        message_m += "Memrec allocation failed on device";
    else if (errorCode == OCL_TIDL_PROCESS_FAIL)
        message_m += "Process call failed on device";
    else if (errorCode == OCL_TIDL_CREATE_PARAMS_MISMATCH)
        message_m += "TIDL_CreateParams definition inconsistent across host"
                     "and device.";
    else
        message_m += std::to_string(errorCode);

}

const char* Exception::what() const noexcept
{
    return message_m.c_str();
}


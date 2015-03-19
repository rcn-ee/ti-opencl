#ifndef __DSP_PROGRAM_H__
#define __DSP_PROGRAM_H__

#include "device.h"
#include "../deviceinterface.h"
#include <vector>

namespace llvm
{
    class ExecutionEngine;
    class Module;
}

namespace Coal
{

class DSPDevice;
class Program;

class DSPProgram : public DeviceProgram
{
    public:
      struct seg_desc
      {
          seg_desc(DSPDevicePtr p, int s) : ptr(p), size(s) {}
          DSPDevicePtr ptr;
          unsigned size;
      };
  
      typedef std::vector<seg_desc> segment_list;

    public:
        DSPProgram(DSPDevice *device, Program *program);
        ~DSPProgram();

        bool linkStdLib() const;
        void createOptimizationPasses(llvm::PassManager *manager, bool optimize);
        bool build(llvm::Module *module);
        DSPDevicePtr query_symbol(const char *symname);
        DSPDevicePtr data_page_ptr();
        void load();
        bool is_loaded() const;

    private:
        DSPDevice    *p_device;
        Program      *p_program;
        llvm::Module *p_module;
        int           p_program_handle;
        char          p_outfile[32];
        bool          p_loaded;
        segment_list  p_segments_written;
};
}
#endif

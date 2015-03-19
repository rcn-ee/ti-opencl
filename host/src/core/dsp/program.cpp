#include "program.h"
#include "device.h"
#include "kernel.h"

#include "../program.h"

#include <llvm/PassManager.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Analysis/Verifier.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Bitcode/ReaderWriter.h>

#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>

#define KEEP_FILES 1

timespec getTime()
{
    struct timespec tp;
    if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0)
        clock_gettime(CLOCK_REALTIME, &tp);
    return tp;
}

double ts_to_double(const timespec &t)
    { return ((double)t.tv_nsec) /1000000000.0 + (double)t.tv_sec; }

double tsdiff (const timespec& start, const timespec& end)
    { return ts_to_double(end) - ts_to_double(start); }


using namespace Coal;

DSPProgram::DSPProgram(DSPDevice *device, Program *program)
: DeviceProgram(), p_device(device), p_program(program), p_program_handle(-1), p_loaded(false)
{
}

DSPProgram::~DSPProgram()
{
#ifndef KEEP_FILES
    struct stat statbuf;
    if (stat(p_outfile, &statbuf) == 0)
        if (remove(p_outfile) != 0)
            printf("File \"%s\" cannot be removed", p_outfile);
#endif
}

DSPProgram::segment_list *segments;

void DSPProgram::load()
{
    segments = &p_segments_written;
    p_program_handle = p_device->load(p_outfile);
    segments = NULL;
    p_loaded = true;
}

bool DSPProgram::is_loaded() const
{
    return p_loaded;
}

bool DSPProgram::linkStdLib() const
{
    return false;
}

DSPDevicePtr DSPProgram::data_page_ptr() 
{
    DSPDevicePtr p;

    if (!is_loaded()) load();

    DLOAD_get_static_base(p_device->dload_handle(), p_program_handle,  &p);
    return p;
}

void DSPProgram::createOptimizationPasses(llvm::PassManager *manager, bool optimize)
{
    if (optimize)
    {
        /*
         * Inspired by code from "The LLVM Compiler Infrastructure"
         */
        manager->add(llvm::createDeadArgEliminationPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createFunctionInliningPass());
        manager->add(llvm::createPruneEHPass());   // Remove dead EH info.
        manager->add(llvm::createGlobalOptimizerPass());
        manager->add(llvm::createGlobalDCEPass()); // Remove dead functions.
        manager->add(llvm::createArgumentPromotionPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createJumpThreadingPass());

        //ASW TODO maybe turn off re: pete.  might gen bad xlator input 
        //manager->add(llvm::createScalarReplAggregatesPass());

        manager->add(llvm::createFunctionAttrsPass()); // Add nocapture.
        manager->add(llvm::createGlobalsModRefPass()); // IP alias analysis.
        manager->add(llvm::createLICMPass());      // Hoist loop invariants.
        manager->add(llvm::createGVNPass());       // Remove redundancies.
        manager->add(llvm::createMemCpyOptPass()); // Remove dead memcpys.
        manager->add(llvm::createDeadStoreEliminationPass());
        manager->add(llvm::createInstructionCombiningPass());
        manager->add(llvm::createJumpThreadingPass());
        manager->add(llvm::createCFGSimplificationPass());
    }
}


class execvp_args
{
  public:
    execvp_args() : arg_list(0) { }
    void push(const char *str) { arg_list.push_back(str); }
    char **argv()    
    { 
        int i;
        char **ptr = (char**) malloc(sizeof(char*) * (arg_list.size()+1));

        for (i = 0; i < arg_list.size(); ++i) 
            ptr[i] = const_cast<char*> (arg_list[i]);

        ptr[i] = 0;

        return ptr;
    }

  private:
    std::vector<const char *> arg_list;
};


void process_cl6x_options(std::string options, execvp_args& args)
{
    std::istringstream options_stream(options);
    std::string token;

    while (options_stream >> token)
    {
        if (token.find(".obj") != std::string::npos)
            args.push(token.c_str());
        if (token.find(".ae66") != std::string::npos)
            args.push(token.c_str());
        if (token.find(".out") != std::string::npos)
            args.push(token.c_str());
        if (token.find(".lib") != std::string::npos)
            args.push(token.c_str());
        if (token.find(".cmd") != std::string::npos)
            args.push(token.c_str());
    }
}

static int run_cl6x(char *filename, std::string options)
{
    char outfile[32]; strcpy(outfile, filename); strcat(outfile, ".out");
    char mapfile[32]; strcpy(mapfile, filename); strcat(mapfile, ".map");

    execvp_args args;
    
    args.push("cl6x"); 
    args.push("-q"); 
    args.push("-O3"); 
    args.push("--use_llvm"); 
    args.push("-mv6600"); 
    args.push("-mt"); 

#ifdef KEEP_FILES
    args.push("-mu");  
    args.push("-mw");  
    args.push("-os");  
    args.push("-k");  
    args.push("--z");
#endif

    args.push("-ft=/tmp"); 
    args.push("-fs=/tmp"); 
    args.push("-fr=/tmp");
    args.push("--symdebug:none"); 
    args.push("--gen_pic"); 
    args.push(filename); 
    
    
    args.push("-z");  
    args.push("-cr");
    args.push("-lrts6600_elf_pic.lib");
    args.push("--dynamic=exe"); 
    args.push("--relocatable"); 
    args.push("--single_data_segment"); 
    args.push("--no_entry_point");
    args.push("-o"); 
    args.push(outfile);
#ifdef KEEP_FILES
    args.push("-m"); 
    args.push(mapfile); 
#endif

    /*-------------------------------------------------------------------------
    * Any libraries or object files need to go last to resolve references
    *------------------------------------------------------------------------*/
    process_cl6x_options(options, args); 

    pid_t child_pid = fork();

    /*-------------------------------------------------------------------------
    * If we are the child process
    *------------------------------------------------------------------------*/
    if (child_pid == 0)
    {
        char **argv = args.argv();
        execvp("cl6x", argv);
        free(argv);

        /*---------------------------------------------------------------------
        * If execvp returns, it must have failed.
        *--------------------------------------------------------------------*/
        printf("Could not execute the dsp compiler\n");
        exit(0);
    }

    /*-------------------------------------------------------------------------
    * Parent process waits for command to complete
    *------------------------------------------------------------------------*/
    else
    {
        int   child_status;

        while (wait(&child_status) != child_pid) ;

        return child_status;
    }
}

bool DSPProgram::build(llvm::Module *module)
{
    p_module = module;

    char name_template[] =  "/tmp/openclXXXXXX";
    int pFile = mkstemp(name_template);

    strcpy(p_outfile, name_template);
    strcat(p_outfile, ".out");

    if (pFile != -1)
    {
        llvm::raw_fd_ostream ostream(pFile, false);
        llvm::WriteBitcodeToFile(p_module, ostream);
        ostream.flush();
        run_cl6x(name_template,
                   p_program->deviceDependentCompilerOptions(p_device));

#ifndef KEEP_FILES
        struct stat statbuf;
        if (stat(name_template, &statbuf) == 0)
            if (remove(name_template) != 0)
                printf("File \"%s\" cannot be removed", name_template);

        char objfile[32]; strcpy(objfile, name_template); strcat(objfile, ".obj");
        if (stat(objfile, &statbuf) == 0)
            if (remove(objfile) != 0)
                printf("File \"%s\" cannot be removed", objfile);
#endif
    }

    return true;
}

DSPDevicePtr DSPProgram::query_symbol(const char *symname)
{
    DSPDevicePtr addr;

    bool found = DLOAD_query_symbol(p_device->dload_handle(), p_program_handle,
                                    symname, &addr);

    return (found) ? addr : 0;
}

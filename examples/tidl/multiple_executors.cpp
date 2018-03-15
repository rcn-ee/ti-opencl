/******************************************************************************
 * Copyright (c) 2018, Texas Instruments Incorporated - http://www.ti.com/
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

//! @file multiple_executors.cpp
//! Illustrates how to setup multiple Executor instances using
//! non-overlapping sets of device ids and running the Executor instances
//! in parallel - each in its own thread

#include <signal.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <functional>
#include <algorithm>
#include <pthread.h>

#include "executor.h"
#include "execution_object.h"
#include "configuration.h"


using namespace tidl;

void* run_network(void *data);

static bool ReadFrame(ExecutionObject&     eo,
                      int                  frame_idx,
                      const Configuration& configuration,
                      std::istream&        input_file);

static bool WriteFrame(const ExecutionObject &eo,
                       std::ostream& output_file);

static void DisplayHelp();

struct ThreadArg
{
    std::string config_file;
    DeviceIds ids;
    ThreadArg(const DeviceIds& ids, const std::string& s):
        ids(ids), config_file(s) {}

};

int main(int argc, char *argv[])
{
    // Catch ctrl-c to ensure a clean exit
    signal(SIGABRT, exit);
    signal(SIGTERM, exit);

    ThreadArg arg1({DeviceId::ID2, DeviceId::ID3},
                   "testvecs/config/infer/tidl_config_jseg21_tiscapes.txt");

    ThreadArg arg2({DeviceId::ID0, DeviceId::ID1},
                   "testvecs/config/infer/tidl_config_j11_cifar.txt");


    // Run network 1 in a thread
    std::cout << "Running network "
              << arg1.config_file.substr(arg1.config_file.find("tidl"))
              << " on EVEs: ";
    for (DeviceId id : arg1.ids)
        std::cout << static_cast<int>(id) << " ";
    std::cout << std::endl;

    pthread_t network_thread_1;
    pthread_create(&network_thread_1, 0, &run_network, &arg1);

    // Run network 2 in a thread
    std::cout << "Running network "
              << arg2.config_file.substr(arg2.config_file.find("tidl"))
              << " on EVEs: ";
    for (DeviceId id : arg2.ids)
        std::cout << static_cast<int>(id) << " ";
    std::cout << std::endl;

    pthread_t network_thread_2;
    pthread_create(&network_thread_2, 0, &run_network, &arg2);

    // Wait for both networks to complete
    pthread_join(network_thread_1, 0);
    pthread_join(network_thread_2, 0);

    return EXIT_SUCCESS;
}


void* run_network(void *data)
{
    const ThreadArg* arg = static_cast<const ThreadArg *>(data);

    const DeviceIds& ids = arg->ids;
    const std::string& config_file = arg->config_file;

    // Read the TI DL configuration file
    Configuration configuration;
    bool status = configuration.ReadFromFile(config_file);
    assert (status != false);

    // Open input and output files
    std::ifstream input_data_file(configuration.inData, std::ios::binary);
    std::ofstream output_data_file(configuration.outData, std::ios::binary);
    assert (input_data_file.good());
    assert (output_data_file.good());

    // Determine input frame size from configuration
    size_t frame_sz = configuration.inWidth * configuration.inHeight *
                      configuration.inNumChannels;

    // Create a executor with the approriate core type, number of cores
    // and configuration specified
    Executor executor(DeviceType::DLA, ids, configuration);

    const ExecutionObjects& execution_objects = executor.GetExecutionObjects();
    int num_eos = execution_objects.size();

    // Allocate input and output buffers for each execution object
    std::vector<void *> buffers;
    for (auto &eo : execution_objects)
    {
        ArgInfo in  = { ArgInfo(malloc_ddr<char>(frame_sz), frame_sz)};
        ArgInfo out = { ArgInfo(malloc_ddr<char>(frame_sz), frame_sz)};
        eo->SetInputOutputBuffer(in, out);

        buffers.push_back(in.ptr());
        buffers.push_back(out.ptr());
    }

    // Process frames with available execution objects in a pipelined fashion
    // additional num_eos iterations to flush the pipeline (epilogue)
    for (int frame_idx = 0;
         frame_idx < configuration.numFrames + num_eos; frame_idx++)
    {
        ExecutionObject* eo = execution_objects[frame_idx % num_eos].get();

        // Wait for previous frame on the same eo to finish processing
        if (eo->ProcessFrameWait())
            WriteFrame(*eo, output_data_file);

        // Read a frame and start processing it with current eo
        if (ReadFrame(*eo, frame_idx, configuration, input_data_file))
            eo->ProcessFrameStartAsync();
    }


    for (auto b : buffers)
        __free_ddr(b);

    input_data_file.close();
    output_data_file.close();

    return nullptr;
}


bool ReadFrame(ExecutionObject &eo, int frame_idx,
               const Configuration& configuration,
               std::istream& input_file)
{
    if (frame_idx >= configuration.numFrames)
        return false;

    char*  frame_buffer = eo.GetInputBufferPtr();
    assert (frame_buffer != nullptr);

    input_file.read(eo.GetInputBufferPtr(),
                    eo.GetInputBufferSizeInBytes());

    if (input_file.eof())
        return false;

    assert (input_file.good());

    // Set the frame index  being processed by the EO. This is used to
    // sort the frames before they are output
    eo.SetFrameIndex(frame_idx);

    if (input_file.good())
        return true;

    return false;
}

bool WriteFrame(const ExecutionObject &eo, std::ostream& output_file)
{
    output_file.write(
            eo.GetOutputBufferPtr(), eo.GetOutputBufferSizeInBytes());
    assert(output_file.good() == true);

    if (output_file.good())
        return true;

    return false;
}




#define STRING(S)  XSTRING(S)
#define XSTRING(S) #S
void DisplayHelp()
{
    std::cout << "Usage: tidl -c <path to config file>\n"
              #ifdef BUILDID
              << "Version: " << STRING(BUILDID) << "\n"
              #endif
                 "Optional arguments:\n"
                 " -n <number of cores> Number of cores to use (1 - 4)\n"
                 " -t <d|e>             Type of core. d -> DSP, e -> DLA\n"
                 " -v                   Verbose output during execution\n"
                 " -h                   Help\n";
}

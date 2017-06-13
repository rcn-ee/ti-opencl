/******************************************************************************
 * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Texas Instruments Incorporated nor the
 *        names of its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/
#ifndef MCTD_CONFIG_H_
#define MCTD_CONFIG_H_

#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <set>
#include "../src/core/error_report.h"

extern "C"
{
#include <json-c/json.h>
}

#define MCTD_CONFIG_FILE "/etc/ti-mctd/ti_mctd_config.json"
#define MAX_CMEM_BLOCKS  4
#define INVALID_CMEM_BLOCKID  -2
#define DEFAULT_LINUX_SHMEM_SIZE_KB  128

/******************************************************************************
* class MctDaemonConfig 
*    This class will read in OpenCL Multicore Daemon Config from a JSON file.  
******************************************************************************/
class MctDaemonConfig 
{
public:

  MctDaemonConfig() :
          cmem_block_offchip_(INVALID_CMEM_BLOCKID),
          cmem_block_onchip_(INVALID_CMEM_BLOCKID),
          linux_shmem_size_KB_(DEFAULT_LINUX_SHMEM_SIZE_KB)
  {
    struct json_object *oclcfg = json_object_from_file(MCTD_CONFIG_FILE);
    if (oclcfg == nullptr)
      ReportError(tiocl::ErrorType::Fatal,
                  tiocl::ErrorKind::DaemonConfigOpenError);

    struct json_object *offchip, *onchip, *comp_unit_list, *shmem_size;
    if (! json_object_object_get_ex(oclcfg, "cmem-block-offchip", &offchip))
      ReportError(tiocl::ErrorType::Fatal, tiocl::ErrorKind::CMEMMinBlocks);
    cmem_block_offchip_ = json_object_get_int(offchip);
    if (cmem_block_offchip_ < 0 || cmem_block_offchip_ >= MAX_CMEM_BLOCKS)
      ReportError(tiocl::ErrorType::Fatal, tiocl::ErrorKind::CMEMInvalidBlockId,
                  cmem_block_offchip_);

    if (json_object_object_get_ex(oclcfg, "cmem-block-onchip", &onchip))
    {
      cmem_block_onchip_ = json_object_get_int(onchip);
      if (cmem_block_onchip_ < 0 || cmem_block_onchip_ >= MAX_CMEM_BLOCKS)
      ReportError(tiocl::ErrorType::Warning,
                  tiocl::ErrorKind::CMEMInvalidBlockId, cmem_block_onchip_);
    }

    if (json_object_object_get_ex(oclcfg, "compute-unit-list", &comp_unit_list))
    {
      comp_units_ = ParseCompUnitList(json_object_get_string(comp_unit_list));

      // Prune set in config file with existing dsps in the system
      // - Enables same OpenCL build/config for both AM571x and AM572x
      int num_compute_units = 0;
      #if defined (DSPC868X)
        num_compute_units = 8;
      #elif defined (DEVICE_AM57)
        DIR *dir = opendir("/proc/device-tree/ocp");
        if (dir == nullptr)
          ReportError(tiocl::ErrorType::Fatal,
                      tiocl::ErrorKind::FailedToOpenFileName,
                      "/proc/device-tree/ocp");

        while (dirent * entry = readdir(dir))
        {
          if (entry->d_name[0] && entry->d_name[0] == 'd' &&
              entry->d_name[1] && entry->d_name[1] == 's' &&
              entry->d_name[2] && entry->d_name[2] == 'p' &&
              entry->d_name[3] && entry->d_name[3] == '@')
          ++num_compute_units;
        }

        closedir(dir);
      #else
        DIR *dir = opendir("/dev");
        if (dir == nullptr)
          ReportError(tiocl::ErrorType::Fatal,
                      tiocl::ErrorKind::FailedToOpenFileName, "/dev");

        while (dirent * entry = readdir(dir))
        {
          if (entry->d_name[0] && entry->d_name[0] == 'd' &&
              entry->d_name[1] && entry->d_name[1] == 's' &&
              entry->d_name[2] && entry->d_name[2] == 'p' &&
              entry->d_name[3] && isdigit(entry->d_name[3]))
          ++num_compute_units;
        }

        closedir(dir);
      #endif

      for (auto it = comp_units_.begin(); it != comp_units_.end(); )
        if (*it >= num_compute_units)  it = comp_units_.erase(it);
        else                           it++;
    }

    if (json_object_object_get_ex(oclcfg, "linux-shmem-size-KB", &shmem_size))
    {
      linux_shmem_size_KB_ = json_object_get_int(shmem_size);
      if (linux_shmem_size_KB_ <= 0)
        linux_shmem_size_KB_ = DEFAULT_LINUX_SHMEM_SIZE_KB;
    }

    json_object_put(oclcfg);  // decrement refcount and free
  }

  ~MctDaemonConfig() = default;

  std::set<uint8_t> ParseCompUnitList(const char *comp_unit_list)
  {
    std::set<uint8_t> comp_units;
    const std::string cu = comp_unit_list;
    std::stringstream ss(cu);
    int i;
    while (ss >> i)
    {
      comp_units.insert(i);
      if (ss.peek() == ',')  ss.ignore();
    }
    return comp_units;
  }

  int32_t GetCmemBlockOffChip()    const { return cmem_block_offchip_; }
  int32_t GetCmemBlockOnChip()     const { return cmem_block_onchip_; }
  int32_t GetLinuxShmemSizeKB()    const { return linux_shmem_size_KB_; }
  std::set<uint8_t>& GetCompUnits()      { return comp_units_; }

private:
  int32_t  cmem_block_offchip_;
  int32_t  cmem_block_onchip_;
  uint32_t linux_shmem_size_KB_;
  std::set<uint8_t> comp_units_;

  /*-------------------------------------------------------------------------
  * Prevent copy construction or assignment
  *------------------------------------------------------------------------*/
  MctDaemonConfig            (const MctDaemonConfig &) =delete;
  MctDaemonConfig& operator= (const MctDaemonConfig &) =delete;
};

#endif  // MCTD_CONFIG_H_

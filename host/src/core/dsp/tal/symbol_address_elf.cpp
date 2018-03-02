/******************************************************************************
 * Copyright (c) 2013-2016, Texas Instruments Incorporated - http://www.ti.com/
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

#include <gelf.h>
#include <libelf.h>
#include <fcntl.h>
#include <unistd.h>

#include "symbol_address_elf.h"
#include "../error_report.h"

using namespace tiocl;

SymbolAddressLookupELF::SymbolAddressLookupELF(const std::string &binary_filename):
    binary_filename_(binary_filename)
{
    // Initialize with ALL potential symbol lookups
    name_to_address_ = { {"kernel_config_l2",       0},
                         {"ocl_local_mem_start",    0},
                         {"ocl_local_mem_size",     0},
                         {"ocl_global_mem_start",   0},
                         {"ocl_global_mem_size",    0},
                         {"ocl_msmc_mem_start",     0},
                         {"ocl_msmc_mem_size",      0},
                         {"mbox_d2h_phys",          0},
                         {"mbox_d2h_size",          0},
                         {"mbox_h2d_phys",          0},
                         {"mbox_h2d_size",          0} };
    InitializeLookup();
}

SymbolAddressLookupELF::~SymbolAddressLookupELF()
{ }

void SymbolAddressLookupELF::InitializeLookup() const
{
    // Initialize libelf
    if (elf_version(EV_CURRENT) == EV_NONE)
        ReportError(ErrorType::Fatal, ErrorKind::ELFLibraryInitFailed, elf_errmsg(-1));

    // Open a file descriptor corresponding to the ELF binary
    int fd = open(binary_filename_.c_str(), O_RDONLY, 0);
    if (fd < 0)
        ReportError(ErrorType::Fatal, ErrorKind::FailedToOpenFileName, binary_filename_.c_str());

    // Create an ELF handle from the file descriptor
    Elf* elf_file = elf_begin(fd, ELF_C_READ, NULL);
    if (elf_file == nullptr || (elf_kind(elf_file) != ELF_K_ELF))
    {
        close(fd);
        ReportError(ErrorType::Fatal, ErrorKind::ELFLibraryInitFailed, elf_errmsg(-1));
    }


    // Walk through sections in the ELF file
    Elf_Scn *section = nullptr;
    while ((section = elf_nextscn(elf_file, section)) != nullptr)
    {
        GElf_Shdr shdr;
        gelf_getshdr(section, &shdr);

        // Look for section with the symbol table
        if (shdr.sh_type != SHT_SYMTAB)
            continue;

        Elf_Data *elf_data = NULL;
        elf_data = elf_getdata(section, elf_data);

        int number_of_symbols = shdr.sh_size / shdr.sh_entsize;

        // Walk through the symbol table entries
        for (int i = 0; i < number_of_symbols; i++)
        {
            GElf_Sym symbol;
            gelf_getsym(elf_data, i, &symbol);

            // If symbol binding is not STB_GLOBAL, ignore it
            if (ELF32_ST_BIND(symbol.st_info) != STB_GLOBAL)
                continue;

            const char *sym_name = elf_strptr(elf_file, shdr.sh_link,
                                              symbol.st_name);

            std::map<std::string, DSPDevicePtr>::iterator it;
            if ((it = name_to_address_.find(sym_name)) != name_to_address_.end()) {
                it->second = symbol.st_value;
            }
        }
    }


    elf_end(elf_file);
    close (fd);
}

DSPDevicePtr SymbolAddressLookupELF::GetAddress(const std::string &symbol_name) const
{
    std::map<std::string, DSPDevicePtr>::const_iterator cit =
        name_to_address_.find(symbol_name);

    if (cit == name_to_address_.end() || cit->second == 0)
        ReportError(ErrorType::Fatal, ErrorKind::ELFSymbolAddressNotCached,
                    symbol_name.c_str());

    return cit->second;
}

const SymbolAddressLookup* tiocl::CreateSymbolAddressLookup(const std::string& binary_file)
{
    return new SymbolAddressLookupELF(binary_file);
}

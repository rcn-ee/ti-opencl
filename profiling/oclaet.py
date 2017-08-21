"""
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
"""

import sys
sys.path.append('/usr/share/ti/opencl/profiling')
from classes.core import core_data
from classes.event import event_data
from classes.kernel import kernel_data
from classes.graph import plot
from classes.table import table

import json
import os
import glob

"""Ensures data.txt is the only file in specified profiling aetdata"""
def cleanup_files():
    file_path = sys.argv[len(sys.argv)-1]
    if not os.path.isfile(file_path):
        print "Ensure valid data is in " + file_path
        exit(0)

    """ remove all files except aetdata.txt """
    file_dir = os.path.dirname(file_path)
    for old_file in glob.glob(file_dir + '/*.*'):
        if not old_file.endswith('aetdata.txt'):
            os.remove(old_file)


"""Prints Profiling JSON object to file

Args:
    aggregated_kernels: list of kernel_data objects containing kernel profiling data
"""
def print_json(aggregated_kernels):
    kernels_info = []
    profiling_data = {}

    """ Get kernel information for each kernel """
    for kernel in aggregated_kernels:
        """ form list of kernel profiling data """
        kernel_info = kernel.make_kernel_json()
        kernels_info.append(kernel_info)
    profiling_data['Profiling Data'] = kernels_info

    """ Dump JSON profiling data to file """
    with open('profiling/aetdata.json', 'w') as outfile:
        json.dump(profiling_data, outfile, sort_keys=False, indent=4, ensure_ascii=False)


"""Finds all profile data for kernels and returns list of kernel profile data

Args:
    lines:  raw profile dump. This stream has a separator ---End Kernel which divides core data

Returns:
    kernel_name_to_event:   dictionary of kernel name to event object
"""
def mark_events_and_cores(lines):
    kernel_name_to_event = {}
    """ Mark where events start and end """
    event_start = 0
    for index, line in enumerate(lines):
        if "---End Kernel" in line:
            """ mark event information """
            event1 = event_data()
            event1.start_index = event_start
            event1.last_index = index
            event1.name = lines[index - 1]
            event1.event_type = lines[index - 9]
            event1.event_number1 = lines[index - 8]
            event1.event_number2 = lines[index - 7]
            event1.stall_cycle_threshold = lines[index - 6]

            """ Get event data for core """
            event1.process_core_data(lines)
            event1.update_event_names()

            """ Add event info to dictionary """
            try:
                kernel_name_to_event[event1.name].append(event1)
            except KeyError:
                kernel_name_to_event[event1.name] = [event1]

            event_start = index + 1
    return kernel_name_to_event


"""Read all profiling data into lines and consume '\n'

Returns:
    lines: list of raw data dump
"""
def read_profiling_dump():
    file_path = sys.argv[len(sys.argv)-1]
    with open(file_path) as f:
        lines = f.readlines()
        for index, line in enumerate(lines):
            lines[index] = line.strip('\n')
    return lines


"""Forms list of kernels from dictionary of kernel name to events

Args:
    name_to_events: dictionary of kernel name to event list

Returns:
    kernel_list:    list of kernel objects
"""
def form_kernels(name_to_events):
    kernel_list = []
    for name,event_list in name_to_events.items():
        kernel1 = kernel_data()
        kernel1.name = name
        kernel1.event_list = event_list
        kernel_list.append(kernel1)
    return kernel_list


"""Analyzes kernel data according to user arguments. If the -t arg or -g arg is given,
then a table or plot of kernel data will be formed..

Args:
    kernel_list: list of kernel objects
"""
def analyze_kernel_data(kernel_list):
    user_wants_table = '-t' in sys.argv
    user_wants_graph = '-g' in sys.argv

    """ if user specified -t or -g params:
            Form a graph and or table for each kernel """
    if user_wants_table or user_wants_graph:
        for kernel in kernel_list:
            kernel.form_profiling_matrix()
            kernel.count_total_stall_cycles()
            kernel.det_if_profiling_all_events()
            if user_wants_table:
                table.form_table(kernel)
            if user_wants_graph:
                plot.form_graph(kernel)


"""Main Method"""
def main():
    print "Working..."
    cleanup_files()
    lines = read_profiling_dump()
    kernel_name_to_event = mark_events_and_cores(lines)
    kernel_list = form_kernels(kernel_name_to_event)
    print_json(kernel_list)
    analyze_kernel_data(kernel_list)
    print "Done! Check data in profiling/"


if __name__ == '__main__':
    main()

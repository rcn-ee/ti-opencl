#!/bin/bash

#/******************************************************************************
# * Copyright (c) 2017, Texas Instruments Incorporated - http://www.ti.com/
# *  All rights reserved.
# *
# *  Redistribution and use in source and binary forms, with or without
# *  modification, are permitted provided that the following conditions are met:
# *      * Redistributions of source code must retain the above copyright
# *        notice, this list of conditions and the following disclaimer.
# *      * Redistributions in binary form must reproduce the above copyright
# *        notice, this list of conditions and the following disclaimer in the
# *        documentation and/or other materials provided with the distribution.
# *      * Neither the name of Texas Instruments Incorporated nor the
# *        names of its contributors may be used to endorse or promote products
# *        derived from this software without specific prior written permission.
# *
# *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
# *  THE POSSIBILITY OF SUCH DAMAGE.
# *****************************************************************************/

AETDATA=profiling/aetdata.txt
PROFDIR=/usr/share/ti/opencl/profiling

function Usage_and_exit()
{
    echo "Usage: oclaet.sh [options] oclapp [event_type event_number*]"
    echo "oclaet.sh: in /usr/share/ti/opencl/profiling"
    echo "options:"
    echo "  -g: also ploting data, needs matplot python package,"
    echo "      see OpenCL user guide for details."
    echo "oclapp: the OpenCL application to be profiled for AET counters"
    echo "Optional arguments:"
    echo "  event_type: 1 for stall cycles, 2 for memory events"
    echo "  event_number*: one or more event numbers"
    echo "Defaults:"
    echo "  - If no event_type or event_numbers are sepcified,"
    echo "    profile both event types and all available event numbers."
    echo "  - output aet data file is in ${AETDATA}"
    echo "  - aet data will be processed into json file, html table,"
    echo "    and png plots (with -g) in profiling directory."

    exit -1;
}

if [ $# -lt 1 ]; then
    Usage_and_exit
fi

# pop the binary name off
PLOT=$1
shift

if [ $PLOT != "-g" ]; then
    BINARY=${PLOT}
    PLOT=" "
else
    BINARY=$1
    shift
fi

# remove old data file
echo "Removing ${AETDATA}, if exists"
rm -f ${AETDATA}

# record event_type and pop it off
EVENT_TYPE=$1
shift

# create arg array
args=("$@")


# if event type and event number params specified
if [ $# -gt 0 ]; then
    args=("$@")
    for event in "${args[@]}"
    do
	echo Profiling $event
        export TI_OCL_PROFILING_EVENT_TYPE=$EVENT_TYPE && export TI_OCL_PROFILING_EVENT_NUMBER1=$event
        $BINARY
    done

# otherwise profile all events
else
    i=0
    # run application for all memory events
    while [ $i -lt 30 ]; do
	echo Profiling $i $((i+1))
	export TI_OCL_PROFILING_EVENT_TYPE=2 && export TI_OCL_PROFILING_EVENT_NUMBER1=$i && export TI_OCL_PROFILING_EVENT_NUMBER2=$((i+1))
        $BINARY
        let i=i+2
    done

    # run application for all cpu events
    cpu_events=(0 1 2 3 4 5 6 8 10 11 12 13 14 15 16 17 18 20 21 22)
    for event in "${cpu_events[@]}"
    do
      echo Profiling $event
      export TI_OCL_PROFILING_EVENT_TYPE=1 && export TI_OCL_PROFILING_EVENT_NUMBER1=$event
      $BINARY
    done
fi;

python ${PROFDIR}/oclaet.py -t ${PLOT} ${AETDATA}


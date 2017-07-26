#!/bin/bash

# remove old data file
rm data/data.txt
echo removed data dump

# pop the binary name off 
binary=$1
shift

# record event_type and pop it off
event_type=${args[1]}
shift

# create arg array
args=("$@")


# if event type and event number params specified
if [ $# -gt 0 ]; then
    args=("$@")
    for event in "${args[@]}"
    do
	echo Profiling $event
        export TI_OCL_EVENT_TYPE=$event_type && export TI_OCL_EVENT_NUMBER1=$event && export TI_OCL_EVENT_NUMBER2=1 && export TI_OCL_STALL_CYCLE_THRESHOLD="100"
    	$binary
    done

# otherwise profile all events
else
    i=0
    # run application for all memory events
    while [ $i -lt 30 ]; do
	echo Profiling $i
	export TI_OCL_EVENT_TYPE="2" && export TI_OCL_EVENT_NUMBER1=$i && export TI_OCL_EVENT_NUMBER2=$((i+1)) && export TI_OCL_STALL_CYCLE_THRESHOLD="100"
    	$binary
        let i=i+2
    done

    # run application for all cpu events
    cpu_events=(0 1 2 3 4 5 6 8 10 11 12 13 14 15 16 17 18 20 21 22)
    for event in "${cpu_events[@]}"
    do
      echo Profiling $event
      export TI_OCL_EVENT_TYPE="0" && export TI_OCL_EVENT_NUMBER1=$event && export TI_OCL_STALL_CYCLE_THRESHOLD="6"
      $binary
    done
fi;

cd profiling/src && python profiling.py -t -g
cd ../.. 



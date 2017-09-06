********************************
Profiling
********************************

You can profile an OpenCL application as any other application with generic
profiling tools such as "gprof".  Here we explain how to profile commands
in the OpenCL command queue.

Additionally, you can profile hardware events such as L2 cache misses and
pipeline stalls through the AET library. See the Profiling Hardware Events
section for more information.

Host Side Profiling
=======================================================

To profile commands on the host side, you will need to specify
"CL_QUEUE_PROFILING_ENABLE" property when creating the command queue.
OpenCL runtime will then record the host-side timestamp in nano-seconds
when the command is enqueued, is submitted, starts execution and finishes
execution.  User code can query these timestamps using
"clGetEventProfilingInfo" API on the corresponding event that is returned
at command enqueue time.

DSP Side Profiling
=======================================================

OpenCL runtime also records timestamps for predefined OpenCL activities
on the DSP side and uses lightweight ULM (Usage and Load Monitor) to
communicate those data back to host.  These predefined activities include
start and finish of a workgroup execution, and dsp cache coherency operations.
On the host side, users will need to use dsptop utility to retrieve the
information.  You probably will need two windows/consoles and run your
OpenCL application after dsptop is run, for example,

#. In window 1, run ``dsptop -l last``
#. In window 2, launch your OpenCL application, wait for it to finish
#. Back in window 1, type "q" to quit dsptop.  dsptop should print out
   the information sent back from the DSP side.

Details about usage of dsptop can be found by running ``dsptop -h`` and by
this `dsptop wikipage`_ .

.. _dsptop wikipage: http://processors.wiki.ti.com/index.php/Dsptop

Profiling Hardware Events
=======================================================

Profiling hardware events is a useful capability provided through the CTools
AET library.  Starting from TI OpenCL product v1.1.14, this capability is
integrated into the OpenCL runtime.  See all the stall and memory events
in the AET Profiling Events section below.  Events not prefixed
with ``AET_EVT_STALL_`` or ``AET_EVT_MEM_`` cannot be profiled.
To profile hardware events, there are three choices:

#. Profile All Possible Events
#. Profile a Select Few Events
#. Manually Profile 1 or 2 Events

Profile All Events
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To profile all hardware events, run the profiling script as follows:

.. code-block:: bash

    /usr/share/ti/opencl/profiling/oclaet.sh [-g] oclapp
    # e.g oclapp is ./vecadd

While the the executable is running, raw profiling data is recorded into
profiling/aetdata.txt relative to the current directory.
After the profiling script finishes, it runs a python
script that forms a json table, a html table, and optionally a plot of each
kernel's profiling information if ``-g`` option is specified.  Note that
genering plots may require installing additional python packages.

Profile Select Events
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To profile selected hardware events, run the profiling script as follows:

.. code-block:: bash

    /usr/share/ti/opencl/profiling/oclaet.sh oclapp event_type event_number [event_number ...]
    # event_type: 1 for stall events, 2 for meeory events
    # event_number is the event offset from AET_GEM_STALL_EVT_START, if
    #   profiling stall cycles, or from AET_GEM_MEM_EVT_START, if profiling
    #   memory events.  At each script run, you can only profile one or more
    #   events of the same type.


Manually Profile 1 or 2 Events
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To manually profile 1 stall event, or 1 to 2 memory events, simply set the
environment variables described in :doc:`environment_variables`, as follows.

.. code-block:: bash

    TI_OCL_EVENT_TYPE=1 TI_OCL_EVENT_NUMBER1=13 TI_OCL_STALL_CYCLE_THRESHOLD=0 ./vecadd
    TI_OCL_EVENT_TYPE=2 TI_OCL_EVENT_NUMBER1=11 TI_OCL_EVENT_NUMBER1=12 ./vecadd

Note that profiling data is appended to profiling/aetdata.txt at each
profiling run.  If a fresh profiling is needed, remove profiling/aetdata.txt
before profiling run.

Analyzing Profiling Data
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you manually profile, you will have to run the python script to
obtain the JSON file or html table.

.. code-block:: bash

    python /usr/share/ti/opencl/profiling/oclaet.py -t -g profiling/aetdata.txt

The -t flag and -g flags tell the script to produce an html table and matplot
plot of profiling data, respectively.  If neither of these flags are specified,
then only the json file of raw counter data will be formed.
This json is easier to read than the raw data dump in profiling/aetdata.txt.
The current format for raw data is:

.. code-block:: bash

  EVENT_TYPE            # the event type
  EVENT_NUMBER1         # the first event number (offset from base AET event)
  EVENT_NUMBER2         # the second event number (offset from base AET event)
  STALL_CYCLE_THRESHOLD # the stall cycle threshold
  Core number           # number of core
  Counter0_Value        # hardware counter 0 value: memory event 1
  Counter1_Value       # hardware counter 1 value: memory event 2 or stall event
  ~~~~End Core          # End of core data for Core number
  ...                   # MORE CORE DATA CAN FOLLOW THIS
  EVENT_TYPE
  EVENT_NUMBER1
  EVENT_NUMBER2
  STALL_CYCLE_THRESHOLD
  Core number
  Counter0_Value
  Counter1_Value
  ~~~~End Core
  VectorAdd             # Kernel Name
  ---End Kernel         # Ends Kernel Data

Profiling Data Plotting Requirements
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Plotting profiling data with python script requires matplotlib, pandas, and
seaborn python packages to be installed.  If they are not already installed
on your system, you can follow the instructions below.

.. code-block:: bash

    which pip
    # install pip if it is not already on your system
    wget https://bootstrap.pypa.io/get-pip.py
    python get-pip.py

    pip install matplotlib
    pip install pandas
    pip install seaborn

AET Profiling Events
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AET_EVT_MEM_L1D_RH_SRAM_A   (AET_GEM_MEM_EVT_START + 0)
 L1D Read Hit SRAM A

AET_EVT_MEM_L1D_RH_SRAM_B   (AET_GEM_MEM_EVT_START + 1)
 L1D Read Hit SRAM B

AET_EVT_MEM_L1D_RH_CACHE_A   (AET_GEM_MEM_EVT_START + 2)
 L1D Read Hit Cache A

AET_EVT_MEM_L1D_RH_CACHE_B   (AET_GEM_MEM_EVT_START + 3)
 L1D Read Hit Cache B

AET_EVT_MEM_L1D_WH_BUF_NOT_FULL_A   (AET_GEM_MEM_EVT_START + 4)
 L1D Write Hit, Tag Buffer Not Full A

AET_EVT_MEM_L1D_WH_BUF_NOT_FULL_B   (AET_GEM_MEM_EVT_START + 5)
 L1D Write Hit, Tag Buffer Not Full B

AET_EVT_MEM_L1D_WH_BUF_FULL_A   (AET_GEM_MEM_EVT_START + 6)
 L1D Write Hit, Tag Buffer Full A

AET_EVT_MEM_L1D_WH_BUF_FULL_B   (AET_GEM_MEM_EVT_START + 7)
 L1D Write Hit, Tag Buffer Full B

AET_EVT_MEM_L1D_RM_HITS_L2_SRAM_A   (AET_GEM_MEM_EVT_START + 8)
 L1D Read Miss, Hits L2 SRAM A

AET_EVT_MEM_L1D_RM_HITS_L2_SRAM_B   (AET_GEM_MEM_EVT_START + 9)
 L1D Read Miss, Hits L2 SRAM B

AET_EVT_MEM_L1D_RM_HITS_L2_CACHE_A   (AET_GEM_MEM_EVT_START + 10)
 L1D Read Miss, Hits L2 Cache A

AET_EVT_MEM_L1D_RM_HITS_L2_CACHE_B   (AET_GEM_MEM_EVT_START + 11)
 L1D Read Miss, Hits L2 Cache B

AET_EVT_MEM_L1D_RM_HITS_EXT_CABLE_A   (AET_GEM_MEM_EVT_START + 12)
 L1D Read Miss, Hits External, Cacheable A

AET_EVT_MEM_L1D_RM_HITS_EXT_CABLE_B   (AET_GEM_MEM_EVT_START + 13)
 L1D Read Miss, Hits External, Cacheable B

AET_EVT_MEM_L1D_RM_HITS_EXT_NON_CABLE_A   (AET_GEM_MEM_EVT_START + 14)
 L1D Read Miss, Hits External, Non Cacheable A

AET_EVT_MEM_L1D_RM_HITS_EXT_NON_CABLE_B   (AET_GEM_MEM_EVT_START + 15)
 L1D Read Miss, Hits External, Non Cacheable B

AET_EVT_MEM_L1D_WM_WRT_BUF_NOT_FULL_A   (AET_GEM_MEM_EVT_START + 16)
 L1D Write Miss, Write Buffer Not Full A

AET_EVT_MEM_L1D_WM_WRT_BUF_NOT_FULL_B   (AET_GEM_MEM_EVT_START + 17)
 L1D Write Miss, Write Buffer Not Full B

AET_EVT_MEM_L1D_WM_WRT_BUF_FULL_A   (AET_GEM_MEM_EVT_START + 18)
 L1D Write Miss, Write Buffer Full A

AET_EVT_MEM_L1D_WM_WRT_BUF_FULL_B   (AET_GEM_MEM_EVT_START + 19)
 L1D Write Miss, Write Buffer Full B

AET_EVT_MEM_L1D_WM_TAG_VIC_WRT_BUF_FLUSH_A   (AET_GEM_MEM_EVT_START + 20)
 L1D Write Miss, Tag/Victim/Write Buffer Flush A

AET_EVT_MEM_L1D_WM_TAG_VIC_WRT_BUF_FLUSH_B   (AET_GEM_MEM_EVT_START + 21)
 L1D Write Miss, Tag/Victim/Write Buffer Flush B

AET_EVT_MEM_CPU_CPU_BANK_CONFLICT   (AET_GEM_MEM_EVT_START + 22)
 CPU - CPU Bank Conflict

AET_EVT_MEM_CPU_SNOOP_CONFLICT   (AET_GEM_MEM_EVT_START + 23)
 CPU - Snoop/Coherence Conflict (A or B)

AET_EVT_MEM_CPU_IDMA_EDMA_BANK_CONFLICT   (AET_GEM_MEM_EVT_START + 24)
 CPU - iDMA/EDMA Bank Conflict (A or B)

AET_EVT_MEM_L1P_RH_SRAM   (AET_GEM_MEM_EVT_START + 25)
 L1P Read Hit SRAM

AET_EVT_MEM_L1P_RH_CACHE   (AET_GEM_MEM_EVT_START + 26)
 L1P Read Hit Cache

AET_EVT_MEM_L1P_RM_HITS_L2_SRAM   (AET_GEM_MEM_EVT_START + 27)
 L1P Read Miss, Hits L2 SRAM

AET_EVT_MEM_L1P_RM_HITS_L2_CACHE   (AET_GEM_MEM_EVT_START + 28)
 L1P Read Miss, Hits L2 Cache

AET_EVT_MEM_L1P_RM_HITS_EXT_CABLE   (AET_GEM_MEM_EVT_START + 29)
 L1P Read Miss, Hits External Cacheable

AET_EVT_STALL_CPU_PIPELINE   (AET_GEM_STALL_EVT_START + 0)
 CPU Stall Cycles

AET_EVT_STALL_CROSS_PATH   (AET_GEM_STALL_EVT_START + 1)
 Stall Due to a Cross path

AET_EVT_STALL_BRANCH_TO_SPAN_EXEC_PKT   (AET_GEM_STALL_EVT_START + 2)
 Stall due to a branch to an execute packet that spans two fetch packets

AET_EVT_STALL_EXT_FUNC_IFACE   (AET_GEM_STALL_EVT_START + 3)
 Stall due to an External Functional Interface

AET_EVT_STALL_MVC   (AET_GEM_STALL_EVT_START + 4)
 Stall Conditions: 1) AMR write followed by addressing mode instruction and src2 register is affected by AMR Values 2) Read of emulation registers in the ECM

AET_EVT_STALL_L1P_OTHER   (AET_GEM_STALL_EVT_START + 5)
 Any other stall not prwviously listed

AET_EVT_STALL_L1P_WAIT_STATE   (AET_GEM_STALL_EVT_START + 6)
 Stall due to Wait states in L1P Memory

AET_EVT_STALL_L1P_MISS   (AET_GEM_STALL_EVT_START + 8)
 Execute Packed held off due to a Cache Miss

AET_EVT_STALL_L1D_OTHER   (AET_GEM_STALL_EVT_START + 10)
 Any other L1D Stall not previosuly listed

AET_EVT_STALL_L1D_BANK_CONFLICT   (AET_GEM_STALL_EVT_START + 11)
 Stall on a memory bank conflict between A and B

AET_EVT_STALL_L1D_DMA_CONFLICT   (AET_GEM_STALL_EVT_START + 12)
 Stall while CPU access is held off by a DMA Access

AET_EVT_STALL_L1D_WRITE_BUFFER_FULL   (AET_GEM_STALL_EVT_START + 13)
 Stall on a write miss on A or B while the Write Buffer is Full

AET_EVT_STALL_L1D_TAG_UD_BUF_FULL   (AET_GEM_STALL_EVT_START + 14)
 Stall on a Write Hit with a tag update on either A or B while the tag update buffer is full

AET_EVT_STALL_L1D_LINE_FILL_B   (AET_GEM_STALL_EVT_START + 15)
 Stall on a read miss on B while the read miss data is being fetched from the lower memory level

AET_EVT_STALL_L1D_LINE_FILL_A   (AET_GEM_STALL_EVT_START + 16)
 Stall on a read miss on A while the read miss data is being fetched from the lower memory level

AET_EVT_STALL_L1D_WRT_BUF_FLUSH   (AET_GEM_STALL_EVT_START + 17)
 Stall on a read Miss on Either A or B while the Write Buffer is being flushed

AET_EVT_STALL_L1D_VICTIM_BUF_FLUSH   (AET_GEM_STALL_EVT_START + 18)
 Stall on a read miss on either A or B while the Victim Buffer is being flushed

AET_EVT_STALL_L1D_TAG_UD_BUF_FLUSH   (AET_GEM_STALL_EVT_START + 20)
 Stall on a read miss on either A or B while the Tag Update Buffer is being flushed

AET_EVT_STALL_L1D_SNOOP_CONFLICT   (AET_GEM_STALL_EVT_START + 21)
 Stall while a CPU access is held off by a Snoop access

AET_EVT_STALL_L1D_COH_OP_CONFLICT   (AET_GEM_STALL_EVT_START + 22)
 Stall while a CPU access is held off by a block cache coherence operation access


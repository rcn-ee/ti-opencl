****************************
Debug with CCS
****************************

You can also debug your OpenCL DSP side code using the debug capability in
Code Composer Studio (CCS).  To do that, you will need an additional
hardware, an emulator such as `XDS560v2`_, to connect to the JTAG port of
your EVM.

This feature is available only if your TI OpenCL installation is version
01.01.06.00 or newer.

.. _XDS560v2: http://processors.wiki.ti.com/index.php/XDS560v2_System_Trace

Connect emulator to EVM and CCS
===========================================
First, connect emulator to the EVM via the JTAG port.  Next, depends on
whether you want CCS to communicate with emulator via Ethernet or USB, you
either connect an Ethernet cable to the emulator or connect emulator to the
host machine where you run CCS with a USB cable.  Power the emulator up.

Next, launch CCS, "View -> Target Configurations", create a "User Defined"
"New Target Configuration".  For "Connection", choose your emulator model
from the list, e.g. "Spectrum Digital XDS560V2 STM USB Emulator".
For "Board or Device", choose your EVM model from the list, e.g. "66AK2H",
"TMS320C6678".  For AM57xx, you will need to install DRA7xx Chip Support
Packet (CSP) as an update into CCS.  However, DRA7xx CSP requires NDA
agreement with TI as of this writing.  If it does not come by default with
latest CCS that you can download, please contact your Field Application
Engineer (FAE) for support.  Once DRA7xx CSP is installed in CCS, please
choose "DRA75x_DRA74x" as "Board or Device".  Once "Connection" and "Board
or Device" are chosen, please save configuration and test connection to
ensure that CCS can talk to the EVM via the emulator.

If you choose to have CCS talk to the emulator via Ethernet, you can use
configuration utility that comes with CCS installation to find the IP address
of your emulator, for example, launching "XDS560v2 STM Configuration Utility"
and then click "Find Ethernet Devices" under "Eth" tab.  Or you can google
how to find emulator's IP address using its MAC address.  Once the IP address
is known, click "Advanced" tab, click "Emulator", enter
"The Emulator IP Address", save and test connection.

Finally, right click the target configuration that you just made and
"Launch Selected Configuration".  Once launched, you should see DSP core 0
("C66xx_DSP1") in the list, connect to DSP core 0 and resume running.

Debug DSP side code with CCS
===========================================
Debugging DSP side code with CCS takes similar steps as debugging with gdbc6x.

#. Set environment variable ``TI_OCL_DEBUG`` to "ccs" before running
   application, for example, ``TI_OCL_DEBUG=ccs ./your_ocl_app``
   if you use bash.
#. Once the application is running, before launching your kernel to DSP,
   OpenCL runtime will print out a list of CCS instructions that you should
   perform before continuing, for example,

   * CCS Suspend dsp core 0
   * CCS Load symbols: /tmp/openclwsNYLl.out, code offset: 0x86000000
   * CCS Add symbols: /usr/share/ti/opencl/dsp.out, no code offset
   * CCS Add breakpoint: VectorAdd
   * CCS Resume dsp core 0
   * Press any key, then enter to continue

   You may need to copy the kernel executable and dsp.out to your host
   filesystem where you run CCS, perhaps kernel source code as well so that
   CCS can display it when you debug.
#. Once the CCS symbols have been loaded/added and CCS breakpoints have
   been set, CCS should stop DSP core 0 at your kernel function entry
   when you press a key to continue on the host side.  From this point on,
   you can step through the code, check memory contents, check variables
   values, just as you normally do with CCS debug.

.. Note::
    After debugging with CCS, you may not be able to debug with gdbc6x anymore
    unless you power cycle your EVM.


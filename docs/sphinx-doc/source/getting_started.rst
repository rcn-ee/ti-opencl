******************************************
OpenCL Getting Started
******************************************

Installation
======================================================

66AK2H 
-------------------------------------------------------

OpenCL is installed as a component in the MCSDK-HPC product from TI.

- For installation of MCSDK-HPC on the 66AK2H EVM, please refer to the
  following instructions:
  http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide.

- For installation of MCSDK-HPC on an HP m800 Moonshot cartridge please refer to
  the following instructions:
  http://processors.wiki.ti.com/index.php/MCSDK_HPC_3.x_Getting_Started_Guide_for_HP_ProLiant_m800

A mapping of MCSDK-HPC versions to the versions of its components, including
OpenCL, can be found `here <MCSDK HPC to component version map>`__.

AM57 
-------------------------------------------------------
OpenCL is installed as a component in the Processor SDK product from TI.

Limitations 
======================================================

- This OpenCL implementation is not designed to allow multiple OpenCL enabled
  Linux processes to concurrently execute. If an attempt is made to execute
  concurrent OpenCL processes, the first process will start normally, all other
  processes will block until completion of the first process.

- Support for images and samplers is optional for non GPU devices per the
  OpenCL 1.1 spec and they are not supported by the DSP device in this
  implementation.

- This OpenCL implementation is not ICD enabled and cannot co-exist with other OpenCL 
  implementations.

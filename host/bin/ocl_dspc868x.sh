#!/bin/sh

export TI_OCL_INSTALL=${HOME}/opencl-dspc8681-0.15.0
export TI_OCL_CGT_INSTALL=/opt/ti/c6000_8.0.0B4

export C6X_C_DIR="$TI_OCL_CGT_INSTALL/include;$TI_OCL_CGT_INSTALL/lib"

if [ -n $LD_LIBRARY_PATH ]
then
  export LD_LIBRARY_PATH="$TI_OCL_INSTALL/lib:$LD_LIBRARY_PATH"
else
  export LD_LIBRARY_PATH="$TI_OCL_INSTALL/lib:/usr/lib"
fi

export PATH="$TI_OCL_INSTALL/bin:$TI_OCL_CGT_INSTALL/bin:$PATH"

### Aliases


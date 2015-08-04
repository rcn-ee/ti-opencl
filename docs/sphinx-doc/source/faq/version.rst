**************************************
Which TI OpenCL Version is Installed?
**************************************

1. Executing the command ``clocl --version`` will display the version of the OpenCL compiler 
   installed.

2. Executing the command ``ls -l /usr/lib/libOpenCL*`` will display the OpenCL libraries 
   installed on the device. Follow the soft links from ``/usr/lib/libOpenCL.so`` to a fully 
   version qualified libOpenCL library like ``/usr/lib/libOpenCL.so.1.1.0``. The version of 
   the library will indicate which version of the OpenCL package is installed.

3. If the device is running Ubuntu and ti-opencl was installed using dpkg or apt-get, 
   then the command ``dpkg -s ti-opencl`` will display the currently installed version.

4. From OpenCL C source, the predefined macro ``__TI_OCL_VERSION`` will contain an encoded 
   representation of the OpenCL product version. This macro can be used with the compiler's 
   preprocessor to conditionally compile regions based on the product version of OpenCL. 
   The 4 components of the product version ``xx.yy.zz.ww`` are encoded in a 32-bit value in 
   the pattern ``0xXXYYZZWWu``. For example, product version 01.01.02.12 would be returned as 
   ``0x01010212u`` when ``__TI_OCL_VERSION`` is referenced.

5. The product version can be queried programmatically in an application by using the OpenCL 
   API's to query the platform version. The returned string will have a format similar to: 
   ``OpenCL 1.1 TI product version 1.1.1.0``. Sample c++ code to query the version follows::

    #include <CL/cl.hpp>
    #include <iostream>
     
    std::vector<cl::Platform> platforms;
    std::string               str;
      
    cl::Platform::get(&platforms);
    platforms[0].getInfo(CL_PLATFORM_VERSION, &str);
    std::cout <<  str << std::endl;

****************************
Debug with dsptop
****************************

**dsptop**, a TI utility that is similar to linux utility **top**, can be used
to debug which dsp cores participate in computation, memory usage of OpenCL
buffers, and kernel activities with timestamps such as workgroup start,
workgroup complete, and cache operations.  

Like gdbc6x, use of dsptop requires two windows/consoles as well:
start dsptop in window 1 first, launch your OpenCL application in window 2
next.  Details about the usage of dsptop can be found by running
``dsptop -h`` and by this `dsptop wikipage`_ .

.. _dsptop wikipage: http://processors.wiki.ti.com/index.php/Dsptop


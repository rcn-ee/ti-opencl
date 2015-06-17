******************************************************
Cache Coherency
******************************************************
The A15 CPUs are cache coherent with each other, but they are not cache
coherent with the C66 DSPs.  The C66 DSPs are not cache coherent with the A15s
or with other C66 DSPs. The OpenCL runtime will manage coherency of the various
device caches through software cache coherency calls.

TBD


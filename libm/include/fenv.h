#if defined (_TMS320C6X)
#include "../c66/fenv.h"
#elif defined( __LP64)
#include "../amd64/fenv.h"
#else
#include "../i387/fenv.h"
#endif

#ifndef _TRACE_H
#define _TRACE_H

#if defined(ULM_ENABLED)
#define TRACE(state, kid, wgid) ulm_put_statemsg(state, kid, wgid)
#else
#define TRACE(code, kid, wgid) 
#endif

#endif // _TRACE_H


void ccode(uint32_t timeout_ms, global uint32_t *completion_code); 

kernel void wrapper(uint32_t timeout_ms, global uint32_t *completion_code) 
{ 
    ccode(timeout_ms, completion_code); 
}

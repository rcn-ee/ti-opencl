void ccode(global uint32_t *completion_code); 

kernel void wrapper(global uint32_t *completion_code) 
{ 
    ccode(completion_code); 
}

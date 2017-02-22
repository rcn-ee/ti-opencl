void start_clock(global uint32_t *completion_code);

kernel void wrapper(global uint32_t *completion_code) 
{ 
    start_clock(completion_code); 
}

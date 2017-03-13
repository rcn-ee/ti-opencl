void start_clock(global uint32_t *completion_code);
void stop_clock (global uint32_t *completion_code);

kernel void Kstart_clock(global uint32_t *completion_code)
{ 
    start_clock(completion_code); 
}

kernel void Kstop_clock(global uint32_t *completion_code)  
{ 
    stop_clock(completion_code);  
}

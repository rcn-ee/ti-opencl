void start_task(global uint32_t *completion_code);
void stop_task (global uint32_t *completion_code);

kernel void Kstart_dsp(global uint32_t *completion_code) 
{
    start_task(completion_code);
}

kernel void Kstop_dsp (global uint32_t *completion_code) 
{
    stop_task (completion_code);
}

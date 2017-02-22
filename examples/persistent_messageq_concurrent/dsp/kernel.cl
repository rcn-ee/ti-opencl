void ccode(uint32_t qid, global uint32_t *completion_code);

kernel void wrapper(uint32_t qid, global uint32_t *completion_code) 
{ 
    ccode(qid, completion_code); 
}

#include "driver.h"
#include "mailbox.h"
#include "message.h"
#include "stdio.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

using namespace std;

#define SIZE 128*1024*1024
#define ROUNDS 2048

char data[SIZE];

static int64_t clock_diff (struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000000000 + end->tv_nsec - start->tv_nsec;
}

Mailbox* Mailbox::pInstance = 0;

int main()
{
    struct   timespec tp_start, tp_end;
    int64_t  elapsed;
    int      dsp_core = 0;
    uint32_t trans_id = 0xfaceface;
    uint32_t size;

    Driver::instance(); // to prime the lazy execution;

    mailBox_init(MAILBOX_NODE_ID_HOST, dsp_core);
    mailBox_init(dsp_core, MAILBOX_NODE_ID_HOST);
    int tx_mbox = mailBox_create(MAILBOX_NODE_ID_HOST, dsp_core,0,0,0);
    int rx_mbox = mailBox_create(dsp_core, MAILBOX_NODE_ID_HOST,0,0,0);

    Msg_t msg;
    msg.code = READY;

    clock_gettime(CLOCK_MONOTONIC, &tp_start);
    clock_gettime(CLOCK_MONOTONIC, &tp_end);
    elapsed = clock_diff (&tp_start, &tp_end);

    int64_t tot_time = 0;
    for (int i = 0; i < 16; ++i)
    {
        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        elapsed = clock_diff (&tp_start, &tp_end);
        tot_time += elapsed;
    }
    tot_time >>= 4;
    printf ("timer granularity = %ld ns\n\n", tot_time);

    vector<int64_t> times(ROUNDS);
    for (int i = 0; i < ROUNDS; ++i)
    {
        clock_gettime(CLOCK_MONOTONIC, &tp_start);

        mailBox_write(tx_mbox, (uint8_t*)&msg, sizeof(Msg_t), trans_id++);
        mailBox_read (rx_mbox, (uint8_t*)&msg, &size, &trans_id);

        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        times[i] = clock_diff (&tp_start, &tp_end);

    }

    int64_t mean = accumulate( times.begin(), times.end(), 0 ) / times.size();
    printf ("avg mailbox roundtrip = %ld ns\n", mean);

    int64_t max = *max_element( times.begin(), times.end() );
    printf ("max mailbox roundtrip = %ld ns\n", max);

    int64_t min = *min_element( times.begin(), times.end() );
    printf ("min mailbox roundtrip = %ld ns\n", min);
}

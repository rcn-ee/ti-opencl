#include "driver.h"
#include "stdio.h"

#define SIZE 128*1024*1024

char data[SIZE];

static int64_t clock_diff (struct timespec *start, struct timespec *end)
{
    return (end->tv_sec - start->tv_sec) * 1000000000 + end->tv_nsec - start->tv_nsec;
}


int main()
{
    struct timespec tp_start, tp_end;
    int64_t write_time;
    int64_t dwrite_time;
    int64_t read_time;
    int64_t dread_time;

    Driver::instance(); // to prime the lazy execution;

    for (int i = 1; i <= 16; i++)
    {
        int size = i * 1024;

        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        Driver::instance()->write(0,0x80000000, (uint8_t*)data, size);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        write_time = clock_diff (&tp_start, &tp_end);

        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        Driver::instance()->dma_write(0,0x80000000, (uint8_t*)data, size);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        dwrite_time = clock_diff (&tp_start, &tp_end);

        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        Driver::instance()->read     (0,0x80000000, (uint8_t*)data, size);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        read_time = clock_diff (&tp_start, &tp_end);

        clock_gettime(CLOCK_MONOTONIC, &tp_start);
        Driver::instance()->dma_read (0,0x80000000, (uint8_t*)data, size);
        clock_gettime(CLOCK_MONOTONIC, &tp_end);
        dread_time = clock_diff (&tp_start, &tp_end);

        printf ("%3d Kb DMA speedup(r, w)  = %5.2f, %5.2f x\n", 
                size / 1024, 
                (double)read_time/(double)dread_time,
                (double)write_time/(double)dwrite_time);

        printf ("%3d Kb read (rd cpy, rd dma, wr cpy, wr dma) us = %lld, %lld %lld, %lld\n\n", 
                size / 1024, 
                (long long)read_time/1000, (long long)dread_time/1000,
                (long long)write_time/1000, (long long)dwrite_time/1000);
    }
}

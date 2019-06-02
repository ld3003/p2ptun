#include "p2ptun_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

//read操作加上超时时间。
int p2ptun_read_timeout(int fd, void *buf, unsigned int count, int time)
{
    return 0;
}

void p2ptun_get_current_time(struct P2PTUN_TIME *time)
{
    struct timeval tv;
    struct timezone tz;

    gettimeofday(&tv, &tz);

    time->sec = tv.tv_sec;
    time->usec = tv.tv_usec;
}
int t1_sub_t2_sec(struct P2PTUN_TIME *t1, struct P2PTUN_TIME *t2)
{
    //printf("t1 %d t2%d\n",t1->sec , t2->sec);
    if (t1->sec > t2->sec)
        return t1->sec - t2->sec;
    return 0;
}
int t1_sub_t2_ms(struct P2PTUN_TIME *t1, struct P2PTUN_TIME *t2)
{
    if (t1->sec >= t2->sec)
    {
        //printf("@ %d %d %d %d \n",t1->sec,t1->usec,t2->sec,t2->usec);
        return ((t1->sec - t2->sec) * (1000 * 1000) + t1->usec - t2->usec) / 1000;
    }
    return -1;
}

int get_sub_tim_sec(struct P2PTUN_TIME *t2)
{
    struct P2PTUN_TIME t1;
    p2ptun_get_current_time(&t1);
    return t1_sub_t2_sec(&t1, t2);
}

int get_sub_tim_ms(struct P2PTUN_TIME *t2)
{
    struct P2PTUN_TIME t1;
    p2ptun_get_current_time(&t1);
    return t1_sub_t2_ms(&t1, t2);
}

void itimeofday(long *sec, long *usec)
{
    struct timeval time;
    gettimeofday(&time, NULL);
    if (sec)
        *sec = time.tv_sec;
    if (usec)
        *usec = time.tv_usec;
}

long long iclock64(void)
{
    long s, u;
    long long value;
    itimeofday(&s, &u);
    value = ((long long)s) * 1000 + (u / 1000);
    return value;
}

long iclock()
{
    return (long)(iclock64() & 0xfffffffful);
}
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
int t1_sub_t2_sec(struct P2PTUN_TIME *t1 , struct P2PTUN_TIME *t2)
{
    //printf("t1 %d t2%d\n",t1->sec , t2->sec);
    if (t1->sec > t2->sec)
        return t1->sec - t2->sec;
    return 0;
}
int t1_sub_t2_ms(struct P2PTUN_TIME *t1 , struct P2PTUN_TIME *t2)
{
    return 0;
}

int get_sub_tim_sec(struct P2PTUN_TIME *t2)
{
    struct P2PTUN_TIME t1;
    p2ptun_get_current_time(&t1);
    return t1_sub_t2_sec(&t1,t2);
}

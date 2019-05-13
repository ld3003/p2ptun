#ifndef __p2ptun_common_h__
#define __p2ptun_common_h__

#define P2P_SESSION_TIMEOUT 20
#define P2PTUN_PING_TIME   2

typedef char p2ptun_int8;
typedef unsigned char p2ptun_uint8;

typedef short p2ptun_int16;
typedef unsigned short p2ptun_uint16;

typedef int p2ptun_int32;
typedef unsigned int p2ptun_uint32;

struct P2PTUN_TIME
{

    p2ptun_uint32 sec;  /* Seconds.  */
    p2ptun_uint32 usec; /* Microseconds.  */
};

int p2ptun_read_timeout(int fd, void *buf, unsigned int count, int time);
void p2ptun_get_current_time(struct P2PTUN_TIME *time);
int t1_sub_t2_sec(struct P2PTUN_TIME *t1 , struct P2PTUN_TIME *t2);
int t1_sub_t2_ms(struct P2PTUN_TIME *t1 , struct P2PTUN_TIME *t2);
int get_sub_tim_sec(struct P2PTUN_TIME *t2);

#endif
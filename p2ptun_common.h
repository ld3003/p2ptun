#ifndef __p2ptun_common_h__
#define __p2ptun_common_h__


int read_timeout(int fd, void *buf, unsigned int count, int time);
unsigned int get_sys_time();

#endif
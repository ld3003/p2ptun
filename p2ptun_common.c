#include "p2ptun_common.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>

//read操作加上超时时间。
int read_timeout(int fd, void *buf, unsigned int count, int time)
{
    if (time > 0)
    {
        fd_set rSet;
        FD_ZERO(&rSet);
        FD_SET(fd, &rSet);

        struct timeval timeout;
        memset(&timeout, 0, sizeof(timeout));
        timeout.tv_sec = time;
        timeout.tv_usec = 0;

        int ret;
        while (1)
        {
            ret = select(fd + 1, &rSet, NULL, NULL, &timeout);
            if (ret < 0)
            {
                if (errno == EINTR)
                    continue;
                //ERR_EXIT("select");
            }
            else if (ret == 0)
            {
                errno = ETIMEDOUT;
                return -1;
            }
            else
            {
                break;
            }
        }
    }
    int readNum;
    readNum = read(fd, buf, count);
    return readNum;
}
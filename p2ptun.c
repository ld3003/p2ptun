#include "p2ptun.h"
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

struct P2PTUN_CONN_SESSION *p2ptun_alloc_session()
{
    int ret;
    struct P2PTUN_CONN_SESSION *session;
    session = malloc(sizeof(struct P2PTUN_CONN_SESSION));
    if (session <= 0)
        return -P2PTUN_MEMERR;
    memset(session, 0x0, sizeof(struct P2PTUN_CONN_SESSION));
    session->currentstatus = P2PTUN_STATUS_INIT;

    //ret = pipe(session->msgpipe);

    if (ret <= 0)
        return ret;

    return 0;
}

int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session)
{
    return 0;
}

int p2ptun_listen(struct P2PTUN_CONN_SESSION *session)
{

    if (session->currentstatus != P2PTUN_STATUS_INIT)
        return -P2PTUN_STATUSERR;
    session->currentstatus = P2PTUN_STATUS_LISTEN;

    return 0;
}

int p2ptun_connect(struct P2PTUN_CONN_SESSION *session, char *peername)
{

    /*
        session->socketfd = socket
        connect 

        pub -> 
        wait mqtt pipe


        在信令系统查询对方是否在线
        如果不在线则放弃

        获取网络类型
            发送两个 udp
            看两个返回的UDP端口是否一致
        通过 信令 获取对方网络类型

        1，双方都是锥形NAT
            向对方的 公网IP:PORT 发送打洞包
            通过信令 通知对方 向本机的公网IP:PORT发送 打洞包

            若收到对方打洞包
                向对方发送PING包
                若收到对方的PONG包后此时才认为连接成功

        2，若本机是对称型，对方那公是锥形
            向对方的 公网IP:PORT 发送打洞包
            通过信令 通知对方 向本机的公网IP:PORT发送 打洞包



    */

    return 0;
}
int p2ptun_disconnect(struct P2PTUN_CONN_SESSION *session)
{
    return 0;
}

int p2ptun_send_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int len)
{
    return 0;
}
int p2ptun_set_recvdata_cb(struct P2PTUN_CONN_SESSION *session, void *cb)
{
    return 0;
}

int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg)
{
}
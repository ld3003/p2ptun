#ifndef __p2ptun_common_h__
#define __p2ptun_common_h__

#define P2P_SESSION_TIMEOUT 20
#define P2PTUN_SENDHB_TIME 2
#define P2PTUN_PKGTIMEOUT 5

typedef char p2ptun_int8;
typedef unsigned char p2ptun_uint8;

typedef short p2ptun_int16;
typedef unsigned short p2ptun_uint16;

typedef int p2ptun_int32;
typedef unsigned int p2ptun_uint32;

#define P2PTUN_CMD_UDPPING 100 /*< 发送到服务器的ping包 ，用于获取本机AP和网络类型*/
#define P2PTUN_CMD_UDPPONG 101 /*< ECHO服务器返回的 PONG包 ，包含公网IP和端口 */

#define P2PTUN_CMD_MSGPING 102  /*< 测试对方是否在线的PING包，服务器转发 */
#define P2PTUN_CMD_MSGPONG 8102 /*< 测试对方是否在线的PONG包，服务器转发 */

#define P2PTUN_CMD_MSGGETNTYPE 103    /*< 获取对方网络类型的消息 */
#define P2PTUN_CMD_MSGRRESPNTYPE 8103 /*< 返回网络类型的消息 */

#define P2PTUN_CMD_UDP_TEST 104      /*< UDP打洞消息 */
#define P2PTUN_CMD_UDP_RESPTEST 8104 /*< UDP打洞ECHO消息 */

#define P2PTUN_CMD_MSG_REQUESTUDPTEST 105   /*< 请求反向打洞的消息 */
#define P2PTUN_CMD_UDP_RESPONSEUDPTEST 8105 /*< 请求反向打洞的返回消息 */

#define P2PTUN_CMD_MSG_CONNECTED 106      /*< 确定打洞成功的命令 */
#define P2PTUN_CMD_MSG_RESPCONNECTED 8106 /*< 确定打洞成功的命令 */

#define P2PTUN_CMD_UDP_HB 107      /*< 隧道心跳消息 */
#define P2PTUN_CMD_UDP_ECHOHB 8107 /*< 隧道心跳消息 */

#define P2PTUN_CMD_MSG_DISCONNECT 108      /*< 隧道心跳消息 */

#define P2PTUN_UDPPKG_TYPE_PING1 0
#define P2PTUN_UDPPKG_TYPE_PING2 1
#define P2PTUN_UDPPKG_TYPE_P2PMSG 2
#define P2PTUN_UDPPKG_TYPE_RELAYMSG 3
#define P2PTUN_UDPPKG_TYPE_P2PRAWDATA 4

struct P2PTUN_TIME
{

    p2ptun_uint32 sec;  /* Seconds.  */
    p2ptun_uint32 usec; /* Microseconds.  */
};

int p2ptun_read_timeout(int fd, void *buf, unsigned int count, int time);
void p2ptun_get_current_time(struct P2PTUN_TIME *time);
int t1_sub_t2_sec(struct P2PTUN_TIME *t1, struct P2PTUN_TIME *t2);
int t1_sub_t2_ms(struct P2PTUN_TIME *t1, struct P2PTUN_TIME *t2);
int get_sub_tim_sec(struct P2PTUN_TIME *t2);

#endif
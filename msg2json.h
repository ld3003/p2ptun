#ifndef __msgjson_h__
#define __msgjson_h__

#define P2PTUN_CMD_UDPPING 100 /*< 发送到服务器的ping包 ，用于获取本机AP和网络类型*/
#define P2PTUN_CMD_UDPPONG 101 /*< ECHO服务器返回的 PONG包 ，包含公网IP和端口 */

#define P2PTUN_CMD_MSGPING 102  /*< 测试对方是否在线的PING包，服务器转发 */
#define P2PTUN_CMD_MSGPONG 8102 /*< 测试对方是否在线的PONG包，服务器转发 */

#define P2PTUN_CMD_MSGGETNTYPE 103    /*< 获取对方网络类型的消息 */
#define P2PTUN_CMD_MSGRRESPNTYPE 8103 /*< 返回网络类型的消息 */

#define P2PTUN_CMD_UDP_TEST 104      /*< UDP打洞消息 */
#define P2PTUN_CMD_UDP_RESPTEST 8104 /*< UDP打洞ECHO消息 */

#define P2PTUN_CMD_MSG_REQUESTUDPTEST 105   /*< 请求反向打洞的消息 */
#define P2PTUN_CMD_MSG_RESPONSEUDPTEST 8105 /*< 请求反向打洞的返回消息 */

#define P2PTUN_CMD_MSG_CONNECTED 106      /*< 确定打洞成功的命令 */
#define P2PTUN_CMD_MSG_RESPCONNECTED 8106 /*< 确定打洞成功的命令 */

#define P2PTUN_CMD_UDP_HB 107      /*< 隧道心跳消息 */
#define P2PTUN_CMD_UDP_ECHOHB 8107 /*< 隧道心跳消息 */

struct JSONDATA
{
    int cmd;
    int seq;
    char from[32];
    char to[32];
    char addr[32];
    int port;
    int ntype;
};

int json2data(char *json, struct JSONDATA *dat);
char *data2json(struct JSONDATA *dat);

#endif
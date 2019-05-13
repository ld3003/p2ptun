
#ifndef __p2ptun_session_status_h__
#define __p2ptun_session_status_h__

enum
{
    /* 最初的窗台 */
    P2PTUN_STATUS_INIT,
    /* 监听状态，等待客户端连接 */


    P2PTUN_STATUS_LISTEN,
    P2PTUN_STATUS_LISTEN_HANDSHAKE,
    P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE,
    P2PTUN_STATUS_LISTEN_HANDSHAKE_WAIT_GET_NETTYPE1,
    
    P2PTUN_STATUS_CONNECTING,
    P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE,
    P2PTUN_STATUS_CONNECTING_WAIT_GET_NETTYPE1,
    P2PTUN_STATUS_CONNECTING_WAIT_PONG,
    P2PTUN_STATUS_CONNECTING_WAIT_REMOTE_NETTYPE,
    P2PTUN_STATUS_CONNECTING_WAIT_UDPECHO,
    P2PTUN_STATUS_CONNECTING_WAIT_CONNECTED,

    P2PTUN_STATUS_CONNECTING_DOUBLECONE,
    P2PTUN_STATUS_CONNECTING_LOCALCONE,
    P2PTUN_STATUS_CONNECTING_REMOTECONE,
    P2PTUN_STATUS_CONNECTING_DOUBLESYMMETRIC,

    P2PTUN_STATUS_CONNECTED,
    P2PTUN_STATUS_DISCONNECT,
};

#endif
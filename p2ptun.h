#ifndef __p2ptun_h__
#define __p2ptun_h__

#include "p2ptun_common.h"
#include "ikcp.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define ENABLE_KCP 1
#define KCPBUFFER_COUNT (128)

	typedef int (*OUTPUT_DAT)(unsigned char *data, int length, char pkg_type);
	typedef int (*OUTPUT_P2PDAT)(unsigned char *data, int length);
	typedef int (*OUTPUT_P2PEVENT)(void *ctx, int event);

	enum
	{
		P2PTUN_OK = 0,
		P2PTUN_ERROR,
		P2PTUN_MEMERR,
		P2PTUN_KCPBUFFERERR,
		P2PTUN_STATUSERR,
	};

	enum
	{
		P2PTUN_WORKMODE_CLIENT = 0,
		P2PTUN_WORKMODE_SERVER,

	};

	struct P2PTUN_CONN_SESSION
	{
		char remote_peername[32];
		char local_peername[32];

		char remote_ipaddr[32];
		char local_ipaddr[32];

		unsigned short remote_port;
		unsigned short local_port;

		unsigned char remote_nettype;
		unsigned char local_nettype;

		unsigned char workmode;

		unsigned char prev_status;
		unsigned char cur_status;

		struct P2PTUN_TIME status_time;
		struct P2PTUN_TIME routing_time;
		struct P2PTUN_TIME getnettype_time;
		struct P2PTUN_TIME recvhb_time;
		struct P2PTUN_TIME regpkg_time;

		struct P2PTUN_TIME kcpupdate_time;

		ikcpcb *kcp;

		//P2PTUN 发送数据的底层回调函数，P2PTUN的数据将由此函数发送出去
		OUTPUT_DAT out_dat;

		//底层收到用户层的数据后回调此函数，应用层需要指定回调函数做接收数据的相应处理
		OUTPUT_P2PDAT out_p2pdat;
		OUTPUT_P2PDAT out_p2pdat_kcp;
		OUTPUT_P2PEVENT out_p2pevent;
	};

	struct P2PTUN_CONN_SESSION *p2ptun_alloc_session();
	int p2ptun_initkcp(struct P2PTUN_CONN_SESSION *session);
	int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg);
	//下层协议栈送来的数据
	int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);
	void p2ptun_update(struct P2PTUN_CONN_SESSION *session);
	void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status);
	int p2ptun_input_p2pdata_mux(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length, unsigned char mux);

	//应用层如果需要发送数据到对方调用这个函数
	int p2ptun_set_arrived_callback(struct P2PTUN_CONN_SESSION *session, OUTPUT_P2PDAT raw_cb, OUTPUT_P2PDAT kcp_cb);
	int p2ptun_input_p2pdata_raw(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);
	int p2ptun_input_p2pdata_kcp(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);

//kcp
#if (ENABLE_KCP == 1)
	int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user);
#endif

#ifdef __cplusplus
}
#endif

#endif

#ifndef __p2ptun_h__
#define __p2ptun_h__

#include "p2ptun_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

	typedef int (*OUTPUT_DAT)(unsigned char *data, int length , char pkg_type);
	typedef int (*OUTPUT_P2PDAT)(unsigned char *data, int length);


	enum
	{
		P2PTUN_OK = 0,
		P2PTUN_ERROR,
		P2PTUN_MEMERR,
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
		struct P2PTUN_TIME getnettype_time;
		struct P2PTUN_TIME recvhb_time;
		struct P2PTUN_TIME reg_time;

		OUTPUT_DAT out_dat;

		//底层收到用户层的数据后回调此函数，应用层需要指定回调函数做接收数据的相应处理
		OUTPUT_P2PDAT out_p2pdat;
	};



	struct P2PTUN_CONN_SESSION *p2ptun_alloc_session();
	int p2ptun_free_session(struct P2PTUN_CONN_SESSION *session);

	int p2ptun_input_msg(struct P2PTUN_CONN_SESSION *session, char *msg);
	int p2ptun_input_data(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);
	void p2ptun_input_timer(struct P2PTUN_CONN_SESSION *session);
	void p2ptun_setstatus(struct P2PTUN_CONN_SESSION *session, unsigned char status);

	//应用层如果需要发送数据到对方调用这个函数
	int p2ptun_input_p2pdata(struct P2PTUN_CONN_SESSION *session, unsigned char *data, int length);

#ifdef __cplusplus
};
#endif

#endif

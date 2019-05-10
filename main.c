#include <stdio.h>
#include "p2ptun.h"

int main(int argc, char **argv)
{
	struct P2PTUN_CONN_SESSION *p2psession_dev1 = p2ptun_alloc_session("device1",P2PTUN_WORKMODE_CLIENT);
	struct P2PTUN_CONN_SESSION *p2psession_dev2 = p2ptun_alloc_session("device2",P2PTUN_WORKMODE_SERVER);

	return 0;
}

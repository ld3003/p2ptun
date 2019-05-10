#include <stdio.h>
#include "p2ptun.h"

int main(int argc, char **argv)
{
	printf("create new p2ptunsession   kkk\n");

	struct P2PTUN_CONN_SESSION *p2psession_dev1 = p2ptun_alloc_session("device1");
	struct P2PTUN_CONN_SESSION *p2psession_dev2 = p2ptun_alloc_session("device2");

	return 0;
}

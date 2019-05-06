#include <stdio.h>
#include "p2ptun.h"

int main(int argc, char **argv)
{
	printf("create new p2ptunsession   kkk\n");
	
	struct P2PTUN_CONN_SESSION *p2psession = p2ptun_alloc_session();


	return 0;
}

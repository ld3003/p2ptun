#include "linux_udp.h"

static int linux_udp_sock;
static unsigned char udp_recv_buffer[2048];

void send_linux_udp_data(struct sockaddr_in *addr, unsigned char *data, int len)
{
    int n;
    n = sendto(linux_udp_sock, data, len, 0, addr, sizeof(struct sockaddr_in));
    if (n < 0)
    {
        perror("sendto");
        return ;
    }
    //
}

void create_udp_sock(short port, UDP_RECV_PACKAGE recvpkg_cb)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    size_t n;
    socklen_t len = sizeof(clientAddr);

    int linux_udp_sock;
    if ((linux_udp_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }
    //port bind to server
    if (bind(linux_udp_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(1);
    }

    for (;;)
    {
        n = recvfrom(linux_udp_sock, udp_recv_buffer, sizeof(udp_recv_buffer), 0, (struct sockaddr *)&clientAddr, &len);
        if (n > 0)
        {
            //inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)
            printf("RECV from %s : %d \n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));
            recvpkg_cb(&clientAddr, udp_recv_buffer, n);
        }
        else
        {
            perror("recv");
            break;
        }
    }
}

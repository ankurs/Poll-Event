#include<stdio.h>
#include<stdlib.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<string.h>

#include<sys/socket.h>
#include<netinet/in.h>

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in svr_addr, clt_addr;
    memset(&svr_addr, 0 , sizeof(svr_addr));
    svr_addr.sin_family = AF_INET;
    svr_addr.sin_addr.s_addr = htons(INADDR_ANY);
    svr_addr.sin_port = htons(9090);
    bind(sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr));
    listen(sock, 10);

    socklen_t clt_len = sizeof(struct sockaddr_in);

    struct epoll_event ev, events[10];

    int epollfd = epoll_create(10);
    ev.events = EPOLLIN;
    ev.data.fd = sock;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &ev);
    while(1)
    {
        int nfds = epoll_wait(epollfd, events, 10, -1);
        if (nfds == 0)
            fprintf(stderr, "timeout\n");
        int i =0;
        for (;i<nfds;i++)
        {
            if (events[i].data.fd == sock)
            {
                int listenfd = accept(sock, (struct sockaddr*) &clt_addr, &clt_len);
                fprintf(stderr, "got the socket %d\n", listenfd);
            }
        }
    }
    return 0;
}

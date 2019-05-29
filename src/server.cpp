//
// Created by Niujx on 2019/5/28.
//
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <exception>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAXLINE 4096


struct MyException : public std::exception
{
    const char* what () const noexcept
    {
        return "C++ Exception";
    }
};


int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[4096];
    int n;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(6666);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    if (listen(listenfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }

    printf("======waiting for client's request======\n");
    while (true) {
        try {
            if ((connfd = accept(listenfd, (struct sockaddr *) nullptr, nullptr)) == -1)
                throw MyException();

            n = recv(connfd, buff, MAXLINE, 0);
            buff[n] = '\0';
            printf("recv msg from client: %s\n", buff);
            close(connfd);
        }
        catch (MyException& e) {
            std::cout << e.what() << std::endl;
            break;
        }

    }

    close(listenfd);
}

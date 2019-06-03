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
#include <arpa/inet.h>

#define MAXLINE 4096


struct MyException : public std::exception {
    const char *what() const noexcept {
        return "C++ Exception";
    }
};


int main(int argc, char **argv) {
    int listenfd;
    struct sockaddr_in servaddr;

    if ((listenfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
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

    struct sockaddr_in remoteAddr;
    socklen_t nAddrLen = sizeof(remoteAddr);

    printf("======waiting for client's request======\n");
    while (true) {
        try {
            char recvData[255];
            int ret = recvfrom(listenfd, recvData, MAXLINE, 0, (struct sockaddr *) &remoteAddr, &nAddrLen);
            if (ret > 0) {
                recvData[ret] = 0x00;
                printf("Get a connect: %s \r\n", inet_ntoa(remoteAddr.sin_addr));
                std::cout << recvData << std::endl;
            } else
                throw MyException();

            const char *sendData = "One UDP package which come from server\n";
            sendto(listenfd, sendData, strlen(sendData), 0, (sockaddr *) &remoteAddr, nAddrLen);
        }
        catch (MyException &e) {
            std::cout << e.what() << std::endl;
            break;
        }

    }

    close(listenfd);
}

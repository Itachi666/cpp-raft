/**********************************************************************
 * Copyright (C) 2019 Niujx Ltd. All rights reserved.
 *
 * @Author: Niujx
 * @Email: niujx666@foxmail.com
 *
 * Description: A C++ implementation(Demo) of RAFT consensus algorithm.
 **********************************************************************/
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cerrno>
#include <sys/socket.h>
#include <json/json.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "node.h"

#define _DEBUG true
#define MAXLINE 4096

using namespace std;

struct TimeoutExecption : public std::exception {
    const char *what() const noexcept {
        return "Time out!";
    }
};

Address get_addr_by_str(char *s) {
    Address addr;
    addr.ip = strtok(s, ":");
    addr.port = atoi(strtok(nullptr, ":"));
    return addr;
}

struct sockaddr_in servaddr, remoteAddr;

void send_to(Json::Value msg, int udp_socket) {
    string buff = msg.toStyledString();
    sendto(udp_socket, buff.c_str(), buff.length(), 0, (sockaddr *) &remoteAddr, sizeof(remoteAddr));
}

map<string, string> kv;

void command_exec(const string command) {
    Json::Reader reader;
    Json::Value msg;

    if (reader.parse(command, msg)) {
        if (!msg["cmd"].isNull())
            if (msg["cmd"].asString()=="set")
                kv[msg["key"].asString()]=msg["val"].asString();
            else if (msg["cmd"].asString()=="del" && kv.find(msg["key"].asString())!=kv.end())
                kv.erase(msg["key"].asString());

    }



}


int main(int argc, char **argv) {
    if (argc < 5) {
        printf("Usage: ./raft node <selfHost:port> <partner1Host:port> <partner2Host:port> ...");
        return 0;
    }

    if (strcmp(argv[1], "node") == 0) {
        Address self = get_addr_by_str(argv[2]);
        vector<Address> partner;

        for (int i = 3; i < argc; i++) {
            partner.push_back(get_addr_by_str(argv[i]));
        }

        int udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (udp_socket < 0) {
            printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }

        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(self.ip.c_str());
        servaddr.sin_port = htons(self.port);

        if (bind(udp_socket, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
            printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
            return 0;
        }

        struct timeval timeout = {10, 500};//0.5s
        setsockopt(udp_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, sizeof(timeout));
        setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

        //如果ret==0 则为成功,-1为失败,这时可以查看errno来判断失败原因
//        int recvd = recv(sock_fd, buf, 1024, 0);
//        if (recvd == -1 && errno == EAGAIN)
//        {
//            printf("timeout\n");//这里可以直接关闭socket连接
//        }

        Node node(self, partner);
        node.setSendFunc(&send_to);
        node.setExecFunc(&command_exec);
        if (_DEBUG)
            node.Debug();


        printf("======waiting for client's request======\n");
        while (true) {
            try {
                char recvData[4096];
                socklen_t nAddrLen = sizeof(remoteAddr);
                int recvd = recvfrom(udp_socket, recvData, MAXLINE, 0, (struct sockaddr *) &remoteAddr, &nAddrLen);
                if (recvd > 0) {
                    recvData[recvd] = 0x00;
                    printf("Get a connect: %s:%d \r\n", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port));
                    std::cout << recvData << std::endl;
                } else if (recvd == -1 && errno == EAGAIN)
                    throw TimeoutExecption();

                string t=R"({"ret": 888, "err": "One UDP package which come from server"})";
                const char *sendData = t.c_str();
                sendto(udp_socket, sendData, strlen(sendData), 0, (sockaddr *) &remoteAddr, nAddrLen);
                cout<<"Send success"<<endl;
            }
            catch (TimeoutExecption &e) {
                std::cout << e.what() << std::endl;
                break;
            }

        }

        node.output();
    }

    string test = R"({"id":1,"name":"kurama"})";
    Json::Reader reader;
    Json::Value value;
    if (reader.parse(test, value)) {
        if (!value["id"].isNull()) {
            cout << value["id"].asInt() << endl;
            cout << value["name"].asString() << endl;

        }
    }

    return 0;
}
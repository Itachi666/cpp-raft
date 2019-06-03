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
#include <algorithm>
#include <cerrno>
#include <sys/socket.h>
#include <json/json.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory>

#include "node.h"

#define _DEBUG true
#define MAXLINE 65536

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

int udp_socket;

void send_to(Json::Value &msg, Address &addr) {
    if (msg["ret"].isNull())
        msg["_raft"] = 1;

    Json::StreamWriterBuilder writebuilder;
    writebuilder.settings_["indentation"] = "";
    string buff = Json::writeString(writebuilder, msg);

    struct sockaddr_in remoteAddr;
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_addr.s_addr = inet_addr(addr.ip.c_str());
    remoteAddr.sin_port = htons(addr.port);

    sendto(udp_socket, buff.c_str(), buff.length(), 0, (sockaddr *) &remoteAddr, sizeof(remoteAddr));
}

map<string, string> kv;
int seq = 0;
map<int, Address> session;

void command_exec(const string &command) {
    Json::Value msg;

    Json::CharReaderBuilder readerBuilder;
    readerBuilder["collectComments"] = false;
    JSONCPP_STRING errs;
    Json::CharReader *reader = readerBuilder.newCharReader();
    if (!reader->parse(command.data(), command.data() + command.size(), &msg, &errs))
        return;

    if (!msg["cmd"].isNull())
        if (msg["cmd"].asString() == "set")
            kv[msg["key"].asString()] = msg["val"].asString();
        else if (msg["cmd"].asString() == "del" && kv.find(msg["key"].asString()) != kv.end())
            kv.erase(msg["key"].asString());


    if (!msg["seq"].isNull() && session.find(msg["seq"].asInt()) != session.end()) {
        Address addr = session[msg["seq"].asInt()];
        session.erase(msg["seq"].asInt());
        Json::Value tmp;

        if (msg["cmd"].asString() == "get") {
            if (_DEBUG)
                cout << "Special strong consistency read" << endl;
            if (kv.find(msg["key"].asString()) != kv.end()) {
                tmp["ret"] = 0;
                tmp["val"] = kv[msg["key"].asString()];
                send_to(tmp, addr);
            } else {
                tmp["ret"] = -1;
                tmp["err"] = "Not Found";
                send_to(tmp, addr);
            }
        } else {
            tmp["ret"] = 0;
            send_to(tmp, addr);
        }
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

        udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        struct sockaddr_in servaddr, remoteAddr;

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

        struct timeval timeout = {0, 100};//0.1s
        setsockopt(udp_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *) &timeout, sizeof(timeout));
        setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &timeout, sizeof(timeout));

        Node node(self, partner);
        node.setSendFunc(&send_to);
        node.setExecFunc(&command_exec);
        if (_DEBUG)
            node.Debug();


        printf("======waiting for client's request======\n");
        while (true) {
            try {
                char recvData[65536];
                socklen_t nAddrLen = sizeof(remoteAddr);
                int rec = recvfrom(udp_socket, recvData, MAXLINE, 0, (struct sockaddr *) &remoteAddr, &nAddrLen);
                if (rec > 0) {
                    recvData[rec] = 0x00;
                    //printf("Get a connect: %s:%d \r\n", inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port));
                    //std::cout << recvData << std::endl;
                } else if (rec == -1 && errno == EAGAIN)
                    throw TimeoutExecption();

                Address addr(inet_ntoa(remoteAddr.sin_addr), htons(remoteAddr.sin_port));

                Json::Value msg;
                string command = recvData;
                Json::CharReaderBuilder readerBuilder;
                readerBuilder["collectComments"] = false;
                JSONCPP_STRING errs;
                Json::CharReader *reader = readerBuilder.newCharReader();
                if (!reader->parse(command.data(), command.data() + command.size(), &msg, &errs))
                    throw errs;

                if (!msg["_raft"].isNull()) {
                    msg.removeMember("_raft");
                    node.messageRecv(addr, msg);
                } else {
                    while (true) {
                        if (msg["cmd"].isNull() || (msg["cmd"].asString() != "get" && msg["cmd"].asString() != "set" &&
                                                    msg["cmd"].asString() != "del"))
                            break;
                        if (msg["key"].isNull())
                            break;
                        if (msg["cmd"].asString() == "set" && msg["val"].isNull())
                            break;

                        if (_DEBUG)
                            if (msg["cmd"].asString() != "get")
                                cout << "New Request: " << command << endl;

                        if (!node.isLeader()) {
                            Json::Value tmp;
                            tmp["ret"] = -999;
                            tmp["err"] = "Not Leader";
                            tmp["redirect"] = node.getLeader();
                            send_to(tmp, addr);
                            break;
                        }

                        if (msg["cmd"].asString() == "get" && !node.isReadOnlyNeedAppendCommend()) {
                            Json::Value tmp;
                            if (kv.find(msg["key"].asString()) != kv.end()) {
                                tmp["ret"] = 0;
                                tmp["val"] = kv[msg["key"].asString()];
                            } else {
                                tmp["ret"] = -1;
                                tmp["err"] = "Not Found";
                            }
                            send_to(tmp, addr);
                        } else {
                            seq++;
                            session[seq] = addr;
                            msg["seq"] = seq;

                            Json::StreamWriterBuilder writebuilder;
                            writebuilder.settings_["indentation"] = "";
                            node.appendCommand(Json::writeString(writebuilder, msg));
                        }
                        break;
                    }
                }
            }
            catch (TimeoutExecption &e) {
                //cout << e.what() << endl;
            }
            catch (JSONCPP_STRING &e) {
                cout << "Wrong json command: " << endl << e << endl;
                break;
            }
            node.tick();
        }
        node.output();
    }

    return 0;
}
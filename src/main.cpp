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
#include <sys/socket.h>
#include <json/json.h>

#include "node.h"

using namespace std;

Address get_addr_by_str(char *s) {
    Address addr;
    addr.ip = strtok(s, ":");
    addr.port = atoi(strtok(nullptr, ":"));
    return addr;
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

        Node node(self, partner);
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
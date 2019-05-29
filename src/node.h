//
// Created by Niujx on 2019/5/28.
//

#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <string>

struct Address {
    std::string ip;
    int port;
};

class Node {
public:
    Node() = default;
};


#endif //RAFT_NODE_H

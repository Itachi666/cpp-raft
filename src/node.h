//
// Created by Niujx on 2019/5/28.
//

#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <string>
#include <vector>
#include <ctime>

struct Address {
    std::string ip;
    int port;

    Address(std::string s = "", int p = 0) : ip(std::move(s)), port(p) {};
};

struct {
    int FOLLOWER = 1;
    int CANDIDATE = 2;
    int LEADER = 3;
} _STATE;

class Node {
public:
    Node(Address &saddr, std::vector<Address> &paddr);

    ~Node() = default;

    void output();

private:
    Address self_addr;
    std::vector<Address> part_addrs;
    int state = _STATE.FOLLOWER;
    int votes_count = 0;
    Address leader;

    time_t now = clock();
    time_t election_dl = now + genTimedl();
    time_t genTimedl();


};


#endif //RAFT_NODE_H

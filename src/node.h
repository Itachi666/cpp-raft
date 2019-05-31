//
// Created by Niujx on 2019/5/28.
//

#ifndef RAFT_NODE_H
#define RAFT_NODE_H

#include <string>
#include <vector>
#include <map>
#include <ctime>
#include <utility>

#include "Log.h"

struct Address {
    std::string ip;
    int port;

    explicit Address(std::string s = "", int p = 0);
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

    bool isLeader() { return state == _STATE.LEADER; };

    int getLeader() { return state; };

    bool isReadOnlyNeedAppendCommend() { return }

    void output();

private:
    Address self_addr;
    std::vector<Address> part_addrs;
    int state = _STATE.FOLLOWER;
    int votes_count = 0;
    Address leader;
    Address votedFor;

    int currentTerm = 0;
    int votesCount = 0;
    BaseLog log;

    int commitIndex = 0;
    int lastApplied = 0;
    int leaderCommitIndex = 0;

    std::map<Address, int> nextIndex;
    std::map<Address, int> matchIndex;

    bool debug = false;

    time_t now = clock();
    time_t election_dl = now + genTimedl();

    time_t genTimedl();
};


#endif //RAFT_NODE_H

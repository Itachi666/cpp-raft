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
#include <json/json.h>

#include "Log.h"

struct Address {
    std::string ip;
    int port;

    explicit Address(std::string s = "NULL", int p = 0);
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

    void setSendFunc(void (*func)(Json::Value, int));

    void setExecFunc(void (*func)(std::string));

    bool isLeader() { return state == _STATE.LEADER; };

    int getLeader() { return state; };

    bool isReadOnlyNeedAppendCommend() { return needLastApplied > lastApplied; };

    void tick();

    void messageRecv(Address addr, Json::Value msg);

    void becomeLeader();

    void Debug(bool flag = true);

    void output();

private:
    Address self_addr, leader, votedFor;
    std::vector<Address> part_addrs;

    void (*_send)(Json::Value, int) = nullptr;
    void (*_exec)(std::string) = nullptr;

    int state = _STATE.FOLLOWER;
    int votes_count = 0;

    BaseLog log;

    int currentTerm = 0, votesCount = 0;

    int needLastApplied = 0;

    int commitIndex = 0, lastApplied = 0, leaderCommitIndex = 0;

    std::map<Address, int> nextIndex, matchIndex;

    bool debug = false;

    time_t now = clock();
    time_t election_dl = now + genTimedl();

    time_t genTimedl();
};


#endif //RAFT_NODE_H

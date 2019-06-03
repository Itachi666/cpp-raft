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

    std::string toString();
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

    void setSendFunc(void (*func)(Json::Value&, Address &));

    void setExecFunc(void (*func)(const std::string&));

    bool isLeader() { return state == _STATE.LEADER; };

    int getLeader() { return state; };

    bool isReadOnlyNeedAppendCommend() { return needLastApplied > lastApplied; };

    void tick();

    void messageRecv(Address& addr, Json::Value& msg);

    void Debug(bool flag = true);

    void output();

private:
    Address self_addr, leader, votedFor;
    std::vector<Address> part_addrs;

    void (*_send)(Json::Value&, Address &) = nullptr;

    void (*_exec)(const std::string&) = nullptr;

    int state = _STATE.FOLLOWER;

    BaseLog log;

    int currentTerm = 0, votesCount = 0;
    int needLastApplied = 0;
    int commitIndex = 0, lastApplied = 0, leaderCommitIndex = 0;


    std::map<std::string, int> nextIndex, matchIndex;

    std::string INIT_COMMAND="Init";
    bool debug = false;

    time_t now = clock(), newAppendEntriesTime=clock();
    time_t election_dl = now + genTimedl();

    time_t genTimedl();

    void becomeLeader();

    void sendAppendEntriesReq();

    void sendAppendEntriesRsp(Address &addr, int next_index = -1, bool reset = false, bool success = false);

    std::vector<Entry> json2entries(const Json::Value& json);

    Json::Value entries2json(const std::vector<Entry>& entries);

    void debug_show(const std::string& s="NULL", int pIndex=0, int pTerm=0, const std::string& entries="NULL");

};


#endif //RAFT_NODE_H

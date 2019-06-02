//
// Created by Niujx on 2019/5/28.
//

#include "node.h"

#include <iostream>

#define HEARTBREAK 500

Address::Address(std::string s, int p) : ip{std::move(s)}, port{p} {}

std::string Address::toString() {
    return ip + ":" + std::to_string(port);
}

Node::Node(Address &saddr, std::vector<Address> &paddr) : self_addr(saddr), part_addrs(paddr) {
    if (this->log.length() == 0)
        this->log.add();

    for (int i = 0; i < part_addrs.size(); i++)
        nextIndex[part_addrs[i].toString()] = 50;
}

void Node::Debug(const bool flag) {
    debug = flag;
}

void Node::setSendFunc(void (*func)(Json::Value, Address &)) {
    _send = func;
}

void Node::setExecFunc(void (*func)(std::string)) {
    _exec = func;
}

void Node::tick() {
    now = clock();
}

void Node::messageRecv(Address addr, Json::Value msg) {
    now = clock();
}

void Node::becomeLeader() {
    leader = self_addr;
    state = _STATE.LEADER;

}

void Node::sendAppendEntriesReq() {
    now = clock();
    newAppendEntriesTime = now + HEARTBREAK;

    for (int i = 0; i < part_addrs.size(); i++) {
        bool sendLeastOne = true;
        int next_index = nextIndex[part_addrs[i].toString()];
        while (sendLeastOne) {
            int prev_index = 0, prev_term = 0;
            Json::Value msg;
            msg["type"] = "AppendEntriesReq";
            msg["term"] = currentTerm;
            msg["commit_index"] = commitIndex;
            msg["entries"] = 1; //TODO:entries
            msg["prev_log_index"] = prev_index;
            msg["prev_log_term"] = prev_term;

            _send(msg, part_addrs[i]);
            next_index = nextIndex[part_addrs[i].toString()];
            sendLeastOne = false;
        }
    }

}

void Node::sendAppendEntriesRsp(Address &addr, int next_index, bool reset, bool success) {
    if (next_index == -1)
        next_index = 1;  //TODO: log finished

    Json::Value msg;
    msg["type"] = "AppendEntriesRsp";
    msg["next"] = next_index;
    msg["reset"] = reset;
    msg["success"] = success;

    _send(msg, addr);
}

time_t Node::genTimedl() {
    int tmin = 500, tmax = 1500;
    return tmin + (random() % (tmax - tmin));
}

void Node::output() {
    this->sendAppendEntriesReq(

    );
}

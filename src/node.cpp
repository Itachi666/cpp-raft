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
        this->log.add(INIT_COMMAND, 1, 0);

    for (auto &part_addr : part_addrs)
        nextIndex[part_addr.toString()] = 3;
}

void Node::Debug(const bool flag) {
    debug = flag;
}

void Node::setSendFunc(void (*func)(Json::Value, Address &)) {
    _send = func;
}

void Node::setExecFunc(void (*func)(const std::string&)) {
    _exec = func;
}

void Node::tick() {
    now = clock();
    if (state == _STATE.FOLLOWER || state == _STATE.CANDIDATE) {
        if (election_dl < now) {
            election_dl = now + genTimedl();
            state = _STATE.CANDIDATE;
            leader = Address();
            currentTerm++;
            votedFor = self_addr;
            votes_count = 1;
            for (auto addr:part_addrs) {
                Json::Value msg;
                msg["type"] = "RequestVoteReq";
                msg["term"] = currentTerm;
                msg["last_log_index"] = log.getCurrIndex();
                msg["last_log_term"] = log.getCurrTerm();
                _send(msg, addr);

                if (debug)
                    printf("[%d] RequestVoteReq to %s, log(%d, %d)\n", msg["term"].asInt(),
                           addr.toString().c_str(), msg["last_log_index"].asInt(), msg["last_log_term"].asInt());
            }
        }
    } else if (state == _STATE.LEADER) {
        while (commitIndex < log.getCurrIndex()) {
            int nextCommitIndex = commitIndex + 1;
            int count = 1;
            for (auto addr: part_addrs)
                if (matchIndex[addr.toString()] >= nextCommitIndex)
                    count++;
            if (count > (part_addrs.size() + 1) / 2)
                commitIndex = nextCommitIndex;
            else
                break;
        }
        leaderCommitIndex = commitIndex;
        if (newAppendEntriesTime < now)
            sendAppendEntriesReq();
    }

    if (commitIndex > lastApplied) {
        int count = commitIndex - lastApplied;
        std::vector<StandardLog> entries = log.getEntries(lastApplied + 1, count);
        for (auto& entry:entries) {
            if (entry.command != INIT_COMMAND) {
                if (debug)
                    std::cout << "Exec command= " << entry.command << " ..." << std::endl;
                //TODO: exec command
            }
            lastApplied++;
        }
    }
}

void Node::messageRecv(Address addr, Json::Value msg) {
    now = clock();
}

void Node::becomeLeader() {
    leader = self_addr;
    state = _STATE.LEADER;

    if (this->debug)
        std::cout << "Becoming New Leader " << leader.toString() << std::endl;

    for (auto addr:part_addrs) {
        nextIndex[addr.toString()] = log.getCurrIndex();
        matchIndex[addr.toString()] = 0;
    }

    needLastApplied = log.getCurrIndex();
    log.add(INIT_COMMAND, log.getCurrIndex() + 1, currentTerm);
    sendAppendEntriesReq();
}

void Node::sendAppendEntriesReq() {
    now = clock();
    newAppendEntriesTime = now + HEARTBREAK;

    for (auto &part_addr : part_addrs) {
        bool sendLeastOne = true;
        int next_index = nextIndex[part_addr.toString()];

        while (next_index <= log.getCurrIndex() && sendLeastOne) {
            int prev_index = 0, prev_term = 0;
            log.getPrevIndex_Term(next_index, &prev_index, &prev_term);
            std::vector<StandardLog> entries = log.getEntries(next_index);

            Json::Value entry;
            Json::Value msg;
            msg["type"] = "AppendEntriesReq";
            msg["term"] = currentTerm;
            msg["commit_index"] = commitIndex;
            msg["entries"].append(entry); //TODO:entries
            msg["prev_log_index"] = prev_index;
            msg["prev_log_term"] = prev_term;

            _send(msg, part_addr);
            next_index = nextIndex[part_addr.toString()];
            sendLeastOne = false;
        }
    }

}

void Node::sendAppendEntriesRsp(Address &addr, int next_index, bool reset, bool success) {
    if (next_index == -1)
        next_index = log.getCurrIndex() + 1;

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
    tick();
    std::cout << leader.toString() << std::endl;
}

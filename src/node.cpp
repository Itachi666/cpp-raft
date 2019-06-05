//
// Created by Niujx on 2019/5/28.
//

#include "node.h"

#include <iostream>
#include <cassert>
#include <algorithm>

#define HEARTBREAK 150
#define WAITINTTIME 500


Address::Address(std::string s, int p) : ip{std::move(s)}, port{p} {}

std::string Address::toString() {
    return ip + ":" + std::to_string(port);
}

Node::Node(Address &saddr, std::vector<Address> &paddr) : self_addr(saddr), part_addrs(paddr) {
    if (this->log.length() == 0)
        this->log.add(Entry{INIT_COMMAND, 1, 0});

    for (auto &part_addr : part_addrs)
        nextIndex[part_addr.toString()] = 0;
}

void Node::Debug(const bool flag) {
    debug = flag;
}

void Node::setSendFunc(void (*func)(Json::Value &, Address &)) {
    _send = func;
}

void Node::setExecFunc(void (*func)(const std::string &)) {
    _exec = func;
}

std::vector<Entry> Node::json2entries(const Json::Value &json) {
    std::vector<Entry> entries;
    for (auto &i : json)
        entries.emplace_back(i[0].asString(), i[1].asInt(), i[2].asInt());
    return entries;
}

Json::Value Node::entries2json(const std::vector<Entry> &entries) {
    Json::Value json;
    for (const auto &entry:entries) {
        Json::Value msg;
        msg.append(entry.command);
        msg.append(entry.index);
        msg.append(entry.term);
        json.append(msg);
    }
    return json;
}

void Node::appendCommand(const std::string &command) {
    assert(state == _STATE.LEADER);

    log.add(Entry{command, log.getCurrIndex() + 1, currentTerm});
    sendAppendEntriesReq();
}

void Node::onTick() {
    now = clock();
    if (state == _STATE.FOLLOWER || state == _STATE.CANDIDATE) {
        if (election_dl < now) {
            election_dl = now + genTimedl();
            state = _STATE.CANDIDATE;
            leader = Address();
            currentTerm++;
            votedFor = self_addr;
            votesCount = 1;
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
        std::vector<Entry> entries = log.getEntries(lastApplied + 1, count);
        for (auto &entry:entries) {
            if (entry.command != INIT_COMMAND) {
                if (debug)
                    std::cout << "Exec command = " << entry.command << " ..." << std::endl;
                _exec(entry.command);
            }
            lastApplied++;
        }
    }
}

void Node::messageRecv(Address &addr, Json::Value &msg) {
    now = clock();

    if (msg["type"] == "RequestVoteReq") {
        //{'type': 'RequestVoteReq', 'term': 1, 'last_log_index': 1, 'last_log_term': 0, '_raft': 1}
        if (debug)
            printf("[%d] RequestVoteReq from %s, log(%d, %d)\n", msg["term"].asInt(),
                   addr.toString().c_str(), msg["last_log_index"].asInt(), msg["last_log_term"].asInt());

        if (msg["term"].asInt() > currentTerm) {
            currentTerm = msg["term"].asInt();
            votedFor = Address();
            state = _STATE.FOLLOWER;
            leader = Address();
        }
        if (state == _STATE.FOLLOWER || state == _STATE.CANDIDATE)
            if (msg["term"].asInt() >= currentTerm) {
                int lastIndex = msg["last_log_index"].asInt();
                int lastTerm = msg["last_log_index"].asInt();
                if (lastTerm < log.getCurrIndex() || (lastTerm == log.getCurrIndex() && lastIndex < log.getCurrIndex()))
                    return;
                if (votedFor.ip != "NULL")
                    return;

                votedFor = addr;
                election_dl = now + genTimedl();
                Json::Value msg_send;
                msg_send["type"] = "RequestVoteRsp";
                msg_send["term"] = msg["term"].asInt();
                _send(msg_send, addr);
                if (debug)
                    printf("[%d] RequestVoteRsp to %s\n", msg["term"].asInt(), addr.toString().c_str());
            }

    } else if (msg["type"] == "RequestVoteRsp") {
        if (debug)
            printf("[%d] RequestVoteRsp from %s\n", msg["term"].asInt(), addr.toString().c_str());
        if (state == _STATE.CANDIDATE && msg["term"].asInt() == currentTerm) {
            votesCount++;
            if (votesCount > (part_addrs.size() + 1) / 2)
                becomeLeader();
        }

    } else if (msg["type"] == "AppendEntriesReq") {
        Json::StreamWriterBuilder writebuilder;
        writebuilder.settings_["indentation"] = "";

        //{'type': 'AppendEntriesReq', 'term': 1, 'commit_index': 3, 'entries': [], 'prev_log_index': 3, 'prev_log_term': 1, '_raft': 1}
        if (msg["term"].asInt() >= currentTerm)
            election_dl = now + genTimedl();
        if (leader.toString() != addr.toString()) {
            leader = addr;
            if (debug)
                std::cout << "Follow New Leader " << leader.toString() << std::endl;
        }
        if (msg["term"].asInt() > currentTerm) {
            currentTerm = msg["term"].asInt();
            votedFor = Address();
        }
        state = _STATE.FOLLOWER;
        std::vector<Entry> newEntries = json2entries(msg["entries"]);
        leaderCommitIndex = msg["commit_index"].asInt();

        int prevIndex = msg["prev_log_index"].asInt();
        int prevTerm = msg["prev_log_term"].asInt();
        std::vector<Entry> prevEntries = log.getEntries(prevIndex);

        if (prevEntries.empty()) {
            debug_show("[Missing more]", prevIndex, prevTerm, Json::writeString(writebuilder, msg["entries"]));

            sendAppendEntriesRsp(addr, -1, true, false);
            return;
        }

        if (prevEntries[0].term != prevTerm) {
            debug_show("[Last conflict]", prevIndex, prevTerm, Json::writeString(writebuilder, msg["entries"]));
            sendAppendEntriesRsp(addr, prevIndex, true, false);
            return;
        }

        if (prevEntries.size() > 1) {
            debug_show("[Redundant]", prevIndex, prevTerm, Json::writeString(writebuilder, msg["entries"]));
            log.deleteEntriesFrom(prevIndex + 1);
        }

        int next_index = prevIndex + 1;
        if (!newEntries.empty()) {
            debug_show("[Append]", prevIndex, prevTerm, Json::writeString(writebuilder, msg["entries"]));
            for (const auto &entry:newEntries) {
                assert(log.length() + 1 == entry.index);
                log.add(entry);
            }
            next_index = newEntries.back().index;
            if (debug)
                printf("Local Logs[%d] (New)\n", log.length());
        }
        sendAppendEntriesRsp(addr, next_index, false, true);
        commitIndex = std::min(leaderCommitIndex, log.length());

    } else if (msg["type"] == "AppendEntriesRsp") {
        //{'type': 'AppendEntriesRsp', 'next_index': 3, 'reset': False, 'success': True, '_raft': 1}
        if (state == _STATE.LEADER) {
            int next_index = msg["next_index"].asInt(), curr_index = next_index - 1;
            bool reset = msg["reset"].asBool();
            bool success = msg["success"].asBool();

            if (reset)
                nextIndex[addr.toString()] = next_index;
            if (success)
                matchIndex[addr.toString()] = curr_index;
        }
    }
}

void Node::becomeLeader() {
    leader = self_addr;
    state = _STATE.LEADER;

    if (this->debug)
        std::cout << "Becoming New Leader " << leader.toString() << std::endl;

    for (auto addr:part_addrs) {
        nextIndex[addr.toString()] = log.getCurrIndex() + 1;
        matchIndex[addr.toString()] = 0;
    }

    needLastApplied = log.getCurrIndex();

    log.add(Entry{INIT_COMMAND, log.getCurrIndex() + 1, currentTerm});
    sendAppendEntriesReq();
}

void Node::sendAppendEntriesReq() {
    now = clock();
    newAppendEntriesTime = now + HEARTBREAK;

    for (auto &part_addr : part_addrs) {
        bool sendLeastOne = true;
        int next_index = nextIndex[part_addr.toString()];

        while (next_index <= log.getCurrIndex() || sendLeastOne) {
            int prev_index = 0, prev_term = 0;

            log.getPrevIndex_Term(next_index, &prev_index, &prev_term);
            std::vector<Entry> entries;

            if (next_index <= log.getCurrIndex()) {
                entries = log.getEntries(next_index);
                nextIndex[part_addr.toString()] = entries.back().index + 1;
            }

            Json::Value entry = entries2json(entries);
            Json::Value msg;
            msg["type"] = "AppendEntriesReq";
            msg["term"] = currentTerm;
            msg["commit_index"] = commitIndex;
            msg["entries"] = entry;
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
    msg["next_index"] = next_index;
    msg["reset"] = reset;
    msg["success"] = success;

    _send(msg, addr);
}

void Node::debug_show(const std::string &s, int pIndex, int pTerm, const std::string &entries) {
    if (!debug)
        return;
    if (s != "NULL")
        std::cout << s << std::endl;
    printf("Leader:\n"
           "    prevIndex: %d\n"
           "    prevTerm:  %d\n"
           "    entries: %s\n"
           "Local Logs[%d]\n", pIndex, pTerm, entries.c_str(), log.length());
}

time_t Node::genTimedl() {
    int tmin = WAITINTTIME, tmax = tmin + 1000;
    return tmin + (random() % (tmax - tmin));
}

void Node::testplase() {

    printf("%d %d\n", log.getCurrTerm(), currentTerm);


    Json::Value msg, command;
    std::string s = R"({'type': 'AppendEntriesReq', 'term': 1, 'commit_index': 2, 'entries': [], 'prev_log_index': 2, 'prev_log_term': 1, '_raft': 1})";
    std::replace(s.begin(), s.end(), '\'', '\"');

    std::string t = (R"({"cmd": "set", "key": "niu", "val": "123", "seq": 1})");
    for (int i = 0; i < 5; i++) {
        command[i].append(t);
        command[i].append(i + 1);
        command[i].append(i + 3);
    }


    Json::CharReaderBuilder readerBuilder;
    readerBuilder["collectComments"] = false;
    JSONCPP_STRING errs;
    Json::CharReader *reader = readerBuilder.newCharReader();
    if (!reader->parse(s.data(), s.data() + s.size(), &msg, &errs)) {
        std::cout << s << std::endl;
        return;
    }
    msg["entries"] = (command);
}

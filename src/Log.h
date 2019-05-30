//
// Created by Niujx on 2019/5/28.
//
#include <vector>
#include <map>

#ifndef RAFT_LOG_H
#define RAFT_LOG_H


class Log {
public:
    Log() = default;

    ~Log() = default;

    void add();
};

class BaseLog : public Log {
public:
    BaseLog() = default;

    ~BaseLog() = default;
};

#endif //RAFT_LOG_H

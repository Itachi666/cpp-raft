//
// Created by Niujx on 2019/5/28.
//
#include <vector>

#ifndef RAFT_LOG_H
#define RAFT_LOG_H


class Log {
public:
    Log() = default;

    ~Log() = default;

    virtual void add();
};

class BaseLog : public Log {
public:
    BaseLog() = default;

    ~BaseLog() = default;

    void add() override;

    int length();

    void clear();

private:
    std::vector<int> log;
};

#endif //RAFT_LOG_H

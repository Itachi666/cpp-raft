//
// Created by Niujx on 2019/5/28.
//
#include <vector>
#include <string>

#ifndef RAFT_LOG_H
#define RAFT_LOG_H

struct Entry {
    std::string command;
    int term;
    int index;

    explicit Entry(std::string s = "NULL", int x = 0, int y = 0);
};

class Log {
public:
    Log() = default;

    ~Log() = default;

    void add(const Entry& entry);

    int getCurrIndex() { return logs.back().index; };

    int getCurrTerm() { return logs.back().term; };

    void getPrevIndex_Term(int next_index, int *prevIndex, int *prevTerm);

    std::vector<Entry> getEntries(int start_index, int num = -1);

protected:
    std::vector<Entry> logs;
};

class BaseLog : public Log {
public:
    BaseLog() = default;

    ~BaseLog() = default;

    int length();

    void clear();

private:

};

#endif //RAFT_LOG_H

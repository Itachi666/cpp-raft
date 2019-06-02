//
// Created by Niujx on 2019/5/28.
//

#include "Log.h"
#include <iostream>

using namespace std;

StandardLog::StandardLog(std::string s, int i, int t) : command{std::move(s)}, index{i}, term{t} {}

void Log::add(std::string &command, int index, int term) {
    StandardLog log(command, index, term);
    logs.push_back(log);
}

void Log::getPrevIndex_Term(int next_index, int *prevIndex, int *prevTerm) {
    int pIndex = next_index - 1;
    std::vector<StandardLog> entries = getEntries(pIndex, 1);
    if (!entries.empty()) {
        *prevIndex = pIndex;
        *prevTerm = entries[0].term;
    } else {
        *prevIndex = -1;
        *prevTerm = -1;
    }
}

std::vector<StandardLog> Log::getEntries(int start_index, int num) {
    int first_index = logs[0].index;
    std::vector<StandardLog> entries;
    if (start_index < first_index)
        return entries;

    int diff = start_index - first_index;
    if (num == -1) {
        for (int i = diff - 1; i < logs.size(); i++)
            entries.push_back(logs[i]);
    } else {
        for (int i = diff - 1; i < std::min(diff+num, (int)logs.size()); i++)
            entries.push_back(logs[i]);
    }
    return entries;
}

void BaseLog::clear() {
    this->logs.clear();
}

int BaseLog::length() {
    return this->logs.size();
}

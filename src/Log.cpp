//
// Created by Niujx on 2019/5/28.
//

#include "Log.h"
#include <iostream>
using namespace std;

void Log::add() {
    cout<<"Hello"<<endl;
}

void BaseLog::add() {
    cout<<"Hello BaseLog"<<endl;
}

void BaseLog::clear() {
    this->log.clear();
}

int BaseLog::length() {
    return this->log.size();
}

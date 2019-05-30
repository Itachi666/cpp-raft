//
// Created by Niujx on 2019/5/28.
//

#include "node.h"
#include "Log.h"

#include <iostream>

Node::Node(Address &saddr, std::vector<Address> &paddr) : self_addr(saddr), part_addrs(paddr) {

}

time_t Node::genTimedl() {
    int tmin = 500, tmax = 1500;
    return tmin +  (random() % (tmax - tmin));
}

void Node::output() {
    std::cout<<now<<" "<<election_dl<<std::endl;
}
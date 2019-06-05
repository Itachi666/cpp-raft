#==================================================================
# Copyright (C) 2019 Niujx Ltd. All rights reserved.
# File Name: Makefile
# Author: Niujx
# Mail: niujx666@foxmail.com
# Created Time:  2019-05-28 17:50:17
# Last modified: 2019-06-05 16:57:31
# Descirption:
#==================================================================
.PHONY: server node1 node2 node3

all:
	@echo "Usage: make server/node1/node2/node3/test"

server:
	@python raft_http.py 9909 127.0.0.1:9901 127.0.0.1:9902 127.0.0.1:9903

node1:
	@./build/raft node 127.0.0.1:9901 127.0.0.1:9902 127.0.0.1:9903

node2:
	@./build/raft node 127.0.0.1:9902 127.0.0.1:9901 127.0.0.1:9903

node3:
	@./build/raft node 127.0.0.1:9903 127.0.0.1:9902 127.0.0.1:9901


test:
	python raft_http_test.py 127.0.0.1:9909 10 100

# cpp-raft 

This is a C++ implementation of the [RAFT](https://raft.github.io/) distributed consensus protocol. Just for learning.

For more details on Raft, you can read [In Search of an Understandable Consensus Algorithm](https://raft.github.io/raft.pdf) by Diego Ongaro and John Ousterhout.

### Prerequisites

- cmake
- python 3.7
- [Flask](http://flask.pocoo.org/)

### Build
```shell
$cmake CmakeLists.txt && make
```

### Run

```shell
$./raft node 127.0.0.1:9901 127.0.0.1:9902 127.0.0.1:9903
$./raft node 127.0.0.1:9902 127.0.0.1:9901 127.0.0.1:9903
$./raft node 127.0.0.1:9903 127.0.0.1:9901 127.0.0.1:9902
$python raft_http.py 9909 127.0.0.1:9901 127.0.0.1:9902 127.0.0.1:9903
```

### Test

```shell
$python raft_http_test.py 127.0.0.1:9909 10 100
```


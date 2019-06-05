# cpp-raft 

This is a C++ implementation of the [RAFT](https://raft.github.io/) distributed consensus protocol. Just for learning. 

For more details on Raft, you can read [In Search of an Understandable Consensus Algorithm](https://raft.github.io/raft.pdf) by Diego Ongaro and John Ousterhout.  

Also have some reference from [chishaxie/py-raft](https://github.com/chishaxie/py-raft).

Todo：

- Cluster membership changes
- Log compaction

### Dep

- CMake 3.14.4
- python 3.7.3
- [Jsoncpp](https://github.com/open-source-parsers/jsoncpp)
- [Flask](http://flask.pocoo.org/)

### Build
```shell
$ mkdir build 
$ cd build
$ cmake ../ && make
```

### Run
Back to the source dir.
```shell
$ make node1  	
$ make node2	
$ make node3	
$ make server   
```

### Test

```shell
$ make test
```


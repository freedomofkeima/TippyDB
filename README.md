# TippyDB

By: Iskandar Setiadi (freedomofkeima)

## Development Environment

- Linux/UNIX based Operating System

- C++ version 4.7.2 (g++ compiler)

- Python version 2.7

- LevelDB 1.15.0

- Apache Thrift 0.9.2

- MongoDB 3.0.1 & PyMongo 3.0 (for benchmarking)

## Requirements (Prototype)

** LevelDB **

1. Install LevelDB

2. Put your levelDB dependency at ```Makefile```. For example, if your levelDB is located at ```~/leveldb-1.15.0```, put it as ```LEVELDB = ~/leveldb-1.15.0```


** Apache Thrift **

1. Install C++ Boost Library (boost-devel in CentOS / libboost-all-dev in Ubuntu ver 1.54 (raring) and up)

2. Install Apache Thrift (follow the tutorial at http://thrift-tutorial.readthedocs.org/en/latest/installation.html)

## How to Run

** Prototype **

- Run the ```Makefile``` by running ```make all``` command.

- Set your distributed database environments by specifying all machines at ```db.config```. You should set ```own``` as ```true``` for the specified machine's address. The value of ```replicationFactors``` should be smaller than or equal to the number of running nodes.

- To run the prototype server, you could simply use ```make run``` or ```bin/server```. You could also use ```bin/testcase``` which provides several testcases.


** Benchmark **

- To run the benchmarking for LevelDB, you could simply use ```bin/local_testcase```.

- To run the benchmarking for MongoDB, you could simply use ```python mongodb/mongodb_testcase.py```. See http://docs.mongodb.org/manual/tutorial/deploy-replica-set to have a better understanding in deploying multiple nodes with MongoDB.

## Additional Information

The benchmarking for TippyDB uses RDTSC & RDTSCP for measuring average running time. ( Reference: https://idea.popcount.org/2013-01-28-counting-cycles---rdtsc/ )

Make sure to "warm up" the code before benchmarking. (avoid cache effects in the first iteration)

---
#### License

Copyright (c) 2015 Iskandar Setiadi <13511073@std.stei.itb.ac.id>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Last Updated: April 10, 2015

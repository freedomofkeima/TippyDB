# TippyDB

TippyDB is a geographically-aware distributed NoSQL written in C++. This NoSQL database is designed to store data at the nearest location with the data's writer. One of the main advantage is to provide lower latency in average, which is based on a scientific research (Backstrom, Sun, & Marlow, 2010) that the location of data's reader is usually located near the original writer (70% in 100 miles range, 80% in 500 miles range).

It should be noted that TippyDB is not intended for production use. TippyDB is intended to show that geo-aware distributed NoSQL is feasible to be implemented and it has several advantages such as lower average latencies, etc.

The paper for the detailed system's design will be informed in the future time.

By: Iskandar Setiadi - freedomofkeima (iskandarsetiadi@gmail.com)

## Development Environment

- Linux/UNIX based Operating System

- C++ version 4.7.2 (g++ compiler, with C++11 support)

- Python version 2.7

- LevelDB 1.15.0

- Apache Thrift 0.9.2

- MongoDB 3.0.1 & PyMongo 3.0 (for benchmarking)

## Requirements (Prototype)

** LevelDB **

1. Install LevelDB

2. Put your levelDB dependency at the [Makefile](Makefile). For example, if your levelDB is located at ```~/leveldb-1.15.0```, put it as ```LEVELDB = ~/leveldb-1.15.0```


** Apache Thrift **

1. Install C++ Boost Library (boost-devel in CentOS / libboost-all-dev in Ubuntu ver 1.54 (raring) and up)

2. Install Apache Thrift (follow the tutorial at http://thrift-tutorial.readthedocs.org/en/latest/installation.html)

## How to Run

** Prototype **

- Run the [Makefile](Makefile) by running ```make all``` command.

- Set your distributed database environments by specifying all machines at [db.config](db.config). You should set ```own``` as ```true``` for the specified machine's address. The value of ```replicationFactors``` should be smaller than or equal to the number of running nodes. Caution: Do not change the orderings of the existing members. For example:

```
{
	"id": "set1",
	"numberNodes": 2,
	"replicationFactors": 2,
	"shardSize": 32,
	"distance": [[0, 1], [1, 0]],
	"members": [
		{
			"region": 1,
			"node": 1,
			"ip": "127.0.0.1",
			"port": 9090,
			"own": true
		},
		{
			"region": 2,
			"node": 1,
			"ip": "127.0.0.1",
			"port": 9091,
			"own": false
		}
	]
}
```

- To run the prototype server, you could simply use ```./application_name port_number db_path```, such as ```bin/server 9090 /tmp/testdb```. You could also use ```./application_name ip_address port_number```, such as ```bin/testcase 127.0.0.1 9090``` and ```bin/testcase2 127.0.0.1 9090``` which provides several testcases.


** Benchmark **

- To run the benchmarking for LevelDB, you could simply use ```./application_name db_path```, such as ```bin/local_testcase /tmp/testdb```.

- To run the benchmarking for MongoDB, you could simply use ```python mongodb/mongodb_testcase.py``` and ```python mongodb/mongodb_testcase2.py```. See http://docs.mongodb.org/manual/tutorial/deploy-replica-set to have a better understanding in deploying multiple nodes with MongoDB. It is also recommended to read http://docs.mongodb.org/manual/tutorial/deploy-shard-cluster/ in order to deploy sharded clusters.

## Performance

### First Experiment

In this experiment, we measure the average latency from TippyDB in 2 access-point deployment, Singapore and N. Virginia (Amazon EC2), from the request of remote client. We use 2 replicas in Singapore and 1 replica in N. Virginia. Each experiment is done with 80% write operation to Singapore (since it's the closest one with the writer's location) and 20% write operation to N. Virginia. There are two kinds of operation type: ```Stable``` and ```1 Replica Failed```. ```1 Replica Failed``` means that one of the nodes in Singapore is down.

**Insert**

|Type             |Data Size (KB)|Total Operations|TippyDB Performance (msec/op)|MongoDB Performance (msec/op)|
|:---------------:|:------------:|:--------------:|:---------------------------:|:---------------------------:|
|Stable           |100           |2000            |127.58                       |268.48                       |
|Stable           |1000          |200             |154.27                       |455.83                       |
|1 Replica Failed |100           |2000            |159.92                       |274.01                       |
|1 Replica Failed |1000          |200             |195.39                       |477.91                       |

We notice that MongoDB writes is quite time-consuming and proportional to the data size. We believe that we need to optimize sharding for chunk with large files in order to achieve better performance. On the other hand, TippyDB always writes on the nearest node, which is twice faster than the balanced operation in MongoDB. From this experiment, it can be concluded that we need to deploy more active servers in busy area, and we can take benefits from TippyDB's latency reduction, compared to the widespread-balanced operation in MongoDB and several other NoSQLs.

Under ```1 Replica Failed```, TippyDB needs ~60 seconds to do automatic failover and the overall performance will be slower for those ~60 seconds interval.

**Read**

|Type             |Data Size (KB)|Total Operations|TippyDB Performance (msec/op)|MongoDB Performance (msec/op)|
|:---------------:|:------------:|:--------------:|:---------------------------:|:---------------------------:|
|Stable           |100           |2000            |191.79                       |225.65                       |
|Stable           |1000          |200             |331.14                       |374.73                       |
|1 Replica Failed |100           |2000            |223.51                       |243.32                       |
|1 Replica Failed |1000          |200             |378.32                       |443.18                       |

**Update**

|Type             |Data Size (KB)|Total Operations|TippyDB Performance (msec/op)|MongoDB Performance (msec/op)|
|:---------------:|:------------:|:--------------:|:---------------------------:|:---------------------------:|
|Stable           |100           |2000            |193.54                       |251.58                       |
|Stable           |1000          |200             |330.41                       |394.31                       |
|1 Replica Failed |100           |2000            |241.75                       |249.61                       |
|1 Replica Failed |1000          |200             |395.21                       |481.28                       |

**Delete**

|Type             |Data Size (KB)|Total Operations|TippyDB Performance (msec/op)|MongoDB Performance (msec/op)|
|:---------------:|:------------:|:--------------:|:---------------------------:|:---------------------------:|
|Stable           |100           |2000            |102.55                       |205.71                       |
|Stable           |1000          |200             |102.86                       |194.10                       |
|1 Replica Failed |100           |2000            |127.45                       |230.85                       |
|1 Replica Failed |1000          |200             |127.91                       |226.89                       |

From the first experiment, we can simply conclude that TippyDB will support the idea that the location of data's reader is located near the original writer (80% near, 20% far), while MongoDB writes without considering users' location (50% near, 50% far).

### Second Experiment

In this experiment, we will compare the basic performance of TippyDB and MongoDB. Each operation is done 100,000 times with 100 bytes data.

|Operation|TippyDB Performance (usec/op)|MongoDB Performance (usec/op)|
|:-------:|:---------------------------:|:---------------------------:|
|Fill     |603.97                       |224.15                       |
|Update   |566.22                       |259.17                       |
|Read     |479.97                       |197.99                       |
|Delete   |553.04                       |223.19                       |

TippyDB is built with the support of LevelDB and Apache Thrift. From our experimentation, TippyDB is fast since it is only contributed to 1-5% of total time. However, Apache Thrift (RPC) is quite costly, which an empty RPC needs 466 usec/op in average (~80% of total time).

### Third Experiment

In the third experiment, we will use several variations of ```insert : read``` ratio in measuring TippyDB and MongoDB performances from the request of remote client. This test uses the same environment with the first one. Each experiment is done with 10,000 requests and 1,024 bytes data.

|Insert Ratio|Read Ratio|TippyDB Performance (msec/op)|MongoDB Performance (msec/op)|
|:----------:|:--------:|:---------------------------:|:---------------------------:|
|1%          |99%       |92.97                        |202.41                       |
|5%          |95%       |90.46                        |209.76                       |
|15%         |85%       |80.22                        |213.44                       |
|25%         |75%       |76.81                        |225.12                       |
|50%         |50%       |70.21                        |246.79                       |

TippyDB is faster in handling insert operation since TippyDB will always execute this operation in the nearest node with the client. On the other hand, MongoDB will handle insert operation in a balanced manner. Also, TippyDB and MongoDB have comparable performance in handling read operation. 

## Conclusion

If your system is capable in handling huge quantity of users' requests (example: a lot of nodes in busy area), TippyDB offers better performance to your clients with its latency reduction. We simply want to show that geo-aware NoSQL is possible and it can offer several advantages despite of its drawbacks. Nevertheless, we believe that there are a lot of rooms for future improvement in this area.

## Acknowledgement

The writer feels thankful to Mr. Achmad Imam Kistijantoro as writer's supervisor in this final thesis.

## Additional Information

The benchmarking for TippyDB uses RDTSC & RDTSCP for measuring average running time. (Reference: https://idea.popcount.org/2013-01-28-counting-cycles---rdtsc/)

Make sure to "warm up" the code before benchmarking. (avoid cache effects in the first iteration)

---
#### License

Copyright (c) 2015 Iskandar Setiadi <13511073@std.stei.itb.ac.id>

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Last Updated: May 19, 2015

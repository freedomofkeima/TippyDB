/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * DBServiceServer.cpp
  *
  */

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/TToString.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>
#include <thread>
#include <mutex>

#include "./gen-cpp/DBService.h"
#include "./db/database.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using boost::shared_ptr;

using namespace rapidjson;

using namespace dbservice;

/** Own configuration */
int replication_factors = 1; // default (own)
int shard_size = 32; // default (in MB)

string global_id;
string server_ip;
int server_port;
int server_region;
int server_node;
int own_id;

struct Member {
  int region;
  int node;
  string ip;
  int port;
  long long lclock;
  vector<int> distance;
};

vector<Member> members;

/***** MEMBERSHIP SECTION *****/

void printMembers() {
  int size = (int) members.size();
  for (int i = 0; i < size; i++) {
      cout << "Member #" << (i+1) << ": " << endl;
      cout << "Region: " << members[i].region << endl;
      cout << "Node: " << members[i].node << endl;
      cout << "IP: " << members[i].ip << endl;
      cout << "Port: " << members[i].port << endl;
      cout << "Logical Clock: " << members[i].lclock << endl;
      cout << "Distance: ";
      int size2 = (int) members[i].distance.size();
      for (int j = 0; j < size2; j++) {
          cout << members[i].distance[j];
          if (j != size2 - 1) cout << ", ";
      }
      cout << endl;
  }
}

void loadMembers() {
  /** Read json from file */
  ifstream t("db.config");
  string str;

  t.seekg(0, ios::end);
  str.reserve(t.tellg());
  t.seekg(0, ios::beg);

  str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());

  // Value& s = d["stars"];
  // s.SetInt(s.GetInt() + 1);

  /** Convert string to json */
  const char* json = str.c_str();
  Document d;
  d.Parse(json);

  global_id = d["id"].GetString();
  uint32_t num_members = d["numberRegions"].GetInt();
  replication_factors = (int) d["replicationFactors"].GetInt();
  shard_size = (int) d["shardSize"].GetInt();
  const Value& distances = d["distance"];

  for (SizeType i = 0; i < num_members; i++) {
      Member m;
      m.lclock = 0; // initialize (we don't need to know the initial logical clock)
      const Value& distance = distances[i];
      for (SizeType j = 0; j < num_members; j++) {
           m.distance.push_back(distance[SizeType(j)].GetInt());
      }
      members.push_back(m); // add new member
  }

  const Value& ms = d["members"];
  for (SizeType i = 0; i < num_members; i++) {
      const Value& info = ms[i];
      members[i].region = (int) info["region"].GetInt();
      members[i].node = (int) info["node"].GetInt();
      members[i].ip = info["ip"].GetString();
      members[i].port = info["port"].GetInt();

     // add additional info for self
     if (info["own"].GetBool()) {
         own_id = (int) i;
         server_ip = members[i].ip;
         server_port = members[i].port;
         server_region = members[i].region;
         server_node = members[i].node;
     }
  }

  // TODO: Init metadata if metadata_counter = 0 and there's no response from other members, else ask for newest version from all available server and compare metadata_counter value

  // StringBuffer buffer;
  // Writer<StringBuffer> writer(buffer);
  // d.Accept(writer);
  // cout << buffer.GetString() << endl;
}

/***** END OF MEMBERSHIP SECTION *****/

/***** LOGICAL CLOCK SECTION *****/

mutex logical_mutex;

class LogicalClock {
public:
  LogicalClock() {
    logical_clock = getLClock();
  	cout << "Logical clock: " << logical_clock << endl;
  }

  long long incrementLClock() {
    logical_clock++;
    members[own_id].lclock = logical_clock;
    return logical_clock;
  }

private:
  long long logical_clock = 0; // for timestamping
};

LogicalClock* lClock;

/***** END OF LOGICAL CLOCK SECTION *****/

/***** SECONDARY SECTION *****/

void failureTask(const int32_t remote_region, const int32_t remote_node) {

}

void replicateTask(const std::string& key, const std::string& value, long long ts) {
	// TODO: Handle failure (try for 10, 20, ..., 60 seconds before declaring failure)
	cout << "Replicate task: " << key << " " << value << " (ts : " << ts << ")" << endl;
}

void updateTask(const std::string& key, const std::string& value, long long ts) {
	// TODO: Handle failure (try for 10, 20, ..., 60 seconds before declaring failure)
	cout << "Update task: " << key << " " << value << " (ts : " << ts << ")" << endl;
}

/***** END OF SECONDARY SECTION *****/

/***** RPC THRIFT SECTION *****/

class DBServiceHandler : virtual public DBServiceIf {
 public:
  DBServiceHandler() {}

  void ping() {
    // Do nothing
  }

  void putData(std::string& _return, const std::string& value) {
    _return = putDB(value, server_region, server_node, false);
	if (_return.length() == 16) {
		// replicate data to secondary nodes
		logical_mutex.lock();
		long long clock = lClock->incrementLClock();
		logical_mutex.unlock();
		putLClock(clock);
		thread t(replicateTask, _return, value, clock);
		t.detach();
	} else {
		// shard limit, check other partitions

		// if exist, send putDataForce

		// if doesn't exist, force to putDB
	}
  }

  /**
   * putDataForce
   * Write a new data by force (due to partition limitation)
   * 
   * @param value
   */
  void putDataForce(std::string& _return, const std::string& value) {
    _return = putDB(value, server_region, server_node, true); // return by force
	if (_return.length() == 16) {
		// replicate data to secondary nodes
		logical_mutex.lock();
		long long clock = lClock->incrementLClock();
		logical_mutex.unlock();
		putLClock(clock);
		thread t(replicateTask, _return, value, clock);
		t.detach();
	}
  }

  bool updateData(const Data& d) {
    bool isSuccess = updateDB(d.key, d.value);
	if (isSuccess) {
		logical_mutex.lock();
		long long clock = lClock->incrementLClock();
		logical_mutex.unlock();
		putLClock(clock);
		thread t(updateTask, d.key, d.value, clock);
		t.detach();
	}
    return isSuccess;
  }

  /**
   * updateSecondaryData
   * Propagate latest data to secondary nodes where region = remote_region && node == remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool updateSecondaryData(const Data& d, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    return true;
  }

  void getData(std::string& _return, const std::string& sharded_key) {
    // Your implementation goes here
    printf("getData\n");
  }

  bool deleteData(const std::string& sharded_key) {
    // Your implementation goes here
    return true;
  }

  /**
   * deleteSecondaryData
   * Remove data from secondary nodes where region = remote_region && node == remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool deleteSecondaryData(const Data& d, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    return true;
  }

  /**
   * replicateData
   * Replicate a new data from primary to secondary where region = remote_region && node = remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool replicateData(const Data& d, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    return true;
  }

  /**
   * resyncData
   * Retrieve all newest shard contents where region = remote_region && node = remote_node
   * 
   * @param remote_region
   * @param remote_node
   */
  void resyncData(ShardContent& _return, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    printf("resyncData\n");
  }

  void zip() {
    // Do nothing
  }

};

/***** END OF RPC THRIFT SECTION *****/

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage: ./application_name port_number db_path" << endl;
    return 1;
  }
  cout << "Port number: " << argv[1] << endl;
  cout << "DB path: " << argv[2] << endl;

  int port = atoi(argv[1]);
  boost::shared_ptr<DBServiceHandler> handler(new DBServiceHandler());
  boost::shared_ptr<TProcessor> processor(new DBServiceProcessor(handler));
  boost::shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  boost::shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  boost::shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

  /** Create Threaded server */
  TThreadedServer server(processor,
                         serverTransport,
                         transportFactory,
                         protocolFactory);

  // Load members configuration
  loadMembers();
  printMembers();

  // Create one thread for each member

  // Test leveldb
  initDB(argv[2], shard_size);
  test();

  // Load logical clock
  lClock = new LogicalClock();

  cout << "** Starting the server **" << endl;
  server.serve();
  cout << "** Done **" << endl;

  return 0;
}


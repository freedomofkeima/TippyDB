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
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
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
#include <chrono>

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
  vector<int> distance;
  int active; // 0 = active, 1 = dead suspect, 2 = dead
};

struct SKey {
  string id;
  pair<int, int> primary;
  vector< pair<int, int> > secondary;
};

vector<Member> members;
map<string, int> member_pos;
map<string, SKey> skeys;

/***** MEMBERSHIP SECTION *****/

void printMembers() {
  int size = (int) members.size();
  for (int i = 0; i < size; i++) {
      cout << "Member #" << (i+1) << ": " << endl;
      cout << "Region: " << members[i].region << endl;
      cout << "Node: " << members[i].node << endl;
      cout << "IP: " << members[i].ip << endl;
      cout << "Port: " << members[i].port << endl;
      cout << "Distance: ";
      int size2 = (int) members[i].distance.size();
      for (int j = 0; j < size2; j++) {
          cout << members[i].distance[j];
          if (j != size2 - 1) cout << ", ";
      }
      cout << endl << endl;
  }
}

void printSKeys() {
  map<string, SKey>::iterator iter;
  for (iter = skeys.begin(); iter != skeys.end(); iter++) {
      cout << "Sharded Key ID: " << iter->first << endl;
      cout << "Primary (Region, Node): (" << iter->second.primary.first << ", " << iter->second.primary.second << ")" << endl;
      int sec_size = (int) iter->second.secondary.size();
      for (int j = 0; j < sec_size; j++) {
        cout << "-- Secondary (Region, Node): (" << iter->second.secondary[j].first << ", " << iter->second.secondary[j].second << ")" << endl;
      }
  }
  cout << endl;
}

void loadMembers() {
  /** Read json from file */
  ifstream t("db.config");
  string str;

  t.seekg(0, ios::end);
  str.reserve(t.tellg());
  t.seekg(0, ios::beg);

  str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
  t.close();

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
      members[i].active = 0; // Initially, dim all as active
     // add additional info for self
     if (info["own"].GetBool()) {
         own_id = (int) i;
         server_ip = members[i].ip;
         server_port = members[i].port;
         server_region = members[i].region;
         server_node = members[i].node;
     }
     // construct map
      member_pos[constructShardKey(members[i].region, members[i].node)] = i;
  }

  // StringBuffer buffer;
  // Writer<StringBuffer> writer(buffer);
  // d.Accept(writer);
  // cout << buffer.GetString() << endl;
}

// Lock with mutex before updating
void loadSKeys() {
  // TODO: Consensus for latest version of metadata, if exists

  // TODO: Check with local version, if it's lesser, retrieve from the highest one

  // TODO: Else, read from local json files if version != -1

  // Read from metadata.tmp
  ifstream t("data/metadata.tmp");
  string str;

  t.seekg(0, ios::end);
  str.reserve(t.tellg());
  t.seekg(0, ios::beg);

  str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
  t.close();

  /** Convert string to json */
  const char* json = str.c_str();
  Document d;
  d.Parse(json);

  const Value& shardedkeys = d["shardedkeys"];
  assert(shardedkeys.IsArray());
  // Iterate over all values in shardedkeys
  for (SizeType i = 0; i < shardedkeys.Size(); i++) {
      SKey sk;
      const Value& data = shardedkeys[i];
      sk.id = data["id"].GetString();
      const Value& primary = data["primary"];
      sk.primary.first = (int) primary[0]["region"].GetInt();
      sk.primary.second = (int) primary[0]["node"].GetInt();
      const Value& secondary = data["secondary"];
      for (SizeType j = 0; j < secondary.Size(); j++) {
          pair<int, int> secval;
          const Value& data2 = secondary[j];
          secval.first = (int) data2["region"].GetInt();
          secval.second = (int) data2["node"].GetInt();
          sk.secondary.push_back(secval);
      }
     skeys[sk.id] = sk;
  }

}

/***** END OF MEMBERSHIP SECTION *****/

/***** LOGICAL CLOCK SECTION *****/

mutex logical_mutex;

class LogicalClock {
public:
  LogicalClock() {
  }

  long long incrementLClock(const std::string& key) {
    mutex ts_mutex;
    ts_mutex.lock();
    long long c = getLClock(key);
    c++;
    putLClock(key, c);
    ts_mutex.unlock();
    return c;
  }

  bool tsCheck(const std::string& key, int64_t ts) {
    if (getLClock(key) < (long long) ts) return true;
    else return false;
  }

private:

};

LogicalClock* lClock;

/***** END OF LOGICAL CLOCK SECTION *****/

/***** SECONDARY SECTION *****/

void failureTask(const int32_t remote_region, const int32_t remote_node) {

}

void replicateTask(const std::string& key, const std::string& value, long long ts) {
	// TODO: Handle failure (try for 10, 20, 30 seconds before declaring failure)
	int timer = 0; // initial timer, in seconds
	Data d;
	d.key = key;
	d.value = value;

	cout << "Replicate task: " << key << " " << value << " (ts : " << ts << ")" << endl;
	string identifier = d.key.substr(0, 8); // first 8 characters
	vector< pair<int, int> > secondary = skeys[identifier].secondary;
	bool* isSent;
	isSent = (bool*) malloc ((int) secondary.size() * sizeof(bool));
	while (timer <= 30) {
		timer += 10;
		for(int i = 0; i < (int) secondary.size(); i++) {
			if (!isSent[i]) {
				int idx = member_pos[constructShardKey(secondary[i].first, secondary[i].second)];
				boost::shared_ptr<TTransport> socket(new TSocket(members[idx].ip, members[idx].port));
				cout << "Replicate " << d.key << " to " << members[idx].ip << ":" << members[idx].port;
				if (timer <= 30) cout << " (After Timeout: " << timer <<  " second(s))" << endl;
				else cout << endl;
				boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
				boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
				DBServiceClient client(protocol);

				try {
					transport->open();
					isSent[i] = client.replicateData(d, server_region, server_node, ts); // RPC Replicate
				} catch (TException& tx) {
					this_thread::sleep_for(chrono::seconds(1));
					cout << "ERROR: " << tx.what() << endl;
				}
			}
		}
		bool globalSent = true;
		for (int i = 0; i < (int) secondary.size(); i++)
			if (!isSent[i]) globalSent = false;
		if (globalSent) break;
		else {
			// Sleep for timer seconds here
			if (timer <= 30) this_thread::sleep_for(chrono::seconds(timer));
		}
	}
	for (int i = 0; i < (int) secondary.size(); i++)
		if (!isSent[i]) failureTask(secondary[i].first, secondary[i].second);
}

void updateTask(const std::string& key, const std::string& value, long long ts) {
	// TODO: Handle failure (try for 10, 20, 30 seconds before declaring failure)
	cout << "Update task: " << key << " " << value << " (ts : " << ts << ")" << endl;
}

void deleteTask(const std::string& key) {
	// TODO: Handle failure (try for 10, 20, 30 seconds before declaring failure)
	cout << "Delete task: " << key << endl;
}

/***** END OF SECONDARY SECTION *****/

/***** RPC THRIFT SECTION *****/

class DBServiceHandler : virtual public DBServiceIf {
 public:
  DBServiceHandler() {}

  void ping() {
    // Do nothing
  }

  // First come first serve basis
  void putData(std::string& _return, const std::string& value) {
    _return = putDB(value, server_region, server_node, false);
	if (_return.length() == 16) {
		// replicate data to secondary nodes
		logical_mutex.lock();
		long long clock = lClock->incrementLClock(_return);
		logical_mutex.unlock();
		putLClock(_return, clock);
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
  void putDataForce(std::string& _return, const std::string& value, const int32_t remote_region, const int32_t remote_node) {
    _return = putDB(value, server_region, server_node, true); // return by force
	if (_return.length() == 16) {
		// replicate data to secondary nodes
		logical_mutex.lock();
		long long clock = lClock->incrementLClock(_return);
		logical_mutex.unlock();
		thread t(replicateTask, _return, value, clock);
		t.detach();
	}
  }

  // First come first serve basis
  bool updateData(const Data& d) {
    pair<int, int> location = parse_key(d.key);
    if (location.first == server_region && location.second == server_node) { // TODO : Change to "primary" flag from metadata
    bool isSuccess = updateDB(d.key, d.value);
		if (isSuccess) {
			logical_mutex.lock();
			long long clock = lClock->incrementLClock(d.key);
			logical_mutex.unlock();
			thread t(updateTask, d.key, d.value, clock);
			t.detach();
		}
    	return isSuccess;
    } else {
			// search at specified region and node
			for (int i = 0; i < (int) members.size(); i++) {
				if (location.first == members[i].region && location.second == members[i].node) { // TODO : Change to "primary" flag from metadata
					boost::shared_ptr<TTransport> socket(new TSocket(members[i].ip, members[i].port));
					boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
					boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
					DBServiceClient client(protocol);

					try {
						transport->open();
					    return client.updateData(d);	
					} catch (TException& tx) {
 					   cout << "ERROR: " << tx.what() << endl;
 					   return false;
					}

				}
			}
    }
    return false;
  }

  /**
   * updateSecondaryData
   * Propagate latest data to secondary nodes where region = remote_region && node == remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool updateSecondaryData(const Data& d, const int32_t remote_region, const int32_t remote_node, const int64_t ts) {
    // check whether logical clock 'ts' is higher (ts > lclock)
    return true;
  }

  // First come first serve basis (return empty string if sharded_key != exists)
  void getData(std::string& _return, const std::string& sharded_key) {
    pair<int, int> location = parse_key(sharded_key);
    if (location.first == server_region && location.second == server_node) { // TODO : Change to "primary" flag from metadata
		_return = getDB(sharded_key);
    } else {
			// search at specified region and node
			for (int i = 0; i < (int) members.size(); i++) {
				if (location.first == members[i].region && location.second == members[i].node) { // TODO : Change to "primary" flag from metadata
					boost::shared_ptr<TTransport> socket(new TSocket(members[i].ip, members[i].port));
					boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
					boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
					DBServiceClient client(protocol);

					try {
						transport->open();
					    client.getData(_return, sharded_key);	
					} catch (TException& tx) {
 					   cout << "ERROR: " << tx.what() << endl;
					}

				}
			}
    }
  }

  // First come first serve basis
  bool deleteData(const std::string& sharded_key) {
    pair<int, int> location = parse_key(sharded_key);
    if (location.first == server_region && location.second == server_node) { // TODO : Change to "primary" flag from metadata
		bool isSuccess = deleteDB(sharded_key);
		if (isSuccess) {
			thread t(deleteTask, sharded_key);
			t.detach();
		}
		return isSuccess;
    } else {
			// search at specified region and node
			for (int i = 0; i < (int) members.size(); i++) {
				if (location.first == members[i].region && location.second == members[i].node) { // TODO : Change to "primary" flag from metadata
					boost::shared_ptr<TTransport> socket(new TSocket(members[i].ip, members[i].port));
					boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
					boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
					DBServiceClient client(protocol);

					try {
						transport->open();
					    return client.deleteData(sharded_key);	
					} catch (TException& tx) {
 					   cout << "ERROR: " << tx.what() << endl;
					}

				}
			}
    }
    return false;
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
  bool replicateData(const Data& d, const int32_t remote_region, const int32_t remote_node, const int64_t ts) {
	cout << "replicateData is called" << endl;
    return true;
  }

  /**
   * resyncData
   * Retrieve all newest shard contents where region = remote_region && node = remote_node (choose the nearest one for primary / the smallest db size for secondary)
   * 
   * @param remote_region
   * @param remote_node
   */
  void resyncData(ShardContent& _return, const int32_t remote_region, const int32_t remote_node, const int64_t ts) {
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

  // Load members configuration (db.config)
  loadMembers();
  printMembers();
  // Load shared keys configuration (metadata.tmp)
  loadSKeys();
  printSKeys();

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


/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * DBServiceServer.h
  *
  */
#ifndef DBSERVICESERVER_H
#define DBSERVICESERVER_H

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/server/TThreadedServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <thrift/TToString.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filewritestream.h"

#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <queue>
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
using namespace boost::posix_time;
using namespace rapidjson;
using namespace dbservice;

struct Member {
  int region;
  int node;
  string ip;
  int port;
  vector<int> distance;
  int active; // 1 = active, 0 = fail
};

struct SKey {
  string id;
  pair<int, int> primary;
  vector< pair<int, int> > secondary;
};

/***** LOG WRITER DEFINITION *****/
class LogWriter {
public:
  LogWriter();

  void writeLog(const std::string& message);

private:
  string file;
  time_facet* facet;
  mutex log_mutex;
};

/***** END OF LOG WRITER DEFINITION *****/

/***** RAFT DEFINITION *****/
class RaftConsensus {
public:
  RaftConsensus(vector<Member> _members, string _metadata, int _own_id);

  void initElection();

  bool commit(const std::string entry, int _commit_idx);

  // Send vote request
  bool leaderRequest();

  // Send append request
  bool appendRequest(const std::string& entry);

  void checkFailure(int remote_region, int remote_node);

  /** Getter & Setter */
  int getTerm();

  void setTerm(int _term);

  string getLog();

  int getVotedFor();

  void setVotedFor(int _voted_for);

  int getCommitIdx();

  int getNodeId();

private:
  int current_term; // server's current term (initial = 0)
  string log; // newest metadata
  int voted_for; // the candidate the server voted for (initial = 0)
  int commit_idx; // highest log entry known to be committed
  vector<int> votes_for_me; // who has voted for me
  vector<Member> nodes; // all consensus members
  int num_nodes; // number of nodes
  int timeout_elapsed; // random timeout, in miliseconds
  int node_id; // my node ID
};

/***** END OF RAFT DEFINITION *****/

/***** LOGICAL CLOCK DEFINITION *****/
class LogicalClock {
public:
  LogicalClock();

  long long incrementLClock(const std::string& key);

  bool tsCheck(const std::string& key, int64_t ts);

private:
  mutex ts_mutex;
};

/***** END OF LOGICAL CLOCK DEFINITION *****/

/***** RPC THRIFT DEFINITION *****/
class DBServiceHandler : virtual public DBServiceIf {
 public:
  DBServiceHandler();

  void ping();

  // First come first serve basis
  void putData(std::string& _return, const std::string& value);

  /**
   * putDataForce
   * Write a new data by force (due to partition limitation)
   * 
   * @param value
   */
  void putDataForce(std::string& _return, const std::string& value, const int32_t remote_region, const int32_t remote_node);

  // First come first serve basis
  bool updateData(const Data& d);

  /**
   * updateSecondaryData
   * Propagate latest data to secondary nodes where region = remote_region && node == remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool updateSecondaryData(const Data& d, const int32_t remote_region, const int32_t remote_node);

  // First come first serve basis (return empty string if sharded_key != exists)
  void getData(std::string& _return, const std::string& sharded_key);

  // First come first serve basis
  bool deleteData(const std::string& sharded_key);

  /**
   * deleteSecondaryData
   * Remove data from secondary nodes where region = remote_region && node == remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool deleteSecondaryData(const std::string& sharded_key, const int32_t remote_region, const int32_t remote_node);

  /**
   * replicateData
   * Replicate a new data from primary to secondary where region = remote_region && node = remote_node
   * 
   * @param d
   * @param remote_region
   * @param remote_node
   */
  bool replicateData(const Data& d, const int32_t remote_region, const int32_t remote_node);

  /**
   * resyncData
   * Retrieve all newest shard contents where region = remote_region && node = remote_node
   * 
   * @param remote_region
   * @param remote_node
   */
  void resyncData(ShardContent& _return, const int32_t remote_region, const int32_t remote_node);

  /**
   * pushResyncData
   * Push ShardContent from primary node to other node
   * 
   * @param contents
   */
  bool pushResyncData(const ShardContent& contents);

  /**
   * getRecover
   * Get newest metadata (recovery phase)
   */
  void getRecover(GetRecover& _return);

  /**
   * sendAppend
   * Send append request -> Update metadata (consensus). On the other hand, lock metadata from other R/W operation
   * 
   * @param request
   */
  void sendAppend(AppendResponse& _return, const AppendRequest& request);

  /**
   * sendVote
   * Send vote request
   * 
   * @param request
   */
  void sendVote(VoteResponse& _return, const VoteRequest& request);

  /**
   * followerAppend
   * Append newest committed metadata at follower
   * 
   * @param request
   */
  bool followerAppend(const AppendRequest& request);

  void zip();

};

/***** END OF RPC THRIFT DEFINITION *****/

// Decode JSON to skeys
void decodeKeys(map<string, SKey> &ret, const char* json);

// Write JSON to file
void writeMetadata(const std::string& m);

string convertShardedMaptoJSON(map<string, SKey> m);

// Create a metadata
string createMetadata();

// Push resync data task
bool pushResyncTask(ShardContent ds, const std::string ip, int port);

// Tweak current metadata
string tweakMetadata(const std::string& m, int idx, int remote_region, int remote_node);

// Comparator for priority queue
struct compare {
	bool operator()(const pair<int, int>& l, const pair<int, int>& r) {
			return l.second > r.second;
	}
};

// Check metadata update on node failure
void checkMetadataChange(const std::string prev_entry, const std::string next_entry);

/***** MEMBERSHIP SECTION *****/

void printMembers();

void printSKeys();

void loadMembers();

void loadSKeys();

/***** END OF MEMBERSHIP SECTION *****/

/***** SECONDARY SECTION *****/

void failureTask(int remote_region, int remote_node);

/**
  * code 0: replicateTask
  * code 1: updateTask
  * code 2: deleteTask
  */
void backgroundTask(const std::string& key, const std::string& value, long long ts, int code);

void broadcastMetadata(const std::string& entry, int commit_idx, int term);

/***** END OF SECONDARY SECTION *****/


#endif


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
  int active; // 1 = active, 0 = fail
};

struct SKey {
  string id;
  pair<int, int> primary;
  vector< pair<int, int> > secondary;
};

vector<Member> members;
map<string, int> member_pos;
map<string, SKey> skeys;
string identity;
string metadata;

// Decode JSON to skeys
void decodeKeys(map<string, SKey> &ret, const char* json) {
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

      // Update failure detection information
      int idx = member_pos[constructShardKey(sk.primary.first, sk.primary.second)];
      members[idx].active = 1;

      const Value& secondary = data["secondary"];
      for (SizeType j = 0; j < secondary.Size(); j++) {
          pair<int, int> secval;
          const Value& data2 = secondary[j];
          secval.first = (int) data2["region"].GetInt();
          secval.second = (int) data2["node"].GetInt();
          sk.secondary.push_back(secval);
      }
     ret[sk.id] = sk;
  }
}

// Write JSON to file
void writeMetadata(const std::string& m) {
  metadata = m;
  const char* json = metadata.c_str();
  decodeKeys(skeys, json); // update volatile memory

  Document d;
  d.Parse(json);

  FILE* fp = fopen("data/metadata.tmp", "w");

  char* writeBuffer;
  writeBuffer = (char*) malloc((m.length() + 1) * sizeof(char));
  FileWriteStream os (fp, writeBuffer, sizeof(writeBuffer));

  Writer<FileWriteStream> writer(os);
  d.Accept(writer);

  fclose(fp);
}

string convertShardedMaptoJSON(map<string, SKey> m) {
  string ret;
  map<string,SKey>::iterator iter;
  ret = "{\"shardedkeys\":[";
  int ctx = 0;
  for (iter = m.begin(); iter != m.end(); iter++) {
      if (ctx != 0) ret = ret + ",";
      ctx++;
      ret = ret + "{";
      // Insert ID
      ret = ret + "\"id\":\"" + iter->first + "\",";
      // Insert Primary
      ret = ret + "\"primary\":[{\"region\":" + to_string(iter->second.primary.first) + ",\"node\":" + to_string(iter->second.primary.second) + "}],";
      // Insert Secondary
      ret = ret + "\"secondary\":[";
      int sec_size = (int) iter->second.secondary.size();
      for (int j = 0; j < sec_size; j++) {
        ret = ret + "{\"region\":" + to_string(iter->second.secondary[j].first) + ",\"node\":" + to_string(iter->second.secondary[j].second) + "}";
        if (j != sec_size - 1) ret = ret + ",";
      }
      ret = ret + "]}";
  }
  ret = ret + "]}";
  return ret;
}

// Create a metadata
string createMetadata() {
  int size = replication_factors;
  map<string, SKey> temp_ret;
  vector<int> active_members;
  string ret;
  for (int i = 0; i < (int) members.size(); i++)
      if (members[i].active == 1) active_members.push_back(i);
  if ((int) active_members.size() - 1 < size) size = (int) active_members.size() - 1; // allow replication which is lower than specified if there is not enough number of nodes

  for (int i = 0; i < (int) active_members.size(); i++) {
      int idx = active_members[i];
      if (members[idx].active == 1) {
		  SKey sk;
		  sk.id = constructShardKey(members[idx].region, members[idx].node);
		  sk.primary.first = members[idx].region;
		  sk.primary.second = members[idx].node;
		  int next_id = i;
		  for (int j = 0; j < size; j++) { // circular secondaries (can be optimized later, by specifying secondary in exact different region)
		  	pair<int, int> secval;
		  	next_id++; // increment
		  	if (next_id >= (int) active_members.size()) next_id = next_id - active_members.size();
		  	int next_idx = active_members[next_id];
		  	secval.first = members[next_idx].region;
		  	secval.second = members[next_idx].node;
		  	sk.secondary.push_back(secval);
		  }
		  temp_ret[sk.id] = sk;
      }
  }
  return convertShardedMaptoJSON(temp_ret);
}

// TODO: Tweak current metadata
string tweakMetadata(const std::string& m) {
  string ret;

  return ret;
}

/***** LOG WRITER SECTION *****/
class LogWriter {
public:
  LogWriter() {
    file = "data/app.log";
    facet = new time_facet("%Y-%m-%d-%H:%M:$S.%f");
  }

  void writeLog(const std::string& message) {
    log_mutex.lock();
    ofstream log_file(file, ios_base::out | ios_base::app);
    cout.imbue(locale(cout.getloc(), facet));
    log_file << microsec_clock::local_time() << " | " << message << endl;
    log_mutex.unlock();
  }

private:
  string file;
  time_facet* facet;
  mutex log_mutex;
};

LogWriter* logWriter;

/***** END OF LOG WRITER SECTION *****/

/***** CONSENSUS (RAFT) SECTION *****/
enum {
	RAFT_STATE_NONE,
	RAFT_STATE_FOLLOWER,
	RAFT_STATE_CANDIDATE,
	RAFT_STATE_LEADER
};

class RaftConsensus {
public:
  RaftConsensus(vector<Member> _members, string _metadata, int _own_id) {
    current_term = 0;
    log = _metadata;
    voted_for = -1;
    commit_idx = getMetadataValue();
    timeout_elapsed = (rand() % 500) + 150; // random factor
    nodes = _members;
    num_nodes = nodes.size();
    node_id = _own_id;

    // Recover here (or become candidate if fails)
    bool receiveLeader = false;
    vector<thread> workers;

    for (int i = 0; i < num_nodes; i++) { // broadcast to all nodes except own
    	if (i == node_id) continue;
    	int current_id = i;
    	workers.push_back(thread([&](int current_id) {
    		boost::shared_ptr<TTransport> socket(new TSocket(nodes[current_id].ip, nodes[current_id].port));
    		cout << "Recover Metadata: Check " << nodes[current_id].ip << ":" << nodes[current_id].port << endl;

    		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    		DBServiceClient client(protocol);

    		try {
    			transport->open();
    			GetRecover data;
    			client.getRecover(data); // RPC Recover
    			if (data.isLeader) {
    				receiveLeader = true;
    				voted_for = current_id; // current leader
    				current_term = data.term;
    				commit(data.entry, data.commit_idx);
    				logWriter->writeLog("Recovered from node_id (leader) = " + to_string(voted_for));
    			}
    		} catch (TException& tx) {
    			cout << "ERROR: " << tx.what() << endl;
    		}
    	}, current_id)); // end of thread
    }
    for_each(workers.begin(), workers.end(), [](thread &t) {
    	t.join();
    });

    if (receiveLeader) { // Tweak only own primary (change old primary to secondary, drop one secondary)
    	string new_entry = tweakMetadata(log);
    	// TODO: Wait until finish resync before updating metadata

    	// appendRequest(new_entry);
    } else {
    	thread t(&RaftConsensus::initElection, this);
    	t.detach();
    }
  }

  void initElection() {
    // ensure a majority leader is chosen
	bool result = false;
    while (voted_for == -1) {
    	// sleep for timeout_elapsed, declare candidacy
		this_thread::sleep_for(chrono::milliseconds(timeout_elapsed));
		if (voted_for == -1) result = leaderRequest(); // requesting leader (as a candidate)
		if (result) logWriter->writeLog("Leader election finished");
    }
  }

  bool commit(const std::string& entry, int _commit_idx) {
    if (commit_idx < _commit_idx) commit_idx = _commit_idx;
    else return false;
    // update log
    log = entry;
    putMetadataValue(commit_idx);
    writeMetadata(log);
    return true;
  }

  // Send vote request
  bool leaderRequest() {
    int required = (num_nodes / 2) + 1;
    votes_for_me.clear(); // reset voters
    logWriter->writeLog("Become a candidate: leaderRequest to all nodes");

	bool* isSent;
	isSent = (bool*) malloc (num_nodes * sizeof(bool));
	for(int i = 0; i < num_nodes; i++) isSent[i] = false;
	isSent[node_id] = true;

    vector<thread> workers;

    int prev_term = current_term;
	int prev_commit_idx = commit_idx;

	// Update own information
	current_term++;
	commit_idx++;
	voted_for = node_id;
	votes_for_me.push_back(node_id);

    for (int i = 0; i < num_nodes; i++) { // broadcast to all nodes
    	if (i == node_id) continue;
    	int current_id = i;
    	workers.push_back(thread([&](int current_id) {
    		boost::shared_ptr<TTransport> socket(new TSocket(nodes[current_id].ip, nodes[current_id].port));
    		cout << "Leader Request: Send to " << nodes[current_id].ip << ":" << nodes[current_id].port << endl;

    		boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    		boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    		DBServiceClient client(protocol);

    		try {
    			transport->open();
    			VoteRequest request;
    			VoteResponse response;

    			/** Start next term */
    			request.term = prev_term + 1; // increment term
    			request.last_commit_idx = prev_commit_idx + 1; // next commit idx
    			request.peer_id = node_id;

    			client.sendVote(response, request); // RPC VoteRequest
    			if (response.granted) {
    				votes_for_me.push_back(current_id); // node which has voted for this node
    				logWriter->writeLog("Receive leader vote from " + nodes[current_id].ip + ":" + to_string(nodes[current_id].port));
    			}
    			isSent[current_id] = true;
    		} catch (TException& tx) {
    			cout << "ERROR: " << tx.what() << endl;
    		}
    	}, current_id)); // end of thread
	}
	for_each(workers.begin(), workers.end(), [](thread &t) {
		t.join();
	});

	for (int i = 0; i < num_nodes; i++) {
		if (isSent[i]) members[i].active = 1;
		else members[i].active = 0;
	}


    if ((int) votes_for_me.size() >= required) { // become a leader
    	// Send newest metadata based on all active nodes
    	string new_entry = createMetadata();
    	logWriter->writeLog("Become a leader: send newest metadata to all nodes");
    	if (voted_for == node_id) appendRequest(new_entry); // propagate metadata
    } else {
		voted_for = -1; // re-do leader selection
		return false;
	}

    return true;
  }

  // Send append request
  bool appendRequest(const std::string& entry) {
    int leader_id = voted_for;
    if (leader_id == -1) return false;

    boost::shared_ptr<TTransport> socket(new TSocket(nodes[leader_id].ip, nodes[leader_id].port));
    cout << "Append Request: Send to " << nodes[leader_id].ip << ":" << nodes[leader_id].port << endl;

    boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
    boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
    DBServiceClient client(protocol);

    AppendRequest request;
    AppendResponse response;

    try {
    	transport->open();

    	/** Start next term */
    	request.term = current_term + 1; // increase term
    	request.commit_idx = commit_idx + 1;
    	request.entry = entry;

    	logWriter->writeLog("send appendRequest to leader (term " + to_string(current_term) + ")");

    	client.sendAppend(response, request); // RPC AppendRequest
    } catch (TException& tx) {
    	cout << "ERROR: " << tx.what() << endl;
    }

    return response.succeeds;
  }

  void checkFailure(int remote_region, int remote_node) {
    // TODO: Become a raft leader if the failure_node == voted_for

    // TODO: Change unavailable nodes primary setup, update metadata (At this point, there will be only replication_factor - 1 nodes for this shard key)

    // TODO: Search the next nearest one as the new primary and fill the required number of secondaries (Tell a oneway message to resyncData, add new thrift procedure) -> update metadata for a second time here

  }

  /** Getter & Setter */

  int getTerm() {
    return current_term;
  }

  void setTerm(int _term) {
    current_term = _term;
  }

  string getLog() {
    return log;
  }

  int getVotedFor() {
    return voted_for;
  }

  void setVotedFor(int _voted_for) {
    voted_for = _voted_for;
  }

  int getCommitIdx() {
    return commit_idx;
  }

  int getNodeId() {
    return node_id;
  }

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

RaftConsensus* raft;

/***** END OF CONSENSUS (RAFT) SECTION *****/

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
      members[i].active = 1; // assume all nodes are active
     // add additional info for self
     if (info["own"].GetBool()) {
         own_id = (int) i;
         server_ip = members[i].ip;
         server_port = members[i].port;
         server_region = members[i].region;
         server_node = members[i].node;
         identity = constructShardKey(server_region, server_node);
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
  if (getMetadataValue() != -1) {
	  // Read from metadata.tmp
	  ifstream t("data/metadata.tmp");
	  string str;

	  t.seekg(0, ios::end);
	  str.reserve(t.tellg());
	  t.seekg(0, ios::beg);

	  str.assign((istreambuf_iterator<char>(t)), istreambuf_iterator<char>());
	  t.close();

	  /** Convert string to json */
	  metadata = str; // initialize metadata
	  const char* json = metadata.c_str();

	  decodeKeys(skeys, json);	
  }
}

/***** END OF MEMBERSHIP SECTION *****/

/***** LOGICAL CLOCK SECTION *****/

class LogicalClock {
public:
  LogicalClock() {
  }

  long long incrementLClock(const std::string& key) {
    ts_mutex.lock();
    long long c = getLClock(key);
    c++;
    putLClock(key, c);
    ts_mutex.unlock();
    return c;
  }

  bool tsCheck(const std::string& key, int64_t ts) {
    long long db_ts = getLClock(key);
    if (db_ts < (long long) ts) return true;
    else return false;
  }

private:
  mutex ts_mutex;
};

LogicalClock* lClock;

/***** END OF LOGICAL CLOCK SECTION *****/

/***** SECONDARY SECTION *****/

void failureTask(int remote_region, int remote_node) {
	raft->checkFailure(remote_region, remote_node);
}

/**
  * code 0: replicateTask
  * code 1: updateTask
  * code 2: deleteTask
  */
void backgroundTask(const std::string& key, const std::string& value, long long ts, int code) {
	int timer = 10; // initial timer, in seconds
	Data d;
	d.key = key;
	d.value = value;
	d.ts = ts;

	if (code == 0) cout << "Replicate task: " << d.key << " " << d.value << " (ts : " << ts << ")" << endl;
	else if (code == 1) cout << "Update task: " << d.key << " " << d.value << " (ts : " << ts << ")" << endl;
	else if (code == 2) cout << "Delete task: " << d.key << endl;
	else return;

	string identifier = d.key.substr(0, 8); // first 8 characters
	vector< pair<int, int> > secondary = skeys[identifier].secondary;
	bool* isSent;
	isSent = (bool*) malloc ((int) secondary.size() * sizeof(bool));
	for(int i = 0; i < (int) secondary.size(); i++) {
		int idx = member_pos[constructShardKey(secondary[i].first, secondary[i].second)];
		if (members[idx].active == 1) isSent[i] = false;
		else isSent[i] = true; // failed node
	}
	while (timer <= 30) {
		vector<thread> workers;
		for(int i = 0; i < (int) secondary.size(); i++) { // broadcast to all secondaries
			if (!isSent[i]) {
				int current_id = i;
				workers.push_back(thread([&](int current_id) {
					int idx = member_pos[constructShardKey(secondary[current_id].first, secondary[current_id].second)];
					boost::shared_ptr<TTransport> socket(new TSocket(members[idx].ip, members[idx].port));

					if (code == 0) cout << "Replicate " << d.key << " to " << members[idx].ip << ":" << members[idx].port;
					else if (code == 1) cout << "Update " << d.key << " to " << members[idx].ip << ":" << members[idx].port;
					else if (code == 2) cout << "Delete " << d.key << " from " << members[idx].ip << ":" << members[idx].port;

					if (timer <= 30) cout << " (After Timeout: " << timer <<  " second(s))" << endl;
					else cout << endl;
					boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
					boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
					DBServiceClient client(protocol);

					try {
						transport->open();

						if(code == 0) isSent[current_id] = client.replicateData(d, server_region, server_node); // RPC Replicate
						else if (code == 1) isSent[current_id] = client.updateSecondaryData(d, server_region, server_node); // RPC Update
						else if (code == 2) isSent[current_id] = client.deleteSecondaryData(d.key, server_region, server_node); // RPC Delete

					} catch (TException& tx) {
						cout << "ERROR: " << tx.what() << endl;
					}
				}, current_id)); // end of thread
			}
		}
		for_each(workers.begin(), workers.end(), [](thread &t) {
			t.join();
		});
		bool globalSent = true;
		for (int i = 0; i < (int) secondary.size(); i++)
			if (!isSent[i]) globalSent = false;
		if (globalSent) break;
		else {
			// Sleep for timer seconds here
			if (timer <= 30) this_thread::sleep_for(chrono::seconds(timer));
		}
		timer += 10;
	}
	for (int i = 0; i < (int) secondary.size(); i++)
		if (!isSent[i]) {
			int idx = member_pos[constructShardKey(secondary[i].first, secondary[i].second)];
			members[idx].active = 0; // declare failure
			failureTask(secondary[i].first, secondary[i].second);
		}
}

void broadcastMetadata(const std::string& entry, int commit_idx, int term) {
	int timer = 10; // initial timer, in seconds

	bool* isSent;
	isSent = (bool*) malloc ((int) members.size() * sizeof(bool));
	for(int i = 0; i < (int) members.size(); i++) {
		if (members[i].active == 1)	isSent[i] = false;
		else isSent[i] = true; // failed node
	}
	isSent[own_id] = true; // exclude own id
	while (timer <= 30) {
		vector<thread> workers;

		for (int i = 0; i < (int) members.size(); i++) { // broadcast to all nodes
			if (!isSent[i]) {
				int current_id = i;
				workers.push_back(thread([&](int current_id) {
					boost::shared_ptr<TTransport> socket(new TSocket(members[current_id].ip, members[current_id].port));
					cout << "Broadcast metadata to " << members[current_id].ip << ":" << members[current_id].port << " (After Timeout: " << timer <<  " second(s))" << endl;

					boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
					boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
					DBServiceClient client(protocol);

					try {
						transport->open();
						AppendRequest request;

						request.term = term;
						request.commit_idx = commit_idx;
						request.entry = entry;

						isSent[current_id] = client.followerAppend(request); // RPC FollowerAppend
					} catch (TException& tx) {
						cout << "ERROR: " << tx.what() << endl;
					}
				}, current_id)); // end of thread
			}
		}
		for_each(workers.begin(), workers.end(), [](thread &t) {
			t.join();
		});
		bool globalSent = true;
		for (int i = 0; i < (int) members.size(); i++)
			if (!isSent[i]) globalSent = false;
		if (globalSent) break;
		else {
			// Sleep for timer seconds here
			if (timer <= 30) this_thread::sleep_for(chrono::seconds(timer));
		}
		timer += 10;
	}
	for (int i = 0; i < (int) members.size(); i++)
		if (!isSent[i]) {
			members[i].active = 0;
			failureTask(members[i].region, members[i].node);
		}
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
		long long clock = lClock->incrementLClock(_return);
		thread t(backgroundTask, _return, value, clock, 0);
		t.detach();
	} else {
		// TODO: shard limit, check other secondary partitions
		vector< pair<int, int> > secondary = skeys[identity].secondary;
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
		long long clock = lClock->incrementLClock(_return);
		thread t(backgroundTask, _return, value, clock, 0);
		t.detach();
	}
  }

  // First come first serve basis
  bool updateData(const Data& d) {
	string identifier = d.key.substr(0, 8); // first 8 characters
	pair<int, int> location = skeys[identifier].primary;
    if (location.first == server_region && location.second == server_node) {
    bool isSuccess = updateDB(d.key, d.value);
		if (isSuccess) {
			long long clock = lClock->incrementLClock(d.key);
			thread t(backgroundTask, d.key, d.value, clock, 1);
			t.detach();
		}
    	return isSuccess;
    } else {
			// search at specified region and node
			int idx = member_pos[constructShardKey(location.first, location.second)];

			boost::shared_ptr<TTransport> socket(new TSocket(members[idx].ip, members[idx].port));
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
  bool updateSecondaryData(const Data& d, const int32_t remote_region, const int32_t remote_node) {
	cout << "updateSecondaryData is called" << endl;
	bool check = lClock->tsCheck(d.key, d.ts);
	if (check) { // update if logical clock 'ts' is higher (ts > lclock)
		bool isSuccess = updateDB(d.key, d.value);
		if (isSuccess) {
			putLClock(d.key, d.ts);
		}
	}
    return true;
  }

  // First come first serve basis (return empty string if sharded_key != exists)
  void getData(std::string& _return, const std::string& sharded_key) {
	string identifier = sharded_key.substr(0, 8); // first 8 characters
	pair<int, int> location = skeys[identifier].primary;
    if (location.first == server_region && location.second == server_node) {
		_return = getDB(sharded_key);
    } else {
			// search at specified region and node
			int idx = member_pos[constructShardKey(location.first, location.second)];
			boost::shared_ptr<TTransport> socket(new TSocket(members[idx].ip, members[idx].port));
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

  // First come first serve basis
  bool deleteData(const std::string& sharded_key) {
	string identifier = sharded_key.substr(0, 8); // first 8 characters
	pair<int, int> location = skeys[identifier].primary;
    if (location.first == server_region && location.second == server_node) {
		bool isSuccess = deleteDB(sharded_key);
		if (isSuccess) {
			thread t(backgroundTask, sharded_key, "", 0, 2);
			t.detach();
		}
		return isSuccess;
    } else {
			// search at specified region and node
			int idx = member_pos[constructShardKey(location.first, location.second)];
			boost::shared_ptr<TTransport> socket(new TSocket(members[idx].ip, members[idx].port));
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
  bool deleteSecondaryData(const std::string& sharded_key, const int32_t remote_region, const int32_t remote_node) {
	cout << "deleteSecondaryData is called" << endl;
	deleteDB(sharded_key);
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
	cout << "replicateData is called" << endl;
	bool check = lClock->tsCheck(d.key, d.ts);
	if (check) {
		bool isSuccess = updateDB(d.key, d.value); // same behavior with update
		if (isSuccess) {
			putLClock(d.key, d.ts);
		}
	}
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
	cout << "resyncData is called" << endl;
	list< pair< pair<string, string>, long long> > data; // (key, value, ts)
	resyncDB(data, constructShardKey(remote_region, remote_node));
	cout << "Size: " << data.size() << endl;
	for (list< pair< pair<string, string>, long long> >::iterator it=data.begin(); it != data.end(); ++it) {
		Data t;
		t.key = (*it).first.first;
		t.value = (*it).first.second;
		t.ts = (*it).second;
		_return.data.push_back(t); // append result
	}
  }

  /**
   * getRecover
   * Get newest metadata (recovery phase)
   */
  void getRecover(GetRecover& _return) {
	if (raft->getVotedFor() == raft->getNodeId()) {
		_return.isLeader = true;
		_return.term = raft->getTerm();
		_return.commit_idx = raft->getCommitIdx();
		_return.entry = raft->getLog();
	} else _return.isLeader = false;

	/** Write to log */
	string message;
	if (_return.isLeader) message = "accepted";
	else message = "rejected (this node is not a leader)";
	logWriter->writeLog("getRecover " + message);
  }

  /**
   * sendAppend
   * Send append request -> Update metadata (consensus). On the other hand, lock metadata from other R/W operation
   * 
   * @param request
   */
  void sendAppend(AppendResponse& _return, const AppendRequest& request) {
	bool succeeds = false;
	if (raft->getVotedFor() == raft->getNodeId() && request.term >= raft->getTerm()) {
		raft->setTerm(request.term); // update Term
		if (raft->getCommitIdx() < request.commit_idx) {
			succeeds = raft->commit(request.entry, request.commit_idx); // commit
			if (succeeds) {
				// Send metadata / state to all followers (exclude own id)
				thread t(broadcastMetadata, request.entry, request.commit_idx, request.term);
				t.detach();
			}
		}
	}
	_return.term = request.term;
	_return.succeeds = succeeds;

	/** Write to log */
	string message;
	if (succeeds) message = "accepted";
	else message = "rejected";
	logWriter->writeLog("sendAppend " + message + " (term " + to_string(request.term) + ")");
  }

  /**
   * sendVote
   * Send vote request
   * 
   * @param request
   */
  void sendVote(VoteResponse& _return, const VoteRequest& request) {
	bool granted = false;
	if (request.term >= raft->getTerm()) {
		raft->setTerm(request.term); // update Term
		int voted_for = raft->getVotedFor();
		if ((voted_for == -1 || voted_for == request.peer_id) && request.last_commit_idx >= raft->getCommitIdx()) {
			granted = true;
			raft->setVotedFor(request.peer_id);
		}
	}
	_return.term = request.term;
	_return.granted = granted;

	/** Write to log */
	string message;
	if (granted) message = "accepted";
	else message = "rejected";
	logWriter->writeLog("sendVote " + message + " to " + to_string(request.peer_id) + " (term " + to_string(request.term) + ")");
  }

  /**
   * followerAppend
   * Append newest committed metadata at follower
   * 
   * @param request
   */
  bool followerAppend(const AppendRequest& request) {
	if (request.commit_idx < raft->getCommitIdx()) return false;
	if (raft->getTerm() < request.term) raft->setTerm(request.term);
	logWriter->writeLog("receive followerAppend");
	return raft->commit(request.entry, request.commit_idx); // commit
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
  // Load log writer
  logWriter = new LogWriter();
  logWriter->writeLog("DBServiceServer is started");

  // Test leveldb
  initDB(argv[2], shard_size);
  // test();

  // Load shared keys configuration (metadata.tmp)
  loadSKeys();
  printSKeys();

  // Load logical clock
  lClock = new LogicalClock();

  // Create RAFT object
  raft = new RaftConsensus(members, metadata, own_id);

  cout << "** Starting the server **" << endl;
  server.serve();
  cout << "** Done **" << endl;

  return 0;
}


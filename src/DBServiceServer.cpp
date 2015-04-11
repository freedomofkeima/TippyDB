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

string global_id;
string server_ip;
int server_port;
int server_region;
int server_node;

struct Member {
  int region;
  int node;
  string ip;
  int port;
  vector<int> distance;
};

vector<Member> members;

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

     // add additional info for self
     if (info["own"].GetBool()) {
         server_ip = members[i].ip;
         server_port = members[i].port;
         server_region = members[i].region;
         server_node = members[i].node;
     }
  }

  // StringBuffer buffer;
  // Writer<StringBuffer> writer(buffer);
  // d.Accept(writer);
  // cout << buffer.GetString() << endl;
}

class DBServiceHandler : virtual public DBServiceIf {
 public:
  DBServiceHandler() {}

  void ping() {
    printf("ping\n");
  }

  void putData(std::string& _return, const Data& d) {
    // Your implementation goes here
    _return = d.key;
  }

  bool updateData(const Data& d) {
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
    printf("zip\n");
  }

};

int main(int argc, char **argv) {
  if (argc != 3) {
    cout << "Usage: ./application_name port_number db_path" << endl;
    return 1;
  }
  cout << "Port number: " << argv[1] << endl;
  cout << "DB path: " << argv[2] << endl;

  int port = atoi(argv[1]);
  shared_ptr<DBServiceHandler> handler(new DBServiceHandler());
  shared_ptr<TProcessor> processor(new DBServiceProcessor(handler));
  shared_ptr<TServerTransport> serverTransport(new TServerSocket(port));
  shared_ptr<TTransportFactory> transportFactory(new TBufferedTransportFactory());
  shared_ptr<TProtocolFactory> protocolFactory(new TBinaryProtocolFactory());

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
  initDB(argv[2]);
  test();

  cout << "** Starting the server **" << endl;
  server.serve();
  cout << "** Done **" << endl;

  return 0;
}


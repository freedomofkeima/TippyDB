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

#include <iostream>
#include <cstring>
#include <vector>

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
string server_ip;
int server_port;
int server_region;
int server_node;

struct Member {
  int region;
  int node;
  string ip;
  int port;
  int distance;
};

vector<Member> members;

void loadMembers() {

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
   * Retrieve all shard contents where region = remote_region && node = remote_node
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
  int port = 9090;
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
  // Rapidjson test
  string json_str = "{\"project\":\"rapidjson\", \"stars\": 10}";
  const char* json = json_str.c_str();
  Document d;
  d.Parse(json);
  Value& s = d["stars"];
  s.SetInt(s.GetInt() + 1);
  StringBuffer buffer;
  Writer<StringBuffer> writer(buffer);
  d.Accept(writer);
  cout << buffer.GetString() << endl;

  // Load members configuration
  loadMembers();

  // Create one thread for each member

  // Test leveldb
  initDB();
  test();

  cout << "** Starting the server **" << endl;
  server.serve();
  cout << "** Done **" << endl;

  return 0;
}


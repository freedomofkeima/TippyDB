// This autogenerated skeleton file illustrates how to build a server.
// You should copy it to another filename to avoid overwriting it.

#include "DBService.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>

using namespace ::apache::thrift;
using namespace ::apache::thrift::protocol;
using namespace ::apache::thrift::transport;
using namespace ::apache::thrift::server;

using boost::shared_ptr;

using namespace  ::dbservice;

class DBServiceHandler : virtual public DBServiceIf {
 public:
  DBServiceHandler() {
    // Your initialization goes here
  }

  void ping() {
    // Your implementation goes here
    printf("ping\n");
  }

  void putData(std::string& _return, const Data& d) {
    // Your implementation goes here
    printf("putData\n");
  }

  bool updateData(const Data& d) {
    // Your implementation goes here
    printf("updateData\n");
  }

  void getData(std::string& _return, const std::string& sharded_key) {
    // Your implementation goes here
    printf("getData\n");
  }

  bool deleteData(const std::string& sharded_key) {
    // Your implementation goes here
    printf("deleteData\n");
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
    // Your implementation goes here
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

  TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
  server.serve();
  return 0;
}

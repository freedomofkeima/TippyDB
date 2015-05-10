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

  void putData(std::string& _return, const std::string& value) {
    // Your implementation goes here
    printf("putData\n");
  }

  /**
   * putDataForce
   * Write a new data by force (due to partition limitation)
   * 
   * @param value
   * @param remote_region
   * @param remote_node
   */
  void putDataForce(std::string& _return, const std::string& value, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    printf("putDataForce\n");
  }

  bool updateData(const Data& d) {
    // Your implementation goes here
    printf("updateData\n");
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
    printf("updateSecondaryData\n");
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
   * deleteSecondaryData
   * Remove data from secondary nodes where region = remote_region && node == remote_node
   * 
   * @param sharded_key
   * @param remote_region
   * @param remote_node
   */
  bool deleteSecondaryData(const std::string& sharded_key, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    printf("deleteSecondaryData\n");
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
    printf("replicateData\n");
  }

  /**
   * resyncData
   * Retrieve all newest shard contents where region = remote_region && node = remote_node (choose the nearest one for primary / the smallest db size for secondary)
   * 
   * @param remote_region
   * @param remote_node
   */
  void resyncData(ShardContent& _return, const int32_t remote_region, const int32_t remote_node) {
    // Your implementation goes here
    printf("resyncData\n");
  }

  /**
   * pushResyncData
   * Push ShardContent from primary node to other node
   * 
   * @param contents
   */
  bool pushResyncData(const ShardContent& contents) {
    // Your implementation goes here
    printf("pushResyncData\n");
  }

  /**
   * getRecover
   * Get newest metadata (recovery phase)
   */
  void getRecover(GetRecover& _return) {
    // Your implementation goes here
    printf("getRecover\n");
  }

  /**
   * sendAppend
   * Send append request -> Update metadata (consensus). On the other hand, lock metadata from other R/W operation
   * 
   * @param request
   */
  void sendAppend(AppendResponse& _return, const AppendRequest& request) {
    // Your implementation goes here
    printf("sendAppend\n");
  }

  /**
   * sendVote
   * Send vote request
   * 
   * @param request
   */
  void sendVote(VoteResponse& _return, const VoteRequest& request) {
    // Your implementation goes here
    printf("sendVote\n");
  }

  /**
   * followerAppend
   * Append newest committed metadata at follower
   * 
   * @param request
   */
  bool followerAppend(const AppendRequest& request) {
    // Your implementation goes here
    printf("followerAppend\n");
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


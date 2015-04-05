/**
 * Autogenerated by Thrift Compiler (0.9.2)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#ifndef DBService_H
#define DBService_H

#include <thrift/TDispatchProcessor.h>
#include "dbservice_types.h"

namespace dbservice {

class DBServiceIf {
 public:
  virtual ~DBServiceIf() {}
  virtual void ping() = 0;
  virtual void zip() = 0;
};

class DBServiceIfFactory {
 public:
  typedef DBServiceIf Handler;

  virtual ~DBServiceIfFactory() {}

  virtual DBServiceIf* getHandler(const ::apache::thrift::TConnectionInfo& connInfo) = 0;
  virtual void releaseHandler(DBServiceIf* /* handler */) = 0;
};

class DBServiceIfSingletonFactory : virtual public DBServiceIfFactory {
 public:
  DBServiceIfSingletonFactory(const boost::shared_ptr<DBServiceIf>& iface) : iface_(iface) {}
  virtual ~DBServiceIfSingletonFactory() {}

  virtual DBServiceIf* getHandler(const ::apache::thrift::TConnectionInfo&) {
    return iface_.get();
  }
  virtual void releaseHandler(DBServiceIf* /* handler */) {}

 protected:
  boost::shared_ptr<DBServiceIf> iface_;
};

class DBServiceNull : virtual public DBServiceIf {
 public:
  virtual ~DBServiceNull() {}
  void ping() {
    return;
  }
  void zip() {
    return;
  }
};


class DBService_ping_args {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  DBService_ping_args(const DBService_ping_args&);
  DBService_ping_args& operator=(const DBService_ping_args&);
  DBService_ping_args() {
  }

  virtual ~DBService_ping_args() throw();

  bool operator == (const DBService_ping_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const DBService_ping_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DBService_ping_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const DBService_ping_args& obj);
};


class DBService_ping_pargs {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~DBService_ping_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const DBService_ping_pargs& obj);
};


class DBService_ping_result {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  DBService_ping_result(const DBService_ping_result&);
  DBService_ping_result& operator=(const DBService_ping_result&);
  DBService_ping_result() {
  }

  virtual ~DBService_ping_result() throw();

  bool operator == (const DBService_ping_result & /* rhs */) const
  {
    return true;
  }
  bool operator != (const DBService_ping_result &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DBService_ping_result & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const DBService_ping_result& obj);
};


class DBService_ping_presult {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~DBService_ping_presult() throw();

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);

  friend std::ostream& operator<<(std::ostream& out, const DBService_ping_presult& obj);
};


class DBService_zip_args {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};

  DBService_zip_args(const DBService_zip_args&);
  DBService_zip_args& operator=(const DBService_zip_args&);
  DBService_zip_args() {
  }

  virtual ~DBService_zip_args() throw();

  bool operator == (const DBService_zip_args & /* rhs */) const
  {
    return true;
  }
  bool operator != (const DBService_zip_args &rhs) const {
    return !(*this == rhs);
  }

  bool operator < (const DBService_zip_args & ) const;

  uint32_t read(::apache::thrift::protocol::TProtocol* iprot);
  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const DBService_zip_args& obj);
};


class DBService_zip_pargs {
 public:

  static const char* ascii_fingerprint; // = "99914B932BD37A50B983C5E7C90AE93B";
  static const uint8_t binary_fingerprint[16]; // = {0x99,0x91,0x4B,0x93,0x2B,0xD3,0x7A,0x50,0xB9,0x83,0xC5,0xE7,0xC9,0x0A,0xE9,0x3B};


  virtual ~DBService_zip_pargs() throw();

  uint32_t write(::apache::thrift::protocol::TProtocol* oprot) const;

  friend std::ostream& operator<<(std::ostream& out, const DBService_zip_pargs& obj);
};

class DBServiceClient : virtual public DBServiceIf {
 public:
  DBServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
    setProtocol(prot);
  }
  DBServiceClient(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    setProtocol(iprot,oprot);
  }
 private:
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> prot) {
  setProtocol(prot,prot);
  }
  void setProtocol(boost::shared_ptr< ::apache::thrift::protocol::TProtocol> iprot, boost::shared_ptr< ::apache::thrift::protocol::TProtocol> oprot) {
    piprot_=iprot;
    poprot_=oprot;
    iprot_ = iprot.get();
    oprot_ = oprot.get();
  }
 public:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getInputProtocol() {
    return piprot_;
  }
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> getOutputProtocol() {
    return poprot_;
  }
  void ping();
  void send_ping();
  void recv_ping();
  void zip();
  void send_zip();
 protected:
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> piprot_;
  boost::shared_ptr< ::apache::thrift::protocol::TProtocol> poprot_;
  ::apache::thrift::protocol::TProtocol* iprot_;
  ::apache::thrift::protocol::TProtocol* oprot_;
};

class DBServiceProcessor : public ::apache::thrift::TDispatchProcessor {
 protected:
  boost::shared_ptr<DBServiceIf> iface_;
  virtual bool dispatchCall(::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, const std::string& fname, int32_t seqid, void* callContext);
 private:
  typedef  void (DBServiceProcessor::*ProcessFunction)(int32_t, ::apache::thrift::protocol::TProtocol*, ::apache::thrift::protocol::TProtocol*, void*);
  typedef std::map<std::string, ProcessFunction> ProcessMap;
  ProcessMap processMap_;
  void process_ping(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
  void process_zip(int32_t seqid, ::apache::thrift::protocol::TProtocol* iprot, ::apache::thrift::protocol::TProtocol* oprot, void* callContext);
 public:
  DBServiceProcessor(boost::shared_ptr<DBServiceIf> iface) :
    iface_(iface) {
    processMap_["ping"] = &DBServiceProcessor::process_ping;
    processMap_["zip"] = &DBServiceProcessor::process_zip;
  }

  virtual ~DBServiceProcessor() {}
};

class DBServiceProcessorFactory : public ::apache::thrift::TProcessorFactory {
 public:
  DBServiceProcessorFactory(const ::boost::shared_ptr< DBServiceIfFactory >& handlerFactory) :
      handlerFactory_(handlerFactory) {}

  ::boost::shared_ptr< ::apache::thrift::TProcessor > getProcessor(const ::apache::thrift::TConnectionInfo& connInfo);

 protected:
  ::boost::shared_ptr< DBServiceIfFactory > handlerFactory_;
};

class DBServiceMultiface : virtual public DBServiceIf {
 public:
  DBServiceMultiface(std::vector<boost::shared_ptr<DBServiceIf> >& ifaces) : ifaces_(ifaces) {
  }
  virtual ~DBServiceMultiface() {}
 protected:
  std::vector<boost::shared_ptr<DBServiceIf> > ifaces_;
  DBServiceMultiface() {}
  void add(boost::shared_ptr<DBServiceIf> iface) {
    ifaces_.push_back(iface);
  }
 public:
  void ping() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->ping();
    }
    ifaces_[i]->ping();
  }

  void zip() {
    size_t sz = ifaces_.size();
    size_t i = 0;
    for (; i < (sz - 1); ++i) {
      ifaces_[i]->zip();
    }
    ifaces_[i]->zip();
  }

};

} // namespace

#endif

/**
 * Autogenerated by Thrift Compiler (0.9.2)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
#include "dbservice_types.h"

#include <algorithm>
#include <ostream>

#include <thrift/TToString.h>

namespace dbservice {


Data::~Data() throw() {
}


void Data::__set_key(const std::string& val) {
  this->key = val;
}

void Data::__set_value(const std::string& val) {
  this->value = val;
}

void Data::__set_ts(const int64_t val) {
  this->ts = val;
}

const char* Data::ascii_fingerprint = "A0ED90CE9B69D7A0FCE24E26CAECD2AF";
const uint8_t Data::binary_fingerprint[16] = {0xA0,0xED,0x90,0xCE,0x9B,0x69,0xD7,0xA0,0xFC,0xE2,0x4E,0x26,0xCA,0xEC,0xD2,0xAF};

uint32_t Data::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->key);
          this->__isset.key = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 2:
        if (ftype == ::apache::thrift::protocol::T_STRING) {
          xfer += iprot->readString(this->value);
          this->__isset.value = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      case 3:
        if (ftype == ::apache::thrift::protocol::T_I64) {
          xfer += iprot->readI64(this->ts);
          this->__isset.ts = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t Data::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  oprot->incrementRecursionDepth();
  xfer += oprot->writeStructBegin("Data");

  xfer += oprot->writeFieldBegin("key", ::apache::thrift::protocol::T_STRING, 1);
  xfer += oprot->writeString(this->key);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("value", ::apache::thrift::protocol::T_STRING, 2);
  xfer += oprot->writeString(this->value);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldBegin("ts", ::apache::thrift::protocol::T_I64, 3);
  xfer += oprot->writeI64(this->ts);
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  oprot->decrementRecursionDepth();
  return xfer;
}

void swap(Data &a, Data &b) {
  using ::std::swap;
  swap(a.key, b.key);
  swap(a.value, b.value);
  swap(a.ts, b.ts);
  swap(a.__isset, b.__isset);
}

Data::Data(const Data& other0) {
  key = other0.key;
  value = other0.value;
  ts = other0.ts;
  __isset = other0.__isset;
}
Data& Data::operator=(const Data& other1) {
  key = other1.key;
  value = other1.value;
  ts = other1.ts;
  __isset = other1.__isset;
  return *this;
}
std::ostream& operator<<(std::ostream& out, const Data& obj) {
  using apache::thrift::to_string;
  out << "Data(";
  out << "key=" << to_string(obj.key);
  out << ", " << "value=" << to_string(obj.value);
  out << ", " << "ts=" << to_string(obj.ts);
  out << ")";
  return out;
}


ShardContent::~ShardContent() throw() {
}


void ShardContent::__set_data(const Shard& val) {
  this->data = val;
}

const char* ShardContent::ascii_fingerprint = "CADF07CD5CD3A18B4DEF7E3A4388D017";
const uint8_t ShardContent::binary_fingerprint[16] = {0xCA,0xDF,0x07,0xCD,0x5C,0xD3,0xA1,0x8B,0x4D,0xEF,0x7E,0x3A,0x43,0x88,0xD0,0x17};

uint32_t ShardContent::read(::apache::thrift::protocol::TProtocol* iprot) {

  uint32_t xfer = 0;
  std::string fname;
  ::apache::thrift::protocol::TType ftype;
  int16_t fid;

  xfer += iprot->readStructBegin(fname);

  using ::apache::thrift::protocol::TProtocolException;


  while (true)
  {
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == ::apache::thrift::protocol::T_STOP) {
      break;
    }
    switch (fid)
    {
      case 1:
        if (ftype == ::apache::thrift::protocol::T_LIST) {
          {
            this->data.clear();
            uint32_t _size2;
            ::apache::thrift::protocol::TType _etype5;
            xfer += iprot->readListBegin(_etype5, _size2);
            this->data.resize(_size2);
            uint32_t _i6;
            for (_i6 = 0; _i6 < _size2; ++_i6)
            {
              xfer += this->data[_i6].read(iprot);
            }
            xfer += iprot->readListEnd();
          }
          this->__isset.data = true;
        } else {
          xfer += iprot->skip(ftype);
        }
        break;
      default:
        xfer += iprot->skip(ftype);
        break;
    }
    xfer += iprot->readFieldEnd();
  }

  xfer += iprot->readStructEnd();

  return xfer;
}

uint32_t ShardContent::write(::apache::thrift::protocol::TProtocol* oprot) const {
  uint32_t xfer = 0;
  oprot->incrementRecursionDepth();
  xfer += oprot->writeStructBegin("ShardContent");

  xfer += oprot->writeFieldBegin("data", ::apache::thrift::protocol::T_LIST, 1);
  {
    xfer += oprot->writeListBegin(::apache::thrift::protocol::T_STRUCT, static_cast<uint32_t>(this->data.size()));
    std::vector<Data> ::const_iterator _iter7;
    for (_iter7 = this->data.begin(); _iter7 != this->data.end(); ++_iter7)
    {
      xfer += (*_iter7).write(oprot);
    }
    xfer += oprot->writeListEnd();
  }
  xfer += oprot->writeFieldEnd();

  xfer += oprot->writeFieldStop();
  xfer += oprot->writeStructEnd();
  oprot->decrementRecursionDepth();
  return xfer;
}

void swap(ShardContent &a, ShardContent &b) {
  using ::std::swap;
  swap(a.data, b.data);
  swap(a.__isset, b.__isset);
}

ShardContent::ShardContent(const ShardContent& other8) {
  data = other8.data;
  __isset = other8.__isset;
}
ShardContent& ShardContent::operator=(const ShardContent& other9) {
  data = other9.data;
  __isset = other9.__isset;
  return *this;
}
std::ostream& operator<<(std::ostream& out, const ShardContent& obj) {
  using apache::thrift::to_string;
  out << "ShardContent(";
  out << "data=" << to_string(obj.data);
  out << ")";
  return out;
}

} // namespace

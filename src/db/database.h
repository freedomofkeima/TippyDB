/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * database.h
  *
  */

#ifndef DATABASE_H
#define DATABASE_H

#include <assert.h>
#include <algorithm>
#include <iostream>

#include <mutex>
#include <string>

#include "leveldb/comparator.h"
#include "leveldb/db.h"

using namespace std;

/** Global variable (Single DAO) */
extern leveldb::DB* db;
extern leveldb::Status status;
extern leveldb::WriteOptions write_options;
extern leveldb::WriteOptions write_options2;
extern leveldb::ReadOptions read_options;
extern leveldb::Slice counter_key;
extern int counter_value;

extern string log_filepath;

void initDB(string path);

string fixedLength(int value, int digits);
string generate_key(int region, int node, int ctx);

string putDB(const string value, int region, int node);

void test();

#endif

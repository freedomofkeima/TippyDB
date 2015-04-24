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
extern leveldb::Slice counter_key; // reserved key
extern leveldb::Slice metadata_key; // reserved key
extern leveldb::Slice psize_key; // reserved key (for primary size, in bytes)
extern string lclock_key; // reserved key (for logical clock)
extern int counter_value;
extern long long psize_value;
extern long long size;

extern string log_filepath;

void initDB(string path, int shard_size);

// Retrieve metadata version (return -1 if not set)
int getMetadataValue();

// Update metadata version
void putMetadataValue(int version);

// Retrieve logical clock counter
long long getLClock(const string key);

// Update logical clock counter
void putLClock(const string key, long long logical_clock);

string fixedLength(int value, int digits);
string generate_key(int region, int node, int ctx);
pair<int, int> parse_key(const string key);

string putDB(const string value, int region, int node, bool force);

bool updateDB(const string key, const string value);

string getDB(const string key);

bool deleteDB(const string key);

void test();

#endif

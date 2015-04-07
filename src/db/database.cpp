/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * database.cpp
  *
  */
#include "database.h"

#include <assert.h>
#include <iostream>

#include "leveldb/comparator.h"
#include "leveldb/db.h"

using namespace std;

/** Global variable (Single DAO) */
leveldb::DB* db;
leveldb::Status status;
leveldb::WriteOptions write_options;
leveldb::WriteOptions write_options2;
leveldb::ReadOptions read_options;
leveldb::Slice counter_key = "counter"; // reserved key
string counter_value = "0";

string log_filepath = "../data/app.log";

void initDB() {
	leveldb::Options options;
	options.create_if_missing = true;
	status = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(status.ok());
	cout << status.ToString() << endl;

	leveldb::Status local_status;
	local_status = db->Get(read_options, counter_key, &counter_value);
	if (!local_status.ok()) { // initialize counter if it hasn't been initialized
		db->Put(write_options, counter_key, counter_value);
	}

	write_options2.sync = true;
}

void test() {
	/** Write a pair of key and value to database */
	leveldb::Slice key = "IF3230";
	string value = "This is a test string.";
	status = db->Put(write_options, key, value);

	/** Read non-existing key */
	leveldb::Slice dummy_key = "dummy";
	string result;
	status = db->Get(read_options, dummy_key, &result);
	cout << "Non-existing key: " << result << endl;

	/** Read existing key */
	status = db->Get(read_options, key, &result);
	cout << "Existing key: " << result << endl;

	/** Iterator */
	cout << "Using Iterator" << endl;
	leveldb::Iterator* it = db->NewIterator(read_options);
	for(it->SeekToFirst(); it->Valid(); it->Next()) {
		leveldb::Slice k = it->key();
		leveldb::Slice v = it->value();
		cout <<  k.ToString() << " " << v.ToString() << endl;
	}

	assert(it->status().ok()); // Check for any errors during scan
	delete it;

	/** Compare */
	leveldb::Slice a = std::string("aaaaa");
	leveldb::Slice b = std::string("aaaa");
    leveldb::Options options;
	cout << options.comparator->Compare(a, b) << endl;
}

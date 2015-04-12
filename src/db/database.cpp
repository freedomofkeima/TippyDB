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
#include <algorithm>
#include <iostream>

#include <mutex>
#include <string>

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
int counter_value = 0;

mutex put_mutex;

string log_filepath = "../data/app.log";

void initDB(string path) {
	leveldb::Options options;
	options.create_if_missing = true;
	status = leveldb::DB::Open(options, path, &db);
	assert(status.ok());
	cout << status.ToString() << endl;

	leveldb::Status local_status;
    string temp_value;
	local_status = db->Get(read_options, counter_key, &temp_value);
    counter_value = stoi(temp_value);
	if (!local_status.ok()) { // initialize counter if it hasn't been initialized
		db->Put(write_options, counter_key, to_string(counter_value));
	}

	write_options2.sync = true;
}

string fixedLength(int value, int digits) {
	unsigned int uvalue = value;
	if (value < 0) uvalue = -uvalue;

	string result;
	while (digits-- > 0) {
		result += ('0' + uvalue % 10);
		uvalue /= 10;
	}
	if (value < 0) {
		result += '-';
	}

	reverse(result.begin(), result.end());
	return result;
}

/** 16 bytes shared key */
string generate_key(int region, int node, int ctx) {
	return fixedLength(region, 4) + fixedLength(node, 4) + fixedLength(ctx, 8);
}

string putDB(const string value, int region, int node) {
	string shared_key;

	put_mutex.lock();
	counter_value++;
	// increment counter's value
	db->Put(write_options, counter_key, to_string(counter_value));
	put_mutex.unlock();

	shared_key = generate_key(region, node, counter_value);
	leveldb::Slice key = shared_key;
	db->Put(write_options, key, value);

	return shared_key;
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

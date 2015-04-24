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
leveldb::Slice metadata_key = "metadata"; // reserved key
leveldb::Slice psize_key = "psize"; // reserved key (for primary size, in bytes)
string lclock_key = "lc"; // reserved key (for logical clock)
int counter_value = 0;
long long psize_value = 0;
long long size = 0; // size per shard

mutex put_mutex;

string log_filepath = "../data/app.log";

void initDB(string path, int shard_size) {
	leveldb::Options options;
	options.create_if_missing = true;
	status = leveldb::DB::Open(options, path, &db);
	assert(status.ok());
	cout << status.ToString() << endl;

	size = (long long) shard_size * 943718; // (1 MB size, allow 10% free for other operations)

	leveldb::Status local_status;
    string temp_value;
	local_status = db->Get(read_options, counter_key, &temp_value);
	if (!local_status.ok()) { // initialize counter if it hasn't been initialized
		db->Put(write_options, counter_key, to_string(counter_value));
	} else {
		counter_value = stoi(temp_value);
	}

	local_status = db->Get(read_options, psize_key, &temp_value);
	if (!local_status.ok()) { // initialize metadata if it hasn't been initialized
		db->Put(write_options, psize_key, to_string(psize_value));
	} else {
		psize_value = stoll(temp_value);
	}

	write_options2.sync = true;
}

// Retrieve metadata version (return -1 if not set)
int getMetadataValue() {
	leveldb::Status local_status;
	string temp_value;
	local_status = db->Get(read_options, metadata_key, &temp_value);
	if (!local_status.ok()) {	// initialize metadata if it hasn't been initialized
		db->Put(write_options, metadata_key, "0");
		return -1;
	} else {
		return stoi(temp_value);
	}
}

// Update metadata version
void putMetadataValue(int version) {
	db->Put(write_options, metadata_key, to_string(version));
}

// Retrieve logical clock counter
long long getLClock(const string key) {
	leveldb::Status local_status;
    string temp_value;
    string t_key = lclock_key + key;
	leveldb::Slice l_key = t_key;
	local_status = db->Get(read_options, l_key, &temp_value);
	if (!local_status.ok()) { // initialize lclock if it hasn't been initialized
		db->Put(write_options, l_key, "0");
		return 0;
	} else {
		return stoll(temp_value);
	}
}

// Update logical clock counter
void putLClock(const string key, long long logical_clock) {
    string t_key = lclock_key + key;
	leveldb::Slice l_key = t_key;
	db->Put(write_options, l_key, to_string(logical_clock));
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

pair<int, int> parse_key(const string key) {
	pair<int, int> data;
	data.first = atoi(key.substr(0, 4).c_str());
	data.second = atoi(key.substr(4, 4).c_str());
	return data;
}

string putDB(const string value, int region, int node, bool force) {
	string shared_key;

	long long additional_size = 16 + value.length();

	if (!force) { // TODO: Create allow put flag
		if ((psize_value % size) + additional_size > size) return "";
	}

	put_mutex.lock();
	counter_value++;
	// increment counter's value
	db->Put(write_options, counter_key, to_string(counter_value));
	psize_value += additional_size;
	// increment psize's value
	db->Put(write_options, psize_key, to_string(psize_value));
	put_mutex.unlock();

	shared_key = generate_key(region, node, counter_value);
	leveldb::Slice key = shared_key;
	db->Put(write_options, key, value);

	return shared_key;
}

bool updateDB(const string key, const string value) {
	leveldb::Slice db_key = key;
	leveldb::Status local_status;
	string result;
	long long data_size;

	local_status = db->Get(read_options, db_key, &result);
	if (!local_status.ok()) return false; // non-existing key
	data_size = result.length();

	local_status = db->Put(write_options, db_key, value);
	if (!local_status.ok()) return false; // failure in update

	put_mutex.lock();
	psize_value = psize_value + value.length() - data_size;
	if (psize_value > size) psize_value = size; // set to limit
	// update psize's value
	db->Put(write_options, psize_key, to_string(psize_value));
	put_mutex.unlock();

	return true;
}

string getDB(const string key) {
	leveldb::Slice db_key = key;
	leveldb::Status local_status;
	string result;
	local_status = db->Get(read_options, db_key, &result);
	if (!local_status.ok()) return ""; // non-existing key
	return result;
}

bool deleteDB(const string key) {
	leveldb::Slice db_key = key;
	leveldb::Status local_status;
	local_status = db->Delete(write_options, db_key);
	if (!local_status.ok()) return false; // error in delete

    string t_key = lclock_key + key;
	leveldb::Slice l_key = t_key;
	db->Delete(write_options, l_key); // remove logical clock

	return true;
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

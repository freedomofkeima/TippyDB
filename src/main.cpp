#include <assert.h>
#include <iostream>
#include "leveldb/comparator.h"
#include "leveldb/db.h"

using namespace std;

int main() {
	leveldb::DB* db;
	leveldb::Options options;
	options.create_if_missing = true;
	leveldb::Status status = leveldb::DB::Open(options, "/tmp/testdb", &db);
	assert(status.ok());
	cout << status.ToString() << endl;

	/** Write a pair of key and value to database */
	leveldb::Slice key = "IF3230";
	string value = "This is a test string.";
	leveldb::WriteOptions write_options;
	// write_options.sync = true;
	status = db->Put(write_options, key, value);

	/** Read non-existing key */
	leveldb::Slice dummy_key = "dummy";
	leveldb::ReadOptions read_options;
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

	/** Compare */
	leveldb::Slice a = std::string("aaaaa");
	leveldb::Slice b = std::string("aaaa");
	cout << options.comparator->Compare(a, b) << endl;
	
	assert(it->status().ok()); // Check for any errors during scan
	delete it;

	return 0;
}

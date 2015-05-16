/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * DBLocalTestcase.cpp
  *
  */

#include <boost/lexical_cast.hpp>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <string>

#include "leveldb/comparator.h"
#include "leveldb/db.h"

using namespace std;

long long max_iteration, counter;
uint64_t t1, t2, total, one_second, one_us;
// Constants, the minimum number of cycles required for calling RDTSC_START and RDTSC_STOP
uint64_t rdtscp_cycle = 50;

/** This benchmarking code is adapted from https://idea.popcount.org/2013-01-28-counting-cycles---rdtsc/ */
#ifdef __i386__
#  define RDTSC_DIRTY "%eax", "%ebx", "%ecx", "%edx"
#elif __x86_64__
#  define RDTSC_DIRTY "%rax", "%rbx", "%rcx", "%rdx"
#else
# error unknown platform
#endif

#define RDTSC_START(cycles)								\
	do {												   \
		register unsigned cyc_high, cyc_low;			   \
		asm volatile("CPUID\n\t"						   \
					 "RDTSC\n\t"						   \
					 "mov %%edx, %0\n\t"				   \
					 "mov %%eax, %1\n\t"				   \
					 : "=r" (cyc_high), "=r" (cyc_low)	 \
					 :: RDTSC_DIRTY);					  \
		(cycles) = ((uint64_t)cyc_high << 32) | cyc_low;   \
	} while (0)

#define RDTSC_STOP(cycles)								 \
	do {												   \
		register unsigned cyc_high, cyc_low;			   \
		asm volatile("RDTSCP\n\t"						  \
					 "mov %%edx, %0\n\t"				   \
					 "mov %%eax, %1\n\t"				   \
					 "CPUID\n\t"						   \
					 : "=r" (cyc_high), "=r" (cyc_low)	 \
					 :: RDTSC_DIRTY);					  \
		(cycles) = ((uint64_t)cyc_high << 32) | cyc_low;   \
	} while(0)

void print_result(uint64_t cycle) {
	cout << "Number of iteration: " << max_iteration << endl;
	printf("Average number of cycles: %.2f cycles\n", ((double) cycle / max_iteration));
	printf("Average elapsed time: %.8f us\n\n", ((double) cycle / one_us) / max_iteration);
}

void init() {
	RDTSC_START(t1);
	usleep(1000000); // sleep for 1 second
	RDTSC_STOP(t2);
	one_second = t2 - t1 - rdtscp_cycle;
	cout << "Approximate number of cycles in 1 second: " << one_second << endl;
	one_us = one_second / 1e6;
}

int main(int argc, char** argv) {
  if (argc != 2) {
	cout << "Usage: ./application_name db_path" << endl;
	return 1;
  }
  cout << "DB path: " << argv[1] << endl;

  init();

  leveldb::DB* db;
  leveldb::Options options;
  leveldb::WriteOptions write_options;
  leveldb::WriteOptions write_options2;
  write_options2.sync = true;
  leveldb::ReadOptions read_options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, argv[1], &db);
  assert(status.ok());
  cout << status.ToString() << endl;
  // leveldb::Slice key = "";
  string value;

  cout << "** Starting local benchmarking **" << endl << endl;
  cout << "Key length: " << 10 << " byte(s)" << endl;
  cout << "Value length: " << 10 << " byte(s)" << endl << endl;
  max_iteration = 10;

  /** FILL (local) operation **/
  cout << "--FILL (local)--" << endl;
  counter = 10000000; total = 0;
  while (counter < max_iteration + 10000000) {
	value = "val" + boost::lexical_cast<std::string>(counter);
	leveldb::Slice key("key" + boost::lexical_cast<std::string>(counter));
	// cout << "Key: " << key.ToString() << " ; " << "Value: " << value << endl;
	RDTSC_START(t1); // start operation
	status = db->Put(write_options, key, value);
	RDTSC_STOP(t2); // stop operation
	total += t2 - t1 - rdtscp_cycle;
	counter++;
  }
  print_result(total);
  /** End of FILL (local) operation **/

  usleep(50000); // cooldown 50 ms

  /** FILLSYNC (local) operation **/
  cout << "--FILLSYNC (local)--" << endl;
  counter = 10000000; total = 0;
  while (counter < max_iteration + 10000000) {
	value = "val" + boost::lexical_cast<std::string>(counter);
	leveldb::Slice key("keys" + boost::lexical_cast<std::string>(counter));
	// cout << "Key: " << key.ToString() << " ; " << "Value: " << value << endl;
	RDTSC_START(t1); // start operation
	status = db->Put(write_options2, key, value);
	RDTSC_STOP(t2); // stop operation
	total += t2 - t1 - rdtscp_cycle;
	counter++;
  }
  print_result(total);
  /** End of FILLSYNC (local) operation **/

  usleep(50000); // cooldown 50 ms

  /** OVERWRITE (local) operation **/
  cout << "--OVERWRITE (local)--" << endl;
  counter = 10000000; total = 0;
  while (counter < max_iteration + 10000000) {
	value = "val" + boost::lexical_cast<std::string>(counter);
	leveldb::Slice key("key" + boost::lexical_cast<std::string>(counter));
	// cout << "Key: " << key.ToString() << " ; " << "Value: " << value << endl;
	RDTSC_START(t1); // start operation
	status = db->Put(write_options, key, value);
	RDTSC_STOP(t2); // stop operation
	total += t2 - t1 - rdtscp_cycle;
	counter++;
  }
  print_result(total);
  /** End of OVERWRITE (local) operation **/

  usleep(50000); // cooldown 50 ms

  /** READ (local) operation **/
  cout << "--READ (local)--" << endl;
  counter = 10000000; total = 0;
  while (counter < max_iteration + 10000000) {
	leveldb::Slice key("key" + boost::lexical_cast<std::string>(counter));
	RDTSC_START(t1); // start operation
	status = db->Get(read_options, key, &value);
	RDTSC_STOP(t2); // stop operation
	// cout << "Key: " << key.ToString() << " ; " << "Value: " << value << endl;
	total += t2 - t1 - rdtscp_cycle;
	counter++;
  }
  print_result(total);
  /** End of READ (local) operation **/

  usleep(50000); // cooldown 50 ms

  /** DELETE (local) operation **/
  cout << "--DELETE (local)--" << endl;
  counter = 10000000; total = 0;
  while (counter < max_iteration + 10000000) {
	leveldb::Slice key("key" + boost::lexical_cast<std::string>(counter));
	// cout << "Key: " << key.ToString() << endl;
	RDTSC_START(t1); // start operation
	status = db->Delete(write_options, key);
	RDTSC_STOP(t2); // stop operation
	total += t2 - t1 - rdtscp_cycle;
	counter++;
  }
  print_result(total);
  /** End of DELETE (local) operation **/

  return 0;
}

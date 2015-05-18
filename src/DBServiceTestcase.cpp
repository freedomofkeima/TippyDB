/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * DBServiceTestcase.cpp
  *
  */

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include <boost/lexical_cast.hpp>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <string>

#include "./gen-cpp/DBService.h"

/**
  * TEST_PERFORMANCE: Check for Service Performance (1 = Yes, 0 = No)
  * TEST_CORRECTNESS: Check for Service Correctness (1 = Yes, 0 = No)
  */
#define TEST_PERFORMANCE 1
#define TEST_CORRECTNESS 1

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

using namespace rapidjson;

using namespace dbservice;

long long max_iteration, max_iteration2, counter;
uint64_t t1, t2, total, one_second, one_us;
vector<string> keys; // store list of sharded key
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

void print_result(uint64_t cycle, long long _max_iteration) {
	cout << "Number of iteration: " << _max_iteration << endl;
	printf("Average number of cycles: %.2f cycles\n", ((double) cycle / _max_iteration));
	printf("Average elapsed time: %.8f us\n\n", ((double) cycle / one_us) / _max_iteration);
}

string generate_value(int length) { // create a string with length bytes size
	string ret = "";
	for (int i = 1; i <= length; i++) {
		ret += "a";
	}
	return ret;
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
  if (argc != 3) {
	cout << "Usage: ./application_name ip_address port_number" << endl;
	return 1;
  }
  cout << "IP Address: " << argv[1] << endl;
  cout << "Port number: " << argv[2] << endl;

  boost::shared_ptr<TTransport> socket(new TSocket(argv[1], atoi(argv[2])));
  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  DBServiceClient client(protocol);

  init();

  cout << "** Starting the client **" << endl << endl;
  try {
	transport->open();
	max_iteration = 10;
	max_iteration2 = 100000;

	if (TEST_CORRECTNESS) {

		/** PUTDATA operation */
		cout << "--PUTDATA (correctness)--" << endl;
		Data d;
		d.value = "{key :\"dummy\", value: \"test\"}";
		cout << d.value << endl;
		client.putData(d.key, d.value);
		cout << "Sharded key: " << d.key << endl;
		/** End of PUTDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** PUTDATA (FORCE) operation */
		cout << "--PUTDATA FORCE (correctness)--" << endl;
		client.putDataForce(d.key, d.value, 2, 1);
		cout << "Sharded key: " << d.key << endl;
		/** End of PUTDATA (FORCE) operation **/

		usleep(50000); // cooldown 50 ms

		/** DELETEDATA operation */
		cout << "--DELETEDATA (correctness)--" << endl;
		bool isSuccess = client.deleteData(d.key);
		if (isSuccess) cout << "Delete OK" << endl;
		else cout << "Delete FAILED" << endl;
		/** End of DELETEDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** UPDATEDATA operation */
		cout << "--UPDATEDATA (correctness)--" << endl;
		d.key = "0001000100000001";
		cout << "Sharded key: " << d.key << endl;
		d.value = "{key :\"dummy\", value: \"test2\"}";
		cout << "New value: " << d.value << endl;
		isSuccess = client.updateData(d);
		if (isSuccess) cout << "Update OK" << endl;
		else cout << "Update FAILED" << endl;
		/** End of UPDATEDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** GETDATA operation */
		cout << "--GETDATA (correctness)--" << endl;
		cout << "Sharded key: " << d.key << endl;
		client.getData(d.value, d.key);
		if (d.value != "") cout << "New value: " << d.value << endl;
		else cout << "Get FAILED" << endl;
		/** End of GETDATA operation **/

		/** RESYNCDATA operation **/
		cout << "--RESYNCDATA (correctness)--" << endl;
		ShardContent ds;
		client.resyncData(ds, 1, 1);
		cout << "Size: " << ds.data.size() << endl;
		/** End of RESYNCDATA operation **/

	}

	usleep(50000); // cooldown 50 ms

	if (TEST_PERFORMANCE) {

		Data d;
		d.value = generate_value(20); // create a dummy value

		/** PING operation **/
		cout << "--PING--" << endl;
		counter = 0; total = 0;
		while (counter < max_iteration) {
			RDTSC_START(t1); // start operation
			client.ping();
			RDTSC_STOP(t2); // stop operation
			total += t2 - t1 - rdtscp_cycle;
			counter++;
		}
		print_result(total, max_iteration);
		/** End of PING operation **/

		usleep(50000); // cooldown 50 ms

		/** ZIP operation **/
		cout << "--ZIP (oneway sending)--" << endl;
		counter = 0; total = 0;
		while (counter < max_iteration) {
			RDTSC_START(t1); // start operation
			client.zip();
			RDTSC_STOP(t2); // stop operation
			total += t2 - t1 - rdtscp_cycle;
			counter++;
		}
		print_result(total, max_iteration);
		/** End of ZIP operation **/

		usleep(50000); // cooldown 50 ms

		/** PUTDATA operation **/
	    cout << "--PUTDATA--" << endl;
	    counter = 0; total = 0;
	    while (counter < max_iteration2) {
		  RDTSC_START(t1); // start operation
		  client.putData(d.key, d.value);
		  RDTSC_STOP(t2); // stop operation
		  keys.push_back(d.key); // append sharded key list for testing
		  total += t2 - t1 - rdtscp_cycle;
		  counter++;
		  if (counter % (max_iteration2 / 100) == 0) cout << "..." << (counter * 100) / (max_iteration2) << "%" << endl;
	    }
	    print_result(total, max_iteration2);
		/** End of PUTDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** UPDATEDATA operation **/
	    cout << "--UPDATEDATA--" << endl;
	    counter = 0; total = 0;
	    while (counter < max_iteration2) {
		  d.key = keys[counter];
		  RDTSC_START(t1); // start operation
		  client.updateData(d);
		  RDTSC_STOP(t2); // stop operation
		  total += t2 - t1 - rdtscp_cycle;
		  counter++;
		  if (counter % (max_iteration2 / 100) == 0) cout << "..." << (counter * 100) / (max_iteration2) << "%" << endl;
	    }
	    print_result(total, max_iteration2);
		/** End of UPDATEDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** GETDATA operation **/
	    cout << "--GETDATA--" << endl;
	    counter = 0; total = 0;
	    while (counter < max_iteration2) {
		  d.key = keys[counter];
		  RDTSC_START(t1); // start operation
		  client.getData(d.value, d.key);
		  RDTSC_STOP(t2); // stop operation
		  total += t2 - t1 - rdtscp_cycle;
		  counter++;
		  if (counter % (max_iteration2 / 100) == 0) cout << "..." << (counter * 100) / (max_iteration2) << "%" << endl;
	    }
	    print_result(total, max_iteration2);
		/** End of GETDATA operation **/

		usleep(50000); // cooldown 50 ms

		/** DELETEDATA operation **/
	    cout << "--DELETEDATA--" << endl;
	    counter = 0; total = 0;
	    while (counter < max_iteration2) {
		  d.key = keys[counter];
		  RDTSC_START(t1); // start operation
		  client.deleteData(d.key);
		  RDTSC_STOP(t2); // stop operation
		  total += t2 - t1 - rdtscp_cycle;
		  counter++;
		  if (counter % (max_iteration2 / 100) == 0) cout << "..." << (counter * 100) / (max_iteration2) << "%" << endl;
	    }
	    print_result(total, max_iteration2);
		/** End of DELETEDATA operation **/

	}

  } catch (TException& tx) {
	cout << "ERROR: " << tx.what() << endl;
  }

  cout << endl;
  cout << "** Done **" << endl;

  return 0;
}




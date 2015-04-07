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

#include <boost/lexical_cast.hpp>
#include <assert.h>
#include <iostream>
#include <unistd.h>
#include <cstring>

#include "./gen-cpp/DBService.h"

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;

using boost::shared_ptr;

using namespace dbservice;

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

#define RDTSC_START(cycles)                                \
    do {                                                   \
        register unsigned cyc_high, cyc_low;               \
        asm volatile("CPUID\n\t"                           \
                     "RDTSC\n\t"                           \
                     "mov %%edx, %0\n\t"                   \
                     "mov %%eax, %1\n\t"                   \
                     : "=r" (cyc_high), "=r" (cyc_low)     \
                     :: RDTSC_DIRTY);                      \
        (cycles) = ((uint64_t)cyc_high << 32) | cyc_low;   \
    } while (0)

#define RDTSC_STOP(cycles)                                 \
    do {                                                   \
        register unsigned cyc_high, cyc_low;               \
        asm volatile("RDTSCP\n\t"                          \
                     "mov %%edx, %0\n\t"                   \
                     "mov %%eax, %1\n\t"                   \
                     "CPUID\n\t"                           \
                     : "=r" (cyc_high), "=r" (cyc_low)     \
                     :: RDTSC_DIRTY);                      \
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
  shared_ptr<TTransport> socket(new TSocket("localhost", 9090));
  shared_ptr<TTransport> transport(new TBufferedTransport(socket));
  shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
  DBServiceClient client(protocol);

  init();

  cout << "** Starting the client **" << endl << endl;
  try {
    transport->open();
	max_iteration = 10;

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
    print_result(total);
    /** End of PING operation **/

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
    print_result(total);
    /** End of ZIP operation **/

    /** PUTDATA operation */
    cout << "--PUTDATA (correctness)--" << endl;
    string result;
    cout << "Key: dummy ; Value: test" << endl;
    Data d;
    d.key = "dummy";
    d.value = "test";
    client.putData(result, d);
    cout << "Sharded key: " << result << endl;
    /** End of PUTDATA operation **/

  } catch (TException& tx) {
    cout << "ERROR: " << tx.what() << endl;
  }

  cout << endl;
  cout << "** Done **" << endl;

  return 0;
}




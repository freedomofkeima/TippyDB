/**
  * Iskandar Setiadi 13511073@std.stei.itb.ac.id
  * Institut Teknologi Bandung (ITB) - Indonesia
  * Final Project (c) 2015
  * http://freedomofkeima.com/
  * MongoDBLocalTestcase.cpp
  *
  */

#include <cstdlib>
#include <iostream>
#include "mongo/client/dbclient.h"

using namespace std;

void run() {
	mongo::DBClientConnection c;
	c.connect("localhost");
}

int main() {
	mongo::client::initialize();
	try {
		run();
		cout << "connected ok" << endl;
	} catch(const mongo::DBException &e) {
		cout << "caught " << e.what() << endl;
	}
	return 0;
}

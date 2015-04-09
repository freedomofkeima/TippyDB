#!/usr/bin/python

# Iskandar Setiadi 13511073@std.stei.itb.ac.id
# Institut Teknologi Bandung (ITB) - Indonesia
# Final Project (c) 2015
# mongodb_testcase.py

__author__ = 'freedomofkeima'

import sys
from pymongo import MongoClient

### Create test data
SEED_DATA = [
	{
		'title': 'Hello world'
	}
]

def main(args):
	client = MongoClient('localhost', 27017)
	db = client['tests-database']
	tests = db['tests-collection']
	tests.insert_many(SEED_DATA)
	cursor = tests.find()
	for doc in cursor:
		print ('Title: %s' % (doc['title']))
	client.close()

if __name__ == '__main__':
	main(sys.argv[1:])

#!/usr/bin/python

# Iskandar Setiadi 13511073@std.stei.itb.ac.id
# Institut Teknologi Bandung (ITB) - Indonesia
# Final Project (c) 2015
# mongodb_testcase.py

__author__ = 'freedomofkeima'

import sys
import time
from pymongo import MongoClient

def main(args):
    client = MongoClient('52.74.132.58', 27017) # Nearest Server location
    db = client['tests_database']
    tests = db['tests_collection']
    max_iteration = 2000
    key_size = 10
    value_size = 100 * 1024

    print '** Starting benchmarking **'
    print '** Length key + value: %d byte(s)**' % (key_size + value_size)
    
    item_id = []
    
    print '--EMPTY TIMER--'
    tx = 0 # time counter
    counter = 0
    while counter < max_iteration:
        t0 = time.time()
        tx = tx + (time.time() - t0)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    empty_timer = tx / max_iteration * 1000000
    print 'Average elapsed time: %.10f us' % (empty_timer)
    
    print '--INSERT--'
    tx = 0 # time counter
    counter = 0
    while counter < max_iteration:
        value = "a" * value_size
        data = {}
        data['mongodbval'] = value
        t0 = time.time()
        id = tests.insert_one(data).inserted_id
        tx = tx + (time.time() - t0)
        item_id.append(id)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    print 'Average elapsed time: %.10f us' % (tx / max_iteration * 1000000 - empty_timer)

    client.close()

if __name__ == '__main__':
    main(sys.argv[1:])

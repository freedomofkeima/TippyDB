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
    client = MongoClient('localhost', 27017)
    db = client['tests_database']
    tests = db['tests_collection']
    max_iteration = 10 # Change to 100000
    key_size = 10
    value_size = 10

    print '** Starting local benchmarking **'
    print '** Length key + value: %d byte(s)**' % (key_size + value_size)
    
    item_id = []
    
    print '--EMPTY TIMER--'
    tx = 0 # time counter
    counter = 10000000
    while counter < max_iteration + 10000000:
        t0 = time.time()
        tx = tx + (time.time() - t0)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    empty_timer = tx / max_iteration * 1000000
    print 'Average elapsed time: %.10f us' % (empty_timer)
    
    print '--INSERT (local)--'
    tx = 0 # time counter
    counter = 10000000
    while counter < max_iteration + 10000000:
        value = 'val' + str(counter)
        data = {}
        data['mongodbkey'] = value
        t0 = time.time()
        id = tests.insert_one(data).inserted_id
        tx = tx + (time.time() - t0)
        item_id.append(id)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    print 'Average elapsed time: %.10f us' % (tx / max_iteration * 1000000 - empty_timer)
    
    print '--UPDATE (local)--'
    tx = 0 # time counter
    counter = 10000000
    for item in item_id:
        t0 = time.time()
        tests.update_one({"_id": item}, {'$set': {'mongodbkey' : ('upd' + str(counter))}})
        tx = tx + (time.time() - t0)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    print 'Average elapsed time: %.10f us' % (tx / max_iteration * 1000000 - empty_timer)
    
    print '--READ (local)--'
    tx = 0 # time counter
    counter = 10000000
    for item in item_id:
        t0 = time.time()
        res = tests.find_one({"_id": item})
        tx = tx + (time.time() - t0)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    print 'Average elapsed time: %.10f us' % (tx / max_iteration * 1000000 - empty_timer)
    
    print '--DELETE(local)--'
    tx = 0 # time counter
    counter = 10000000
    for item in item_id:
        t0 = time.time()
        tests.delete_one({"_id": item})
        tx = tx + (time.time() - t0)
        counter = counter + 1
    print 'Number of iteration: %d' % (max_iteration)
    print 'Average elapsed time: %.10f us' % (tx / max_iteration * 1000000 - empty_timer)
    
    client.close()

if __name__ == '__main__':
    main(sys.argv[1:])

# genHashes.py
#
# Adds the needed hashes in the redis database to allow gendb to create redis.db
#
# Copyright 2018 by Northwestern University
# Author: Keith Brister
#
import redis
import platform
import sys
import mung
import orange
import kiwi
import mango

r1 = redis.Redis()

keyBase = r1.hget( 'config.%s' % (platform.node().lower()), 'HEAD')
redisMaster = r1.hget( 'config.%s' % (platform.node().lower()), 'MASTER')

r = redis.Redis(redisMaster)

configList = {}
configList["stns.1"] = mung.configList["stns.1"]
configList["stns.2"] = orange.configList["stns.2"]
configList["stns.3"] = kiwi.configList["stns.3"]
configList["stns.4"] = mango.configList["stns.4"]

for obj in configList[keyBase]:
    k = keyBase + '.' + obj["key"]
    
    r.hdel( k, ['DTYP', 'OUT_RECORD', 'INP', 'ZNAM', 'ONAM', 'OUT_SCAN', 'OUT_PV', 'SETTER', 'IN_RECORD', 'OUT', 'IN_PV', 'OUT_DTYP', 'IN_DTYP'])

for obj in configList[keyBase]:
    k = keyBase + '.' + obj["key"]
    
    if obj.has_key( 'ro'):
        r.hset(k, 'OUT_DTYP', obj["dtyp"])
        r.hset(k, 'OUT_RECORD', obj["ro"])

        if obj.has_key( 'out'):
            r.hset(k, 'OUT', obj["out"])

        if obj.has_key( 'prec'):
            r.hset(k, 'PREC', obj["prec"])
            
        if obj.has_key( 'znam'):
            r.hset(k, 'ZNAM', obj["znam"])

        if obj.has_key( 'onam'):
            r.hset(k, 'ONAM', obj["onam"])

        r.hset(k, 'OUT_SCAN', obj["oscan"])

        if obj.has_key( 'pv'):
            r.hset(k, 'OUT_PV', obj["pv"])
        else:
            r.hset(k, 'OUT_PV', k.replace( '.', ':'))

    if obj.has_key( 'ri'):
        if obj.has_key( 'setter'):
            r.hset(k, 'SETTER', obj["setter"])
    
        r.hset(k, 'IN_DTYP', obj["dtyp"])
        r.hset(k, 'IN_RECORD', obj["ri"])

        if obj.has_key( 'inp'):
            r.hset(k, 'INP', obj["inp"])
            
        if obj.has_key( 'prec'):
            r.hset(k, 'PREC', obj["prec"])

        if obj.has_key( 'znam'):
            r.hset(k, 'ZNAM', obj["znam"])

        if obj.has_key( 'onam'):
            r.hset(k, 'ONAM', obj["onam"])

        r.hset(k, 'IN_SCAN', obj["iscan"])

        if obj.has_key( 'pv'):
            r.hset(k, 'IN_PV', obj["pv"])
        else:
            r.hset(k, 'IN_PV', obj["key"].replace( '.', ':'))

    r.hsetnx(k, 'VALUE', "")

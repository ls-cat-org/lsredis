#!../../bin/linux-x86_64/redis

## You may have to change redis to something else
## everywhere it appears in this file

#< envPaths
epicsEnvSet("TOP","/usr/local/epics/synApps_5_7/support/lsredis")

## Register all support components
dbLoadDatabase("$(TOP)/dbd/redis.dbd",0,0)
redis_registerRecordDeviceDriver(pdbbase) 


dbLoadRecords("$(TOP)/db/redis.db","P=21:mango,S=stns.4,RRH=localhost,RRP=6379,RRDB=0,RWH=mango-2.ls-cat.net,RWP=6379,RWDB=0,pgHost=10.1.0.30,pgPort=5432,pgUser=lsuser,pgDb=ls")


iocInit()

#include "lsRedis.h"

/**
 ** We need an init routine to support the value records but there is
 ** literally nothing to do.  Except return 0 (meaning everything is
 ** AOK).  So I guess that's something.
 */
static long value_init(int phase) {
  return 0;
}


/** Initialize our redis value record
 */
static long value_init_longin_record( aoRecord *prec) {
  return value_init_record( (dbCommon *)prec, 0);
}

/** Tell epics we like i/o intr so much we are willing to tell it how.
 */
static long value_get_ioint_info( int dir, dbCommon* prec, IOSCANPVT* io) {
  redisValueState* rvs;

  rvs = prec->dpvt;
  *io = rvs->in_scan;
  return 0;
}



/** Send our value to redis
 **
 */
static long ca_read_longin( longinRecord *prec) {
  static char *id = "ca_read_longin";
  epicsInt32 ourVal;
  char tmp[128];
  char pgtmp[128];
  redisValueState *rvs;

  rvs = prec->dpvt;
  if( rvs == NULL)
    return 1;

  dbGetLink( &prec->inp, DBR_LONG, &ourVal, NULL, NULL);

  fprintf( stderr, "%s: %s = %d\n", id, prec->name, ourVal);

  if( !prec->udf && prec->val == ourVal)
    return 2;

  prec->val = ourVal;
  prec->udf = 0;

  epicsMutexMustLock( rvs->lock);
  if( strcmp( rvs->setter, "redis") == 0) {
    snprintf( tmp, sizeof(tmp)-1, "%d", ourVal);
    tmp[sizeof(tmp)-1] = 0;
  }

  if( strcmp( rvs->setter, "kvset") == 0) {
    snprintf( pgtmp, sizeof( pgtmp)-1, "select px.kvsetioc( '%s', '%d')", rvs->redisKey, ourVal);
    pgtmp[sizeof(pgtmp)-1] = 0;
  }
  epicsMutexUnlock( rvs->lock);

  //
  // Redis Acync callbacks not needed here because our subscriber
  // process will pick up the publication notice and mark this record
  // as no longer being active by setting pact = 0.
  //
  if( strcmp( rvs->setter, "redis") == 0) {
    setRedis( rvs, tmp);

    // TODO: see note for AI support
    prec->pact = 0;		// Should set back to zero when we see that redis has published our new value. TODO
  }
  
  if( strcmp( rvs->setter, "kvset") == 0) {
    lsRedisSendQuery( rvs->rs, pgtmp);
    prec->pact = 0;		// TODO: set to one here and back to zero when PG acts on (or at least sees) the command
  }
  return 2;
}

/** Send our val
 **
 */
static long val_read_longin( longinRecord *prec) {
  //  static char *id = "val_read_longin";
  char tmp[128];
  char pgtmp[128];
  redisValueState *rvs;
  int dontSet;

  rvs = prec->dpvt;
  if( rvs == NULL)
    return 1;

  dontSet = 0;

  epicsMutexMustLock( rvs->lock);
  if( prec->udf) {
    prec->val = strtol( rvs->stringVal, NULL, 0);
    dontSet = 1;
  } else {    
    if( strcmp( rvs->setter, "redis") == 0) {
      snprintf( tmp, sizeof(tmp)-1, "%d", prec->val);
      tmp[sizeof(tmp)-1] = 0;
    }
    
    if( strcmp( rvs->setter, "kvset") == 0) {
      snprintf( pgtmp, sizeof( pgtmp)-1, "select px.kvsetioc( '%s', '%d')", rvs->redisKey, prec->val);
      pgtmp[sizeof(pgtmp)-1] = 0;
    }
  }
  prec->udf = 0;
  epicsMutexUnlock( rvs->lock);

  if( dontSet)
    return 2;

  //
  // Redis Acync callbacks not needed here because our subscriber
  // process will pick up the publication notice and mark this record
  // as no longer being active by setting pact = 0.
  //
  if( strcmp( rvs->setter, "redis") == 0) {
    setRedis( rvs, tmp);

    // TODO: see note for AI support
    prec->pact = 0;		// Set back to one when we see that redis has published our new value
  }
  
  if( strcmp( rvs->setter, "kvset") == 0) {
    lsRedisSendQuery( rvs->rs, pgtmp);
    prec->pact = 0;		// TODO: set to one here and back to zero when PG acts on (or at least sees) the command
  }
  return 2;
}


struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_longin;
} devLonginCASource = {
  5, /* space for 5 functions */
  NULL,
  value_init,
  value_init_longin_record,
  value_get_ioint_info,
  ca_read_longin
};

epicsExportAddress(dset,devLonginCASource);

struct {
  long num;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  get_ioint_info;
  DEVSUPFUN  read_longin;
} devLonginVALSource = {
  5, /* space for  functions */
  NULL,
  value_init,
  value_init_longin_record,
  value_get_ioint_info,
  val_read_longin
};

epicsExportAddress(dset,devLonginVALSource);


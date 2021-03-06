/*
  devSnmp.cpp - EPICS device support for SNMP

  Original DESY 1.03 version substantially re-written for NSCL
  by J.Priller
*/

/* Here we add DESY patch to convert Count32 (value from
0 to 2^32-1)  to long (range -(2^31 -1) to (2^31 -1) ) */
// #define DESY_PATCH

// undefine items below NOT set given record type val field on read error
#define AI_ON_READ_ERROR_VALUE  -999.999
#define LI_ON_READ_ERROR_VALUE  -999
#define SI_ON_READ_ERROR_VALUE  "INVALID"
// (we do NOT return weird values when output records have error, so that
//  weird values aren't stored and then re-applied by IOC bumpless reboot)

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "devSnmp.h"

#include "alarm.h"
#include "cvtTable.h"
#include "dbDefs.h"
#include "dbAccess.h"
#include "recGbl.h"
#include "recSup.h"
#include "link.h"
#include "epicsExport.h"

#include <aiRecord.h>
#include <stringinRecord.h>
#include <longinRecord.h>
#include <waveformRecord.h>
#include <aoRecord.h>
#include <stringoutRecord.h>
#include <longoutRecord.h>

#define epicsOk    (0)
#define epicsError (-1)

#ifdef  DESY_PATCH
  #include "limits.h"
unsigned long  LONG_MAX_dev_10  = (LONG_MAX / 10);
unsigned long  LONG_MAX_rest_10 = (LONG_MAX % 10);
#endif // DESY_PATCH

#define GROUP_BAR  "============================================================"
#define REPORT_BAR "----------------------------------------"

// global variables
static devSnmp_manager *pManager = NULL;
static int devSnmpDebug = 0;

// internal function prototypes
extern "C" {
  static long snmpAiInit(struct aiRecord *pai);
  static long snmpAiRead(struct aiRecord *pai);
  static long snmpLiInit(struct longinRecord *pli);
  static long snmpLiRead(struct longinRecord *pli);
  static long snmpSiInit(struct stringinRecord *psi);
  static long snmpSiRead(struct stringinRecord *psi);
  static epicsStatus snmpWfInit(struct waveformRecord *pwf);
  static epicsStatus snmpWfRead(struct waveformRecord *pwf);
  static long snmpAoInit(struct aoRecord *pao);
  static long snmpAoWrite(struct aoRecord *pao);
  static long snmpAoReadback(devSnmp_device *pDevice);
  static long snmpLoInit(struct longoutRecord *plo);
  static long snmpLoWrite(struct longoutRecord *plo);
  static long snmpLoReadback(devSnmp_device *pDevice);
  static long snmpSoInit(struct stringoutRecord *pso);
  static long snmpSoWrite(struct stringoutRecord *pso);
  static long snmpSoReadback(devSnmp_device *pDevice);
}
static void checkInit(void);
static char *dup_string(char *str);
static char *snmpVersionString(long version);
static int snmpSessionCallback(int op, SNMP_SESSION *sp, int reqId, SNMP_PDU *pdu, void *magic);
static char *snmpStrStr(char *str, char *mask);
static int snmpStrCmp(char *s0, char *s1);
static int snmpReadTask(devSnmp_manager *pMgr);
static int snmpSendTask(devSnmp_manager *pMgr);

// externally-callable function prototypes
extern "C" {
  int epicsSnmpInit(int param);
  int epicsSnmpSetSnmpVersion(char *hostName, char *versionStr);
  int epicsSnmpSetMaxOidsPerReq(char *hostName, int maxoids);
  int devSnmpSetDebug(int level);
  int snmpr(int level, char *match);
  int snmpz(void);
  int snmpzr(int level, char *match);
};

//--------------------------------------------------------------------
// device support definition structures
//--------------------------------------------------------------------

// ai
struct {
  long            number;
  DEVSUPFUN       report;
  DEVSUPFUN       init;
  DEVSUPFUN       init_record;
  DEVSUPFUN       ioIntInfoGet;
  DEVSUPFUN       read_ai;
  DEVSUPFUN       special_linconv;
} devSnmpAi = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpAiInit,
  NULL,
  (DEVSUPFUN) snmpAiRead,
  NULL
};
epicsExportAddress(dset,devSnmpAi);

// longin
struct {
  long            number;
  DEVSUPFUN       report;
  DEVSUPFUN       init;
  DEVSUPFUN       init_record;
  DEVSUPFUN       ioIntInfoGet;
  DEVSUPFUN       read_li;
  DEVSUPFUN       special_linconv;
} devSnmpLi = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpLiInit,
  NULL,
  (DEVSUPFUN) snmpLiRead,
  NULL
};
epicsExportAddress(dset,devSnmpLi);


// stringin
struct {
  long            number;
  DEVSUPFUN       report;
  DEVSUPFUN       init;
  DEVSUPFUN       init_record;
  DEVSUPFUN       ioIntInfoGet;
  DEVSUPFUN       read_si;
  DEVSUPFUN       special_linconv;
} devSnmpSi = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpSiInit,
  NULL,
  (DEVSUPFUN) snmpSiRead,
  NULL
};
epicsExportAddress(dset,devSnmpSi);

// wave form
struct {
  long            number;
  DEVSUPFUN       report;
  DEVSUPFUN       init;
  DEVSUPFUN       init_record;
  DEVSUPFUN       ioIntInfoGet;
  DEVSUPFUN       read_wf;
  DEVSUPFUN       special_linconv;
} devSnmpWf = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpWfInit,
  NULL,
  (DEVSUPFUN) snmpWfRead,
  NULL
};
epicsExportAddress(dset,devSnmpWf);

// ao
struct {
  long       number;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  ioIntInfoGet;
  DEVSUPFUN  write_ao;
  DEVSUPFUN  special_linconv;
} devSnmpAo = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpAoInit,
  NULL,
  (DEVSUPFUN) snmpAoWrite,
  NULL
};
epicsExportAddress(dset,devSnmpAo);

// longout
struct {
  long       number;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  ioIntInfoGet;
  DEVSUPFUN  write_lo;
  DEVSUPFUN  special_linconv;
} devSnmpLo = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpLoInit,
  NULL,
  (DEVSUPFUN) snmpLoWrite,
  NULL
};
epicsExportAddress(dset,devSnmpLo);

// stringout
struct {
  long       number;
  DEVSUPFUN  report;
  DEVSUPFUN  init;
  DEVSUPFUN  init_record;
  DEVSUPFUN  ioIntInfoGet;
  DEVSUPFUN  write_so;
  DEVSUPFUN  special_linconv;
} devSnmpSo = {
  6,
  NULL,
  NULL,
  (DEVSUPFUN) snmpSoInit,
  NULL,
  (DEVSUPFUN) snmpSoWrite,
  NULL
};
epicsExportAddress(dset,devSnmpSo);

//--------------------------------------------------------------------
// record processing routines
//--------------------------------------------------------------------
static long snmpAiInit(struct aiRecord *pai)
{

  printf("snmpAiInit\n");

  // ai.inp must be an INST_IO
  switch (pai->inp.type) {
    case INST_IO:
        break;

    default :
        recGblRecordError(S_db_badField,(void *)pai,
                         "devSnmpAi (init_record) Illegal INP field");
        return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) pai, &pai->inp, "ai");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)pai,"devSnmpAi (init_record) bad parameters");
    return(S_db_badField);
  }

  pai->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpAiRead(struct aiRecord *pai)
{
  devSnmp_device *pDevice;
  epicsStatus status = epicsError;
  char val[40];
  double new_val;

  if (pai->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) pai->dpvt;

  status = -1;

  if (pDevice->getValueString(val,sizeof(val))) {
    if ((sscanf(val, "%lf", &new_val) == 1) && (! isnan(new_val))) {
      pai->val = new_val;
      pDevice->declareValueValid();
      status = 2;
    } else {
      status = -1;
    }
  }

  if (pDevice->validTimeout()) {
#ifdef AI_ON_READ_ERROR_VALUE
    pai->val = AI_ON_READ_ERROR_VALUE;
#endif
    status = recGblSetSevr(pai, READ_ALARM, INVALID_ALARM);
    if (status && (pai->stat != READ_ALARM || pai->sevr != INVALID_ALARM)) {
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)pai, "devSnmpAi read error");
    }
    status = -1;
  }

  return(status);
}
//--------------------------------------------------------------------
static long snmpLiInit(struct longinRecord *pli)
{
  // ai.inp must be an INST_IO
  switch (pli->inp.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)pli,
                      "devSnmpLi (init_record) Illegal INP field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) pli, &pli->inp, "longin");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)pli,"devSnmpLi (init_record) bad parameters");
    return(S_db_badField);
  }

  pli->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpLiRead(struct longinRecord *pli)
{
  devSnmp_device  *pDevice;
  epicsStatus status = epicsError;
  char val[40];
#ifdef DESY_PATCH
  int len;
#endif

  if (pli->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) pli->dpvt;

  status = 2;

  if (pDevice->getValueString(val,sizeof(val))) {
#ifdef DESY_PATCH
    {
      if (devSnmpDebug) printf("valBeforeScanf=%s\n",val);
      errno =0;
      unsigned long longVal = strtol((const char*) val,NULL,10);
      if (devSnmpDebug) printf("%s: Long=%ld err=%d\n",val,longVal,errno);
      if (errno) {
        if (devSnmpDebug) printf("strtol warning: Str=%s err=%d\n",val,errno);
        if ((len=strlen(val))==10) {
          static char bufWithoutLast[16]; memset(bufWithoutLast,0,16);
          strncpy(bufWithoutLast,val,9); bufWithoutLast[9]=0;
          errno=0;
          unsigned long longValWithoutLast=strtol(bufWithoutLast,NULL,10);
          int rest =  (int)( val[9] -'0') - LONG_MAX_rest_10;
          if (errno) {
            status = 2;
            printf("Counter32-rest for %s overflow shortVal=%s long=%ld\n",
                    val,bufWithoutLast,longValWithoutLast);
          } else {
            pli->val = 10*(longValWithoutLast-LONG_MAX_dev_10) + rest;
            pDevice->declareValueValid();
            if (devSnmpDebug) printf("%s:Patch=%d short Buf=%s Num=%ld rest=%d\n",
                                   val,pli->val,bufWithoutLast,longValWithoutLast,rest);
          }
        } else {
          printf("Counter32 deep overflow valAsString=%s len=%d\n",val,len);
          status = 2;
        }
      } else {
        pli->val = longVal;
        pDevice->declareValueValid();
      }
    }
#else  // DESY_PATCH
    if (sscanf(val, "%ld", (long*) &pli->val) == 1) {
      pDevice->declareValueValid();
      status = epicsOk;
    } else {
      status = 2;
    }
#endif // DESY_PATCH
  }

  if (pDevice->validTimeout()) {
#ifdef LI_ON_READ_ERROR_VALUE
    pli->val = LI_ON_READ_ERROR_VALUE;
#endif
    status = recGblSetSevr(pli, READ_ALARM, INVALID_ALARM);
    if (status && (pli->stat != READ_ALARM || pli->sevr != INVALID_ALARM)) {
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)pli, "devSnmpSi read error");
    }
    status = 2;
  }

  return(status);
}
//--------------------------------------------------------------------
static long snmpSiInit(struct stringinRecord *psi)
{
  // si.inp must be an VME_IO
  switch (psi->inp.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)psi,
                      "devSnmpSi (init_record) Illegal INP field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) psi, &psi->inp, "stringin");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)psi,"devSnmpSi (init_record) bad parameters");
    return(S_db_badField);
  }

  psi->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpSiRead(struct stringinRecord *psi)
{
  devSnmp_device *pDevice;
  epicsStatus status = epicsError;
  char val[40];

  if (psi->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) psi->dpvt;

  status = 2;

  if (pDevice->getValueString(val,sizeof(val))) {
    pDevice->declareValueValid();
    memset(psi->val, '\0', 40);
    strcpy(psi->val, val);
    status = epicsOk;
  }

  if (pDevice->validTimeout()) {
#ifdef SI_ON_READ_ERROR_VALUE
    strcpy(psi->val, SI_ON_READ_ERROR_VALUE);
#endif
    status = recGblSetSevr(psi, READ_ALARM, INVALID_ALARM);
    if (status && (psi->stat != READ_ALARM || psi->sevr != INVALID_ALARM)) {
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)psi, "devSnmpSi read error");
    }
    status = 2;
  }

  return(status);
}
//--------------------------------------------------------------------
static epicsStatus snmpWfInit(struct waveformRecord *pwf)
{
  epicsStatus status = epicsOk;

  // ai.inp must be an INST_IO
  switch (pwf->inp.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)pwf,
                      "devSnmpWf (init_record) Illegal INP field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) pwf, &pwf->inp, "waveform");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)pwf,"devSnmpWf (init_record) bad parameters");
    return(S_db_badField);
  }

  switch (pwf->ftvl) {
    case DBF_STRING:
    case DBF_CHAR:
    case DBF_UCHAR:
        pDevice->setDataLength((sizeof(char) * pwf->nelm) + 1);
        break;
    default:
        pDevice->setDataLength(sizeof(char) * 40);
        break;
  }

  pwf->udf = FALSE;
  return(status);
}
//--------------------------------------------------------------------
static epicsStatus snmpWfRead(struct waveformRecord *pwf)
{
  devSnmp_device *pDevice;
  long status = epicsError;
  int i;
  char *rawval = NULL;

  if (pwf->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) pwf->dpvt;

  status = 2;

  int data_sz = pDevice->getDataLength();
  char *val = new char [data_sz];

  if (pDevice->getRawValueString(val,data_sz)) {
    for (i = 0; wfTypes[i].str != NULL; i++) {
      if (!snmpStrCmp(wfTypes[i].str, (char *)rawval)) {
        if (!wfTypes[i].func(pwf->bptr, (char *)rawval)) {
          pwf->nord = 1;
#if     1
          switch (pwf->ftvl) {
            case DBF_CHAR:
            case DBF_UCHAR:
            case DBF_STRING:
                pwf->nord = strlen((char *)pwf->bptr);
                break;
#if     0
            case DBF_CHAR:
            case DBF_UCHAR:
                printf("bptr char %c\n", pwf->bptr,0,0,0,0,0);
                break;

            case DBF_SHORT:
            case DBF_USHORT:
                printf("bptr short %d\n", pwf->bptr,0,0,0,0,0);
                break;

            case DBF_LONG:
            case DBF_ULONG:
                {
                  long *lp;
                  lp = (long*) pwf->bptr;
                  printf("bptr long %ld\n", *lp,0,0,0,0,0);
                }
                break;
#endif
            default:
                break;
          }
#endif
          pDevice->declareValueValid();
          status = epicsOk;
          break;
        }
      }
    }
  }
  if (val) delete [] val;

  if (pDevice->validTimeout()) {
    status = recGblSetSevr(pwf, READ_ALARM, INVALID_ALARM);
    if (status && (pwf->stat != READ_ALARM || pwf->sevr != INVALID_ALARM)) {
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)pwf, "devSnmpSi read error");
    }
    status = 2;
  }

  return(status);
}
//--------------------------------------------------------------------
static long snmpAoInit(struct aoRecord *pao)
{
  // ao.scan must be Passive
  if (pao->scan != menuScanPassive) {
    recGblRecordError(S_db_badField,(void *)pao,
                      "devSnmpAo (init_record) SCAN must be Passive");
    return(S_db_badField);
  }

  // ao.out must be an INST_IO
  switch (pao->out.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)pao,
                      "devSnmpAo (init_record) Illegal OUT field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) pao, &pao->out, "ao");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)pao,"devSnmpAo (init_record) bad parameters");
    return(S_db_badField);
  }
  pDevice->setPollGapMSec(devSnmp_device::PassivePollGapMSec);

  /* request a periodic call to snmpAoReadback, so our display value is
     periodically updated with value from the remote host */
  pDevice->setPeriodicCallback(snmpAoReadback,(devSnmp_device::PassivePollGapMSec / 1000.0));

  pao->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpAoWrite(struct aoRecord *pao)
{
  devSnmp_device *pDevice;

  if (pao->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) pao->dpvt;

  /* if we're being called as a result of a readback update,
     just report success */
  if (pDevice->doingProcess()) return(epicsOk);

  // test to make sure value is valid
  if (isnan(pao->val)) {
    if (devSnmpDebug)
      printf("----- ERROR: AO setting %s to invalid number!\n",pao->name);
    return(epicsError);
  }

  // fill in set data
  long status = epicsOk;
  char tmp[40];
  sprintf(tmp,"%lg",pao->val);
  pDevice->set(tmp);
  if (devSnmpDebug)
    printf("----- AO setting %s to [%s] ...\n",pao->name,tmp);

  return(status);
}
//--------------------------------------------------------------------
static long snmpAoReadback(devSnmp_device *pDevice)
{
  struct aoRecord *pao = (struct aoRecord *) pDevice->record();
  epicsStatus status = epicsError;
  char val[40];
  double new_val;
  char old_str[40], new_str[40];
  bool process_record = FALSE;

  status = -1;

  if (pDevice->getValueString(val,sizeof(val))) {
    if ((sscanf(val, "%lf", &new_val) == 1) && (! isnan(new_val))) {
      status = 2;
      pDevice->declareValueValid();

      /* if it hasn't been long enough since last setting sent, skip
         checking versus our current val field (don't want our val
         field over-written if device hasn't had time to process
         setting yet) */
      if (! pDevice->wasSetRecently()) {
        /* generate strings to compare (can't just compare double
           values, as round-off errors may constantly trigger us) */
        sprintf(old_str,"%.8lg",pao->val);
        sprintf(new_str,"%.8lg",new_val);
        if (strcmp(old_str,new_str)) {
          // value read from remote host doesn't match what we think val is
          if (devSnmpDebug)
            printf("----- AO value change %s [%lg] ...\n",pao->name,new_val);
          pao->val = new_val;
          process_record = TRUE;
        }
      }

      // need to clear invalid alarm if present
      if (pao->sevr == INVALID_ALARM) {
        recGblResetAlarms(pao);
        process_record = TRUE;
      }

    } else {
      status = -1;
    }
  }

  // check valid read timeout
  if (pDevice->validTimeout()) {
    status = recGblSetSevr(pao, READ_ALARM, INVALID_ALARM);
    if (status && (pao->stat != READ_ALARM || pao->sevr != INVALID_ALARM)) {
      // trigger processing so stat/sevr is updated
      process_record = TRUE;
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)pao, "devSnmpAo read error");
    }
    status = -1;
  }

  // process record if needed
  if (process_record) pDevice->processRecord();

  return(status);
}
//--------------------------------------------------------------------
static long snmpLoInit(struct longoutRecord *plo)
{

  printf("snmpLoInit\n");

  // lo.scan must be Passive
  if (plo->scan != menuScanPassive) {
    recGblRecordError(S_db_badField,(void *)plo,
                      "devSnmpLo (init_record) SCAN must be Passive");
    return(S_db_badField);
  }

  // lo.out must be an INST_IO
  switch (plo->out.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)plo,
                      "devSnmpLo (init_record) Illegal OUT field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) plo, &plo->out, "longout");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)plo,"devSnmpLo (init_record) bad parameters");
    return(S_db_badField);
  }
  pDevice->setPollGapMSec(devSnmp_device::PassivePollGapMSec);

  /* request a periodic call to snmpLoReadback, so our display value is
     periodically updated with value from the remote host */
  pDevice->setPeriodicCallback(snmpLoReadback,(devSnmp_device::PassivePollGapMSec / 1000.0));

  plo->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpLoWrite(struct longoutRecord *plo)
{
  devSnmp_device *pDevice;

  if (plo->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) plo->dpvt;

  /* if we're being called as a result of a readback update,
     just report success */
  if (pDevice->doingProcess()) return(epicsOk);

  // fill in set data
  long status = epicsOk;
  char tmp[40];
  sprintf(tmp,"%ld", (long) plo->val);
  pDevice->set(tmp);

  return(status);
}
//--------------------------------------------------------------------
static long snmpLoReadback(devSnmp_device *pDevice)
{
  struct longoutRecord *plo = (struct longoutRecord *) pDevice->record();
  epicsStatus status = epicsError;
  char val[40];
  long new_val;
  bool process_record = FALSE;

  status = -1;

  if (pDevice->getValueString(val,sizeof(val))) {
    if (strstr(val,"undefined")) {
      /* if string contains "undefined" is a write-only channel (some Wiener
         channels do this, for example groupsSwitch.x) */
      pDevice->declareValueValid();
      status = 2;
      new_val = -1;
    } else if (sscanf(val, "%ld", &new_val) == 1) {
      pDevice->declareValueValid();
      status = 2;
    }

    if (status >= 0) {
      /* if it hasn't been long enough since last setting sent, skip
         checking versus our current val field (don't want our val
         field over-written if device hasn't had time to process
         setting yet) */
      if (! pDevice->wasSetRecently()) {
        if (new_val != plo->val) {
          // value read from remote host doesn't match what we think val is
          if (devSnmpDebug)
            printf("----- LO value change %s [%ld] ...\n",plo->name,new_val);
          plo->val = new_val;
          process_record = TRUE;
        }
      }

      // need to clear invalid alarm if present
      if (plo->sevr == INVALID_ALARM) {
        recGblResetAlarms(plo);
        process_record = TRUE;
      }

    } else {
      status = -1;
    }
  }

  if (pDevice->validTimeout()) {
    status = recGblSetSevr(plo, READ_ALARM, INVALID_ALARM);
    if (status && (plo->stat != READ_ALARM || plo->sevr != INVALID_ALARM)) {
      // trigger processing so stat/sevr is updated
      process_record = TRUE;
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)plo, "devSnmpLo read error");
    }
    status = -1;
  }

  // process record if needed
  if (process_record) pDevice->processRecord();

  return(status);
}
//--------------------------------------------------------------------
static long snmpSoInit(struct stringoutRecord *pso)
{
  // so.scan must be Passive
  if (pso->scan != menuScanPassive) {
    recGblRecordError(S_db_badField,(void *)pso,
                      "devSnmpSo (init_record) SCAN must be Passive");
    return(S_db_badField);
  }

  // so.out must be an INST_IO
  switch (pso->out.type) {
  case INST_IO:
    break;

  default :
    recGblRecordError(S_db_badField,(void *)pso,
                      "devSnmpSo (init_record) Illegal OUT field");
    return(S_db_badField);
  }

  checkInit();
  devSnmp_device *pDevice = pManager->createDevice((struct dbCommon *) pso, &pso->out, "stringout");
  if (! pDevice) {
    recGblRecordError(S_db_badField,
                      (void *)pso,"devSnmpSo (init_record) bad parameters");
    return(S_db_badField);
  }
  pDevice->setPollGapMSec(devSnmp_device::PassivePollGapMSec);

  /* request a periodic call to snmpSoReadback, so our display value is
     periodically updated with value from the remote host */
  pDevice->setPeriodicCallback(snmpSoReadback,(devSnmp_device::PassivePollGapMSec / 1000.0));

  pso->udf = FALSE;
  return(epicsOk);
}
//--------------------------------------------------------------------
static long snmpSoWrite(struct stringoutRecord *pso)
{
  devSnmp_device *pDevice;

  if (pso->dpvt == NULL)
    return(epicsError);
  else
    pDevice = (devSnmp_device *) pso->dpvt;

  /* if we're being called as a result of a readback update,
     just report success */
  if (pDevice->doingProcess()) return(epicsOk);

  // fill in set data
  long status = epicsOk;
  pDevice->set(pso->val);

  if (devSnmpDebug)
    printf("----- SO setting %s to [%s] ...\n",pso->name,pso->val);

  return(status);
}
//--------------------------------------------------------------------
static long snmpSoReadback(devSnmp_device *pDevice)
{
  struct stringoutRecord *pso = (struct stringoutRecord *) pDevice->record();
  epicsStatus status = epicsError;
  char val[40];
  char new_val[40];
  bool process_record = FALSE;

  status = -1;

  if (pDevice->getValueString(val,sizeof(val))) {
    pDevice->declareValueValid();
    strcpy(new_val,val);
    status = 2;

    /* if it hasn't been long enough since last setting sent, skip
       checking versus our current val field (don't want our val
       field over-written if device hasn't had time to process
       setting yet) */
    if (! pDevice->wasSetRecently()) {
      if (strcmp(new_val,pso->val)) {
        // value read from remote host doesn't match what we think val is
        if (devSnmpDebug)
          printf("----- SO value change %s [%s] ...\n",pso->name,new_val);
        strcpy(pso->val,new_val);
        process_record = TRUE;
      }
    }

    // need to clear invalid alarm if present
    if (pso->sevr == INVALID_ALARM) {
      recGblResetAlarms(pso);
      process_record = TRUE;
    }
  }

  if (pDevice->validTimeout()) {
    status = recGblSetSevr(pso, READ_ALARM, INVALID_ALARM);
    if (status && (pso->stat != READ_ALARM || pso->sevr != INVALID_ALARM)) {
      // trigger processing so stat/sevr is updated
      strcpy(pso->val, "INVALID");
      process_record = TRUE;
      if (devSnmpDebug)
        recGblRecordError(-1, (void *)pso, "devSnmpSo read error");
    }
    status = -1;
  }

  // process record if needed
  if (process_record) pDevice->processRecord();

  return(status);
}
//--------------------------------------------------------------------
// internal routines
//--------------------------------------------------------------------
static void checkInit(void)
{
  if (! pManager) pManager = new devSnmp_manager();
}
//--------------------------------------------------------------------
static char *dup_string(char *str)
{
  char *retval = new char[strlen(str)+1];
  strcpy(retval,str);
  return(retval);
}
//--------------------------------------------------------------------
static char *snmpVersionString(long version)
{
  switch (version) {
    case SNMP_VERSION_1:  return("SNMP_VERSION_1");
    case SNMP_VERSION_2c: return("SNMP_VERSION_2c");
    case SNMP_VERSION_3:  return("SNMP_VERSION_3");
    default:              return("unknown");
  }
}
//--------------------------------------------------------------------
static int snmpReadTask(devSnmp_manager *pMgr)
{
  return(pMgr->readTask());
}
//--------------------------------------------------------------------
static int snmpSendTask(devSnmp_manager *pMgr)
{
  return(pMgr->sendTask());
}
//--------------------------------------------------------------------
static int snmpSessionCallback(int op, SNMP_SESSION *sp, int reqId, SNMP_PDU *pdu, void *magic)
{
  // get pointer to devSnmp_session callback is for, ignore if is NULL
  devSnmp_session *pSession = (devSnmp_session *) magic;
  if (! pSession) return(epicsOk);

  // tell session to process itself, return its status
  return( pSession->replyProcessing(op,sp,reqId,pdu) );
}
//--------------------------------------------------------------------
static char *snmpStrStr(char *str, char *mask)
{
  int i, x;
  char *tmpStr = NULL;

  if ((str != NULL) && (mask != NULL)) {

    for (i = 0; *(str + i) != '\0'; i++) {
      if (*(str + i) == *mask) {
        tmpStr = str + i;
        // let '_' in mask match any char
        for (x = 0; (*(str + i) == *(mask + x) || (*(mask + x) == '_'));) {
          ++i; ++x;
          if (*(mask + x) == '\0') {
            return(tmpStr);
          } else
            continue;
        }
      }
    }
  }
  return(NULL);
}
//--------------------------------------------------------------------
static int snmpStrCmp(char *s0, char *s1)
{
  char *ts0 = s0, *ts1 = s1;

  for (; *ts0 != ':' && *ts0 && *ts1; ts0++, ts1++) {
    if (*ts0 != *ts1)
      return(1);
  }
  return(0);
}
//--------------------------------------------------------------------
static int snmpWfGauge32Convert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT32 *lp = (SNMP_UINT32*) rval;

  cp = strchr(sval, ':') + 1;
  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%ld", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfGauge64Convert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT64 *llp = (SNMP_UINT64 *) rval;

  cp = strchr(sval, ':') + 1;
  if ((cp != NULL) && (llp != NULL) && sscanf(cp, "%Ld", llp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfCntr32Convert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT32 *lp = (SNMP_UINT32*) rval;

  cp = strchr(sval, ':') + 1;
  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%ld", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfCntr64Convert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT64 *llp = (SNMP_UINT64*) rval;

  cp = strchr(sval, ':') + 1;
  if ((cp != NULL) && (llp != NULL) && sscanf(cp, "%Ld", llp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfTimeTicksConvert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT32 *lp = (SNMP_UINT32*) rval;

  // timeticks have the value enclosed in '()' so we bump over it
  cp = strchr(sval, '(') + 1;

  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%ld", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfHexStrConvert(void *rval, char *sval)
{
  char *cp;
  char *lp = (char*) rval;

  cp = strchr(sval, ':') + 1;

  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%s", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfNetAddrConvert(void *rval, char *sval)
{
  char *cp;
  char *lp = (char*) rval;

  cp = strchr(sval, ':') + 1;

  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%s", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfIntConvert(void *rval, char *sval)
{
  char *cp;
  SNMP_UINT32 *lp = (SNMP_UINT32*) rval;

  cp = strchr(sval, ':') + 1;


  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%ld", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfStrConvert(void *rval, char *sval)
{
  char *cp;
  char *lp = (char*) rval;

  cp = strchr(sval, ':') + 1;

#if     0
  printf("StrConvert %s\n", cp,0,0,0,0,0);
#endif

  if ((cp != NULL) && (lp != NULL)) {
    while (*cp)
      *lp++ = *cp++;
    return(epicsOk);
  } else
    return(epicsError);
}
//--------------------------------------------------------------------
static int snmpWfIpAddrConvert(void *rval, char *sval)
{
  char *cp;
  char *lp = (char*) rval;

  cp = strchr(sval, ':') + 1;

  if ((cp != NULL) && (lp != NULL) && sscanf(cp, "%s", lp))
    return(epicsOk);
  else
    return(epicsError);
}
//--------------------------------------------------------------------
int list_context_compare(void *p1, void *p2)
{
  pointerList *pL1 = (pointerList *) p1;
  pointerList *pL2 = (pointerList *) p2;

  unsigned long u1 = (unsigned long) pL1->context();
  unsigned long u2 = (unsigned long) pL2->context();

  if (u1 > u2)
    return(-1);
  else if (u2 > u1)
    return(1);
  else
    return(0);
}
//--------------------------------------------------------------------
int devSnmp_host_compare(void *p1, void *p2)
{
  devSnmp_host *pG1 = (devSnmp_host *) p1;
  devSnmp_host *pG2 = (devSnmp_host *) p2;

  return( strcmp(pG1->hostName(),pG2->hostName()) );
}
//--------------------------------------------------------------------
int devSnmp_group_compare(void *p1, void *p2)
{
  devSnmp_group *pG1 = (devSnmp_group *) p1;
  devSnmp_group *pG2 = (devSnmp_group *) p2;

  int ret = strcmp(pG1->hostName(),pG2->hostName());
  if (ret != 0) return(ret);
  return( strcmp(pG1->communityName(),pG2->communityName()) );
}
//--------------------------------------------------------------------
int devSnmp_device_compare(void *p1, void *p2)
{
  devSnmp_device *pD1 = (devSnmp_device *) p1;
  devSnmp_device *pD2 = (devSnmp_device *) p2;

  return( strcmp(pD1->recordName(),pD2->recordName()) );
}
//--------------------------------------------------------------------
int devSnmp_poll_weight_compare(void *p1, void *p2)
{
  devSnmp_device *pD1 = (devSnmp_device *) p1;
  devSnmp_device *pD2 = (devSnmp_device *) p2;

  int w1 = pD1->getPollWeight();
  int w2 = pD2->getPollWeight();

  if (w1 > w2)
    return(1);
  else if (w2 > w1)
    return(-1);
  else
    return(0);
}
//--------------------------------------------------------------------
// routines accessible from ioc shell
//--------------------------------------------------------------------
int epicsSnmpInit(int param)
{
  checkInit();
  if (! pManager) {
    printf("epicsSnmpInit: init FAILED\n");
    return(epicsError);
  }
  return( pManager->start() );
}
//--------------------------------------------------------------------
int epicsSnmpSetSnmpVersion(char *hostName, char *versionStr)
{
  checkInit();
  pManager->setHostSnmpVersion(hostName,versionStr);
  return(epicsOk);
}
//--------------------------------------------------------------------
int epicsSnmpSetMaxOidsPerReq(char *hostName, int maxoids)
{
  checkInit();
  pManager->setMaxOidsPerReq(hostName,maxoids);
  return(epicsOk);
}
//--------------------------------------------------------------------
int devSnmpSetDebug(int level)
{
  checkInit();
  devSnmpDebug = (level <= 0) ? 0 : level;
  if (level == 0)
    printf("devSnmpDebug is now OFF\n");
  else
    printf("devSnmpDebug level is now %d\n",devSnmpDebug);
  return(epicsOk);
}
//--------------------------------------------------------------------
int snmpr(int level, char *match)
{
  checkInit();
  pManager->report(level,match);
  return(epicsOk);
}
//--------------------------------------------------------------------
int snmpz(void)
{
  checkInit();
  pManager->zeroCounters();
  return(epicsOk);
}
//--------------------------------------------------------------------
int snmpzr(int level, char *match)
{
  checkInit();
  snmpz();
  printf("waiting 10 seconds for counts to rack up...\n");
  epicsThreadSleep(10.0);
  return( snmpr(level,match) );
}
//--------------------------------------------------------------------
// class pointerList
//--------------------------------------------------------------------
pointerList::pointerList(int initialCount, int extendCount)
{
  mutex     = epicsMutexCreate();
  delta     = extendCount;
  currSize  = initialCount;
  currCount = 0;
  _context  = NULL;

  pPtrList = new void *[currSize];
  if (pPtrList) {
    for (int ii = 0; ii < currSize; ii++) pPtrList[ii] = NULL;
  }
}
//--------------------------------------------------------------------
pointerList::~pointerList(void)
{
  // we don't know how to call destructors for pointers we're storing
  // so we hope our owner has taken care of clearing us out, we just
  // dispose of the list of pointers we allocated
  if (pPtrList) {
    delete [] pPtrList;
    pPtrList = NULL;
  }

  // delete our mutex
  epicsMutexDestroy(mutex);
}
//--------------------------------------------------------------------
void *pointerList::context(void)
{
  return(_context);
}
//--------------------------------------------------------------------
void pointerList::setContext(void *newcontext)
{
  _context = newcontext;
}
//--------------------------------------------------------------------
void pointerList::append(void *ptr)
{
  epicsMutexLock(mutex);

  if (currCount >= currSize) {
    // dynamic array is full, need to extend it
    int newSize = currSize + delta;
    void **newList = new void *[newSize];
    for (int ii = 0; ii < newSize; ii++) {
      newList[ii] = (ii < currCount) ? pPtrList[ii] : NULL;
    }
    currSize = newSize;
    delete [] pPtrList;
    pPtrList = newList;
  }

  // store pointer
  pPtrList[currCount] = ptr;
  currCount++;

  epicsMutexUnlock(mutex);
}
//--------------------------------------------------------------------
int pointerList::count(void)
{
  epicsMutexLock(mutex);
  int c = currCount;
  epicsMutexUnlock(mutex);
  return(c);
}
//--------------------------------------------------------------------
void *pointerList::itemAt(int idx)
{
  epicsMutexLock(mutex);
  void *retval = ((idx < 0) || (idx >= currCount)) ? NULL : pPtrList[idx];
  epicsMutexUnlock(mutex);
  return(retval);
}
//--------------------------------------------------------------------
void *pointerList::removeItemAt(int idx)
{
  epicsMutexLock(mutex);

  void *retval;
  if ((idx < 0) || (idx >= currCount))
    retval = NULL;
  else {
    retval = pPtrList[idx];
    for (int ii = idx; ii < currCount-1; ii++) {
      pPtrList[ii] = pPtrList[ii+1];
    }
    pPtrList[currCount-1] = NULL;
    currCount--;
  }

  epicsMutexUnlock(mutex);
  return(retval);
}
//--------------------------------------------------------------------
void pointerList::sort(POINTERLIST_COMPARE_PROC proc)
// performs a good old boring shell sort (fairly fast and is easy to code...)
{
  // no compare procedure, don't sort
  if (! proc) return;

  epicsMutexLock(mutex);

  int gap = currCount / 2;
  do {
    int swaps;
    do {
      swaps = 0;
      for (int ii = 0; ii < (currCount - gap); ii++) {
        if (proc(pPtrList[ii],pPtrList[ii+gap]) > 0) {
          void *tmp = pPtrList[ii];
          pPtrList[ii] = pPtrList[ii+gap];
          pPtrList[ii+gap] = tmp;
          swaps++;
        }
      }
    } while (swaps != 0);
    gap = gap / 2;
  } while (gap != 0);

  epicsMutexUnlock(mutex);
}
//--------------------------------------------------------------------
// class timeObject
//--------------------------------------------------------------------
timeObject::timeObject(void)
{
  clear();  // start out zero-ed
}
//--------------------------------------------------------------------
timeObject::~timeObject(void)
{
  // nothing to do here
}
//--------------------------------------------------------------------
void timeObject::start(void)
{
  epicsTimeGetCurrent(&lastStarted);
}
//--------------------------------------------------------------------
bool timeObject::started(void)
{
  return( (lastStarted.secPastEpoch == 0) ? FALSE : TRUE );
}
//--------------------------------------------------------------------
void timeObject::clear(void)
{
  lastStarted.secPastEpoch = 0;
  lastStarted.nsec = 0;
}
//--------------------------------------------------------------------
void timeObject::toDateTimeString(char *str)
{
  struct tm tb;
  unsigned long nsec;

  if (! started()) {
    strcpy(str,"n/a");
  } else {
    epicsTimeToTM(&tb,&nsec,&lastStarted);
    tb.tm_mon++;
    tb.tm_year += 1900;
    sprintf(str,"%04d/%02d/%02d %02d:%02d:%02d",
            tb.tm_year,tb.tm_mon,tb.tm_mday,
            tb.tm_hour,tb.tm_min,tb.tm_sec);
  }
}
//--------------------------------------------------------------------
double timeObject::elapsedSeconds(void)
{
  // we do the math here directly, as calling epicsTimeDiffInSeconds
  // has extra overhead and we aren't worried about roll-over
  epicsTimeStamp now;
  epicsTimeGetCurrent(&now);
  double NowS  = now.secPastEpoch;
  double NowN  = now.nsec;
  double LastS = lastStarted.secPastEpoch;
  double LastN = lastStarted.nsec;
  return((NowS - LastS) + (NowN - LastN)/1.0e+9);
}
//--------------------------------------------------------------------
double timeObject::elapsedMilliseconds(void)
{
  return( elapsedSeconds() * 1000.0 );
}
//--------------------------------------------------------------------
// class devSnmp_transaction
//--------------------------------------------------------------------
devSnmp_transaction::devSnmp_transaction(bool isSet)
{
  createTime.start();
  is_set = isSet;
}
//--------------------------------------------------------------------
devSnmp_transaction:: ~devSnmp_transaction(void)
{
  // nothing to do here
}
//--------------------------------------------------------------------
bool devSnmp_transaction::isSetting(void)
{
  return(is_set);
}
//--------------------------------------------------------------------
double devSnmp_transaction::millisecondsSinceCreated(void)
{
  return( createTime.elapsedMilliseconds() );
}
//--------------------------------------------------------------------
// class devSnmp_getTransaction
//--------------------------------------------------------------------
devSnmp_getTransaction::devSnmp_getTransaction(int MaxItems)
   : devSnmp_transaction(FALSE)
{
  deviceList = new pointerList();
  ourMaxItems = (MaxItems > 0) ? MaxItems : 1;
}
//--------------------------------------------------------------------
devSnmp_getTransaction::~devSnmp_getTransaction(void)
{
  //
  // delete devicelist but not items in it, those belong to other objects
  // we also inform all devices in the list they're now unqueued
  //
  if (deviceList) {
    for (int ii = 0; ii < deviceList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
      if (! pDevice) continue;
      pDevice->setGetQueued(FALSE);
    }
    delete deviceList;
    deviceList = NULL;
  }
}
//--------------------------------------------------------------------
void devSnmp_getTransaction::addDevice(devSnmp_device *pDevice)
{
  if (deviceList) {
    deviceList->append(pDevice);
    pDevice->setGetQueued(TRUE);
  }
}
//--------------------------------------------------------------------
int devSnmp_getTransaction::count(void)
{
  if (! deviceList) return(0);
  return(deviceList->count());
}
//--------------------------------------------------------------------
bool devSnmp_getTransaction::isFull(void)
{
  if (! deviceList) return(TRUE);
  if (deviceList->count() >= ourMaxItems) return(TRUE);
  return(FALSE);
}
//--------------------------------------------------------------------
devSnmp_session *devSnmp_getTransaction::createSession(void)
{
  // no good if no devices
  if ((! deviceList) || (deviceList->count() == 0)) return(NULL);

  // create session
  devSnmp_device *pFirstDev = (devSnmp_device *) deviceList->itemAt(0);
  devSnmp_group *pGroup = pFirstDev->getGroup();
  devSnmp_session *pSession = new devSnmp_session(pFirstDev->getManager(),
                                                  pGroup,
                                                  FALSE);

  // open session
  if (! pSession->open(pGroup->getBaseSession())) {
    snmp_perror("devSnmp_setTransaction::createSession : GET session open failed");
    delete pSession;
    return(NULL);
  }

  // add devices to session
  for (int ii = 0; ii < deviceList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
    if (! pDevice) continue;
    pSession->addReading(pDevice,
                         pDevice->getOid()->Oid,
                         pDevice->getOid()->OidLen);
  }

  return(pSession);
}
//--------------------------------------------------------------------
// class devSnmp_setTransaction
//--------------------------------------------------------------------
devSnmp_setTransaction::devSnmp_setTransaction(devSnmp_device *pDevice)
   : devSnmp_transaction(TRUE)
{
  ourDevice = pDevice;

  // get next setting for device
  // (we're now responsible for the memory for this, device is handing it to us)
  ourDeviceSetting = pDevice->getNextSetting();
}
//--------------------------------------------------------------------
devSnmp_setTransaction::~devSnmp_setTransaction(void)
{
  if (ourDeviceSetting) {
    delete [] ourDeviceSetting;
    ourDeviceSetting = NULL;
  }
}
//--------------------------------------------------------------------
devSnmp_session *devSnmp_setTransaction::createSession(void)
{
  // create session
  devSnmp_group *pGroup = ourDevice->getGroup();
  devSnmp_session *pSession = new devSnmp_session(ourDevice->getManager(),
                                                  pGroup,
                                                  TRUE);

  // open session
  if (! pSession->open(pGroup->getBaseSession())) {
    snmp_perror("devSnmp_setTransaction::createSession : SET session open failed");
    delete pSession;
    return(NULL);
  }

  // add setting to session
  pSession->addSetting(ourDevice,
                       ourDevice->getOid()->Oid,
                       ourDevice->getOid()->OidLen,
                       ourDevice->getSetType(),
                       ourDeviceSetting);

  return(pSession);
}
//--------------------------------------------------------------------
// class devSnmp_host
//--------------------------------------------------------------------
devSnmp_host::devSnmp_host(devSnmp_manager *pMgr, char *host, bool *okay)
{
  (*okay) = FALSE;  // for now...

  pOurMgr = pMgr;
  hostname = dup_string(host);

  snmpGroupList = new pointerList();
  getQueue = new pointerList();
  setQueue = new pointerList();
  activeSessionList = new pointerList();
  maxOidsPerReq = pOurMgr->getHostMaxOidsPerReq(hostname);

  (*okay) = TRUE;
}
//--------------------------------------------------------------------
devSnmp_host::~devSnmp_host(void)
{
  // delete any remaining active sessions
  if (activeSessionList) {
    for (int ii = activeSessionList->count()-1; ii >= 0; ii--) {
      devSnmp_session *pTmp = (devSnmp_session *) activeSessionList->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete activeSessionList;
    activeSessionList = NULL;
  }

  // delete set queue
  if (setQueue) {
    for (int ii = setQueue->count()-1; ii >= 0; ii--) {
      devSnmp_setTransaction *pTmp = (devSnmp_setTransaction *) setQueue->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete setQueue;
    setQueue = NULL;
  }

  // delete get queue
  if (getQueue) {
    for (int ii = getQueue->count()-1; ii >= 0; ii--) {
      devSnmp_getTransaction *pTmp = (devSnmp_getTransaction *) getQueue->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete getQueue;
    getQueue = NULL;
  }

  // delete group list
  if (snmpGroupList) {
    for (int ii = snmpGroupList->count()-1; ii >= 0; ii--) {
      devSnmp_group *pTmp = (devSnmp_group *) snmpGroupList->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete snmpGroupList;
    snmpGroupList = NULL;
  }

  // delete host name
  if (hostname) {
    delete hostname;
    hostname = NULL;
  }
}
//--------------------------------------------------------------------
char *devSnmp_host::hostName(void)
{
  return(hostname);
}
//--------------------------------------------------------------------
devSnmp_group *devSnmp_host::createGroup(char *community)
{
  // create group structure
  bool okay;
  devSnmp_group *pGroup = new devSnmp_group(pOurMgr,this,community,&okay);
  if (! pGroup) return(NULL);
  if (! okay) {
    delete pGroup;
    return(NULL);
  }

  // add created group to group list
  snmpGroupList->append(pGroup);

  // keep group list sorted
  // (no real reason, just makes reports nicer)
  snmpGroupList->sort(devSnmp_group_compare);

  // done
  return(pGroup);
}
//--------------------------------------------------------------------
void devSnmp_host::addDevice(devSnmp_device *pDevice)
{
  // try to locate existing host/community group device belongs with
  bool found = FALSE;
  devSnmp_group *pGroup = NULL;
  for (int ii = 0; ii < snmpGroupList->count(); ii++) {
    pGroup = (devSnmp_group *) snmpGroupList->itemAt(ii);
    if (! pGroup) continue;
    if (strcmp((char *)pDevice->communityName(), (char *)pGroup->communityName()) == 0) {
      found = TRUE;
      break;
    }
  }

  if (! found) {
    // existing group NOT found, need to create it
    pGroup = createGroup((char *)pDevice->communityName());
    if (! pGroup) {
      printf("FAILED to created group for host %s community %s\n",
              hostName(),pDevice->communityName());
      return;
    }
  }

  // group found (or created), append device to group's deviceList
  pGroup->addDevice(pDevice);

  // fill in device's group pointer
  pDevice->setGroup(pGroup);
}
//--------------------------------------------------------------------
void devSnmp_host::queueGetTransaction(devSnmp_getTransaction *pTrans)
{
  if (getQueue) getQueue->append(pTrans);
}
//--------------------------------------------------------------------
void devSnmp_host::queueSetTransaction(devSnmp_setTransaction *pTrans)
{
  if (setQueue) setQueue->append(pTrans);
}
//--------------------------------------------------------------------
void devSnmp_host::setMaxOidsPerReq(int maxoids)
{
  // set our local variable
  maxOidsPerReq = (maxoids > 0) ? maxoids : 1;

  // roll through each host/community group, setting max oids for it
  for (int group_idx = 0; group_idx < snmpGroupList->count(); group_idx++) {
    devSnmp_group *pGroup = (devSnmp_group *) snmpGroupList->itemAt(group_idx);
    if (! pGroup) continue;
    pGroup->setMaxOidsPerReq(maxOidsPerReq);
  }
}
//--------------------------------------------------------------------
int devSnmp_host::getMaxOidsPerReq(void)
{
  return(maxOidsPerReq);
}
//--------------------------------------------------------------------
void devSnmp_host::processing(void)
{
  // roll through each host/community group, having it process
  for (int group_idx = 0; group_idx < snmpGroupList->count(); group_idx++) {
    devSnmp_group *pGroup = (devSnmp_group *) snmpGroupList->itemAt(group_idx);
    if (! pGroup) continue;
    pGroup->processing();
  }

  //
  // dispose of any completed GET/SET sessions
  //
  // we also dispose of sessions that are older than 60 seconds,
  // which shouldn't happen but we want to make sure they're deleted
  //
  // if we find any sessions that were sent but aren't complete, we're 'busy'
  // (some hosts like Wiener/ISEG crates can't handle simultaneous requests,
  //  so we do all transactions one at a time)
  //
  // we go backwards in list so removing items doesn't mess up our indexing
  //
  bool busy = FALSE;
  for (int ii = activeSessionList->count()-1; ii >= 0; ii--) {
    devSnmp_session *pSession = (devSnmp_session *) activeSessionList->itemAt(ii);
    if (! pSession) continue;
    if ((pSession->isCompleted()) || (pSession->secondsSinceCreated() > 60)) {
      if ((devSnmpDebug) && (pSession->secondsSinceCreated() > 60))
        printf("*** devSnmp: %s deleted stale session\n",hostName());
      activeSessionList->removeItemAt(ii);
      delete pSession;
    } else if (pSession->wasSent()) {
      busy = TRUE;
    }
  }

  // if we're busy, that's all for now
  if (busy) return;

  //
  // not busy if we got this far, pull a transaction out of our queues to send
  //
  devSnmp_transaction *pTrans = NULL;
  if (setQueue->count() > 0) {
    // there are set transactions waiting to go out
    if (getQueue->count() <= 0) {
      // no 'get' transactions waiting, set transaction can go out
      pTrans = (devSnmp_transaction *) setQueue->removeItemAt(0);
    } else {
      // 'get' transactions are waiting, select 'set' transaction only
      // if oldest 'get' transaction hasn't been waiting too long
      devSnmp_transaction *pTopGet = (devSnmp_transaction *) getQueue->itemAt(0);
      if (pTopGet->millisecondsSinceCreated() < GetWaitedTooLongMSec)
        pTrans = (devSnmp_transaction *) setQueue->removeItemAt(0);
      else
        pTrans = (devSnmp_transaction *) getQueue->removeItemAt(0);
    }
  } else if (getQueue->count() > 0) {
    // no 'set' transactions waiting, select next 'get' transaction in queue
    pTrans = (devSnmp_transaction *) getQueue->removeItemAt(0);
  }

  if (pTrans) {
    // create session from transaction object
    devSnmp_session *pSession = pTrans->createSession();

    if (! pSession) {
      // could not create session, nothing else to do here
    } else if (! pSession->send()) {
      // send failed
      snmp_perror("devSnmp_host::processing : session send failed");
      delete pSession;
    } else {
      // send succeeded, add it to active session list
      activeSessionList->append(pSession);
    }

    // delete transaction object, we're done with it now
    delete pTrans;
  }
}
//--------------------------------------------------------------------
void devSnmp_host::zeroCounters(void)
{
  // roll through each host/community group, having it zero
  for (int group_idx = 0; group_idx < snmpGroupList->count(); group_idx++) {
    devSnmp_group *pGroup = (devSnmp_group *) snmpGroupList->itemAt(group_idx);
    if (! pGroup) continue;
    pGroup->zeroCounters();
  }
}
//--------------------------------------------------------------------
void devSnmp_host::report(int level, char *match)
{
  // roll through each host/community group, having it report
  for (int group_idx = 0; group_idx < snmpGroupList->count(); group_idx++) {
    devSnmp_group *pGroup = (devSnmp_group *) snmpGroupList->itemAt(group_idx);
    if (! pGroup) continue;
    pGroup->report(level,match);
  }
}
//--------------------------------------------------------------------
// class devSnmp_group
//--------------------------------------------------------------------
devSnmp_group::devSnmp_group(devSnmp_manager *pMgr, devSnmp_host *host, char *community, bool *okay)
{
  (*okay) = FALSE; // for now...

  pOurMgr        = pMgr;
  pOurHost       = host;
  deviceList     = new pointerList();
  weightList     = new pointerList();
  bestReplyMsec  = 0.0;
  worstReplyMsec = 0.0;
  avgReplyMsec   = 0.0;
  sendCount      = 0;
  replyCount     = 0;
  errorCount     = 0;
  maxOidsPerReq  = pOurHost->getMaxOidsPerReq();

  // create base session structure
  base_session = new SNMP_SESSION;
  if (! base_session) return;
  memset(base_session,0,sizeof(SNMP_SESSION));

  // initialize base session structure
  snmp_sess_init(base_session);
  base_session->retries = 5;
  base_session->timeout = 300000;  // units are uSec

  base_session->peername       = dup_string(pOurHost->hostName());
  base_session->community      = (unsigned char *) dup_string(community);
  base_session->community_len  = strlen(community);
  base_session->version        = pOurMgr->getHostSnmpVersion(base_session->peername);
  base_session->callback       = snmpSessionCallback;

  (*okay) = TRUE;
}
//--------------------------------------------------------------------
devSnmp_group::~devSnmp_group(void)
{
  // delete our devices
  if (deviceList) {
    for (int ii = deviceList->count()-1; ii >= 0; ii--) {
      devSnmp_device *pTmp = (devSnmp_device *) deviceList->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete deviceList;
    deviceList = NULL;
  }

  // delete weight list
  if (weightList) {
    delete weightList;
    weightList = NULL;
  }

  // delete base session
  if (base_session) {
    delete [] base_session->peername;
    delete [] base_session->community;
    delete base_session;
    base_session = NULL;
  }
}
//--------------------------------------------------------------------
void devSnmp_group::addDevice(devSnmp_device *pDevice)
{
  // add item to our device list and weight list
  deviceList->append(pDevice);
  weightList->append(pDevice);

  // keep device list sorted
  // (no real reason, just makes reports nicer)
  deviceList->sort(devSnmp_device_compare);
}
//--------------------------------------------------------------------
const char *devSnmp_group::hostName(void)
{
  return(base_session->peername);
}
//--------------------------------------------------------------------
const char *devSnmp_group::communityName(void)
{
  return((const char *)base_session->community);
}
//--------------------------------------------------------------------
SNMP_SESSION *devSnmp_group::getBaseSession(void)
{
  return(base_session);
}
//--------------------------------------------------------------------
int devSnmp_group::snmpVersion(void)
{
  return(base_session->version);
}
//--------------------------------------------------------------------
void devSnmp_group::setMaxOidsPerReq(int maxoids)
{
  maxOidsPerReq = (maxoids > 0) ? maxoids : 1;
}
//--------------------------------------------------------------------
int devSnmp_group::getMaxOidsPerReq(void)
{
  return(maxOidsPerReq);
}
//--------------------------------------------------------------------
void devSnmp_group::processing(void)
{
  if ((! deviceList) || (deviceList->count() == 0)) return;  // if no devices, we're done

  //
  // allow devices to do any periodic processing they have
  //
  for (int ii = 0; ii < deviceList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
    if (! pDevice) continue;
    if (pDevice) pDevice->periodicProcessing();
  }

  //
  // if any devices have a setting they need to send out, build a transaction
  // for each and queue it with our host to be sent out
  //
  for (int ii = 0; ii < deviceList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
    if (! pDevice) continue;
    if (pDevice->needsSet()) {
      // create devSnmp_setTransaction for setting and queue it
      devSnmp_setTransaction *pSetTrans = new devSnmp_setTransaction(pDevice);
      pOurHost->queueSetTransaction(pSetTrans);
    }
  }

  //
  // do 'get' polling
  //
  // We sort weightList by devices most to least needing to be polled (lowest
  // to highest poll weight) and queue a get transaction for the most needy
  // batch of them *IF* the most needy one's poll weight is less than or
  // equal to MaxTopPollWeight (i.e. something actually needs to be polled
  // right now).  This will ensure devices that need to be polled are polled,
  // and we may as well fill up a request for other devices that need to be
  // polled (or soon will be) while we're at it.
  //
  // If we can't get all devices needing a poll into one request, we get
  // called many times a second and they'll be queued for polling next time.
  //

  // instruct all devices to recalculate their poll weight
  // (this needs to be done separately and not done in getPollWeight,
  //  as getPollWeight is called during sort and if the value is constantly
  //  changing the sort may never complete)
  for (int ii = 0; ii < weightList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) weightList->itemAt(ii);
    if (! pDevice) continue;
    pDevice->recalcPollWeight();
  }

  // sort list by poll weight
  weightList->sort(devSnmp_poll_weight_compare);

  // If most poll-needy device is at or below MaxTopPollWeight, add
  // devices to a get transaction object until it is full or we reach a
  // device that should not be polled (its poll weight is DoNotPollWeight
  // or higher)
  devSnmp_device *pTop = (devSnmp_device *) weightList->itemAt(0);
  if ((pTop) && (pTop->getPollWeight() <= devSnmp_device::MaxTopPollWeight)) {
    devSnmp_getTransaction *pGetTrans = new devSnmp_getTransaction(maxOidsPerReq);
    for (int ii = 0; ii < weightList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) weightList->itemAt(ii);
      if (! pDevice) continue;

      // stop filling request if we reach a do-not-poll weighted device
      if (pDevice->getPollWeight() >= devSnmp_device::DoNotPollWeight) break;

      // add this device to request, and stop if the request is then full
      pGetTrans->addDevice(pDevice);
      if (pGetTrans->isFull()) break;
    }
    // queue our constructed get transaction with our host object
    pOurHost->queueGetTransaction(pGetTrans);
  }

  // that's it for us until next time...
}
//--------------------------------------------------------------------
void devSnmp_group::sessionGotError(devSnmp_session *pSession)
{
  errorCount++;
}
//--------------------------------------------------------------------
void devSnmp_group::sessionSent(int state)
{
  if (state)
    sendCount++;
  else
    errorCount++;
}
//--------------------------------------------------------------------
void devSnmp_group::sessionGotReply(devSnmp_session *pSession)
{
  // update avgReplyMsec, etc

  double msec = 1000.0 * pSession->secondsSinceSent();

  if (replyCount) {
    if (msec < bestReplyMsec) bestReplyMsec = msec;
    if (msec > worstReplyMsec) worstReplyMsec = msec;
  } else {
    bestReplyMsec = msec;
    worstReplyMsec = msec;
  }

  double tot_msec = avgReplyMsec * replyCount;
  replyCount++;
  tot_msec += msec;
  avgReplyMsec = tot_msec / replyCount;
}
//--------------------------------------------------------------------
void devSnmp_group::zeroCounters(void)
{
  bestReplyMsec = 0.0;
  worstReplyMsec = 0.0;
  avgReplyMsec = 0.0;
  sendCount = 0;
  replyCount = 0;
  errorCount = 0;

  // roll through devices zero-ing their counters
  for (int ii = 0; ii < deviceList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
    if (! pDevice) continue;
    pDevice->zeroCounters();
  }
}
//--------------------------------------------------------------------
void devSnmp_group::report(int level, char *match)
{
  bool show_us;
  if (! match) {
    // no match string given, always display us
    show_us = TRUE;
  } else {
    // if a 'match' string is given, display us only if we have at
    // least one matching device
    show_us = FALSE;
    for (int ii = 0; ii < deviceList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
      if (! pDevice) continue;
      if (strstr(pDevice->recordName(),match)) {
        show_us = TRUE;
        break;
      }
    }
  }

  // nothing to do if we aren't to be displayed
  if (! show_us) return;

  printf("%s\n",GROUP_BAR);
  printf("Group\n");
  printf("  Host               : %s\n",hostName());
  printf("  Community          : %s\n",communityName());
  printf("  Max oids per get   : %d\n",maxOidsPerReq);
  printf("  Send count         : %ld\n",sendCount);
  printf("  Reply count        : %ld\n",replyCount);
  printf("  Error count        : %ld\n",errorCount);
  printf("  Avg reply time     : %.1lf msec\n",avgReplyMsec);
  printf("  Best reply time    : %.1lf msec\n",bestReplyMsec);
  printf("  Worst reply time   : %.1lf msec\n",worstReplyMsec);
  printf("\n");

  if (level <= 0) {
    // we don't display individual devices, we just aggregate average poll
    // times for sets of devices with the same pollGapMSec
    pollAggregate aggregate;
    for (int ii = 0; ii < deviceList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
      if (! pDevice) continue;
      // if 'match' is defined, skip records that don't match it
      if ((match) && (! strstr(pDevice->recordName(),match))) continue;
      aggregate.addDevice(pDevice);
    }
    aggregate.report();
  } else {
    printf("  Group channels:\n");
    printf("\n");
    if (level == 1) {
      printf("                           desired    actual\n");
      printf("  polls sent  poll repls  polls/sec  polls/sec  age (msec)      errors  name\n");
      printf("  ----------  ----------  ---------  ---------  ----------  ----------  ----------\n");
    }
    // roll through devices
    for (int ii = 0; ii < deviceList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
      if (! pDevice) continue;
      // if 'match' is defined, skip records that don't match it
      if ((match) && (! strstr(pDevice->recordName(),match))) continue;
      // have record report
      pDevice->report(level);
    }
  }
}
//--------------------------------------------------------------------
// class devSnmp_session
//--------------------------------------------------------------------
devSnmp_session::devSnmp_session(devSnmp_manager *pMgr, devSnmp_group *pGroup, bool is_set)
{
  pOurMgr     = pMgr;
  pOurGroup   = pGroup;
  session     = NULL;
  pdu         = NULL;
  is_setting  = is_set;
  completed   = FALSE;
  sent        = FALSE;
  tried_send  = FALSE;
  deviceList = new pointerList();
  timeStarted.start();
  timeSent.clear();
}
//--------------------------------------------------------------------
devSnmp_session::~devSnmp_session(void)
{
  close();
  if (deviceList) {
    // don't delete device objects in list
    // they belong to their devSnmp_group object, not us
    delete deviceList;
    deviceList = NULL;
  }
}
//--------------------------------------------------------------------
SNMP_SESSION *devSnmp_session::getSession(void)
{
  return(session);
}
//--------------------------------------------------------------------
int devSnmp_session::replyProcessing(int op, SNMP_SESSION *sp, int reqId, SNMP_PDU *pdu)
{
  // if we're already completed this is a duplicate, disregard it
  if (completed) {
    return(1);  // 1 = we're done with request, don't keep pending
  }

  // inform our group of reply arrival (for timing purposes)
  if (pOurGroup) pOurGroup->sessionGotReply(this);

  // untick manager's request count so its read task knows when to stop reading
  pOurMgr->decActiveRequests();

  if (op == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
    //
    // received a reply
    //
    if (pdu->errstat == SNMP_ERR_NOERROR) {
      //
      // no errors reported in request
      //

      //
      // sanity check: count variables returned, is an error if doesn't
      // match count we asked for
      //
      // this is needed for MOXA devices (possibly others) that return no
      // error if too many oids requested and just return the number it
      // could handle (very unfriendly!)
      //
      int sanityCount = 0;
      netsnmp_variable_list *vars = pdu->variables;
      while (vars) {
        vars = vars->next_variable;
        sanityCount++;
      }

      if (sanityCount != deviceList->count()) {
        // SANITY CHECK FAILED
        if (pOurGroup) pOurGroup->sessionGotError(this);
        if (devSnmpDebug > 2) {
          printf("*** %s transaction (GET) sanity check failed: asked for %d oids got %d\n"
                 "*** (try adding epicsSnmpSetMaxOidsPerReq call to ioc script)\n",
                 ((pOurGroup) ? pOurGroup->hostName() : ""),
                 deviceList->count(),
                 sanityCount);
        }
        // inform devices we had a problem but it isn't their fault
        // (send them SNMP_ERR_NOERROR but var is NULL)
        for (int ii = 0; ii < deviceList->count(); ii++) {
          devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
          if (! pDevice) continue;
          pDevice->replyProcessing(this,op,SNMP_ERR_NOERROR,NULL);
        }
      } else {
        // roll through reply variables in PDU, sending data along to the
        // devSnmp_device they belong to for processing
        netsnmp_variable_list *vars = pdu->variables;
        int idx = 0;
        while (vars) {
          devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(idx);
          if (pDevice) pDevice->replyProcessing(this,op,SNMP_ERR_NOERROR,vars);
          vars = vars->next_variable;
          idx++;
        }
      }
    } else {
      //
      // error in request
      //
      // send error status to device that caused it
      // send no error (but no var either) to other innocent devices
      //
      if (pOurGroup) pOurGroup->sessionGotError(this);
      int bad_index = pdu->errindex - 1;  // is 1..x, make 0..x-1
      if (devSnmpDebug > 2) {
        printf("*** %s transaction (GET) error = %ld (%s)\n",
               ((pOurGroup) ? pOurGroup->hostName() : ""),
               pdu->errstat, snmp_errstring(pdu->errstat) );
      }
      for (int ii = 0; ii < deviceList->count(); ii++) {
        devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
        if (! pDevice) continue;
        if (ii == bad_index)
          pDevice->replyProcessing(this,op,pdu->errstat,NULL);
        else
          pDevice->replyProcessing(this,op,SNMP_ERR_NOERROR,NULL);
      }
    }
  } else {
    //
    // operation FAILED (timed out, etc)
    //
    // send op to each device for processing (we have to pass an error code,
    // so we just send SNMP_ERR_NOERROR, devices will be processing by op)
    //
    if (pOurGroup) pOurGroup->sessionGotError(this);
    if (devSnmpDebug > 2) {
      printf("*** %s transaction (GET) op = %d\n",
              ((pOurGroup) ? pOurGroup->hostName() : ""),
              op);
    }
    for (int ii = 0; ii < deviceList->count(); ii++) {
      devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
      if (! pDevice) continue;
      if (pDevice) pDevice->replyProcessing(this,op,SNMP_ERR_NOERROR,NULL);
    }
  }

  // done with reply, flag ourselves as completed so we can be
  // deleted later by our owner
  completed = TRUE;

  return(1);  // 1 = we're done with request, don't keep it pending
}
//--------------------------------------------------------------------
bool devSnmp_session::open(SNMP_SESSION *psess)
{
  // error if already opened
  if (session) return(FALSE);

  // open session
  session = snmp_open(psess);
  if (! session) return(FALSE);
  session->callback = snmpSessionCallback;
  session->callback_magic = this;

  // create PDU
  pdu = snmp_pdu_create( (is_setting) ? SNMP_MSG_SET : SNMP_MSG_GET );
  if (! pdu) {
    snmp_close(session);
    session = NULL;
    return(FALSE);
  }

  return(TRUE);
}
//--------------------------------------------------------------------
void devSnmp_session::addReading(devSnmp_device *pDevice, const oid *Oid, size_t size)
{
  if ((pdu) && (! is_setting)) {
    snmp_add_null_var(pdu,Oid,size);
    deviceList->append(pDevice);
  }
}
//--------------------------------------------------------------------
void devSnmp_session::addSetting
     (devSnmp_device *pDevice,
      const oid      *Oid,
      size_t          size,
      char            type,
      const char     *value)
{
  if ((pdu) && (is_setting)) {
    snmp_add_var(pdu,Oid,size,type,value);
    deviceList->append(pDevice);
  }
}
//--------------------------------------------------------------------
bool devSnmp_session::send(void)
{
  tried_send = TRUE;

  bool state;
  if (snmp_send(session,pdu)) {
    timeSent.start();
    sent = TRUE;
    // inc manager's request counter so its read task knows to be reading
    pOurMgr->incActiveRequests();
    state = TRUE;
  } else {
    state = FALSE;
  }

  // inform group of send success (for counters purposes)
  if (pOurGroup) pOurGroup->sessionSent(state);

  // inform devices of send status
  for (int ii = 0; ii < deviceList->count(); ii++) {
    devSnmp_device *pDevice = (devSnmp_device *) deviceList->itemAt(ii);
    if (! pDevice) continue;
    if (is_setting)
      pDevice->settingSendStatus(state);
    else
      pDevice->readingSendStatus(state);
  }

  return(sent);
}
//--------------------------------------------------------------------
void devSnmp_session::close(void)
{
  pOurMgr->sessionMutexLock();

  try {
    // if tried to send then session owns pdu and we shouldn't free it
    // otherwise we need to free it
    if ((pdu) && (! tried_send)) snmp_free_pdu(pdu);
    if (session) snmp_close(session);
  } catch( ... ) {
    printf("devSnmp : EXCEPTION in devSnmp_session::close\n");
  }

  pdu = NULL;
  session = NULL;

  pOurMgr->sessionMutexUnlock();
}
//--------------------------------------------------------------------
int devSnmp_session::itemCount(void)
{
  return (deviceList) ? deviceList->count() : 0;
}
//--------------------------------------------------------------------
bool devSnmp_session::wasSent(void)
{
  return(sent);
}
//--------------------------------------------------------------------
bool devSnmp_session::isCompleted(void)
{
  return(completed);
}
//--------------------------------------------------------------------
bool devSnmp_session::isSetting(void)
{
  return(is_setting);
}
//--------------------------------------------------------------------
double devSnmp_session::secondsSinceCreated(void)
{
  return( timeStarted.elapsedSeconds() );
}
//--------------------------------------------------------------------
double devSnmp_session::secondsSinceSent(void)
{
  return( timeSent.elapsedSeconds() );
}
//--------------------------------------------------------------------
// class devSnmp_device
//--------------------------------------------------------------------
devSnmp_device::devSnmp_device
     (devSnmp_manager *pMgr,
      devSnmp_group   *pGroup,
      struct dbCommon *pRec,
      struct link     *pLink,
      char            *type,
      bool            *okay)
{
  (*okay) = FALSE;  // for now...

  //printf("devSnmp_device::devSnmp_device\n")
  //ddd
  //fff
  //ggg

  // store parameters
  pOurMgr = pMgr;
  pOurGroup = pGroup;
  pRecord = pRec;
  our_type = dup_string(type);

  // initialize misc variables
  host              = NULL;
  community         = NULL;
  valMutex          = epicsMutexCreate();
  mask              = NULL;
  data_len          = 0;
  read_data         = NULL;
  pPeriodicFunction = NULL;
  periodicSeconds   = 0.0;
#ifdef KEEP_MULTIPLE_SETTINGS
  settingsQueue     = new pointerList();
#else
  setMutex          = epicsMutexCreate();
  setting_to_send   = NULL;
#endif
  flagged_read_bad  = SNMP_ERR_NOERROR;
  set_type          = 0;
  in_rec_process    = FALSE;
  pollSendCount     = 0;
  pollReplyCount    = 0;
  errorCount        = 0;
  has_reading       = FALSE;
  queued_for_get    = FALSE;
  pollWeight        = DoNotPollWeight;

  lastValidFlagSet.start();
  pollStart.start();
  lastSetSent.clear();
  lastPollSent.clear();
  lastPollReply.clear();
  lastPeriodicFuncCall.clear();

  setDataLength(40);

  // set polling gap msec by record scan rate
  switch (pRec->scan) {
    case menuScanPassive:   pollGapMSec = PassivePollGapMSec; break;
    case menuScan10_second: pollGapMSec = 10000;              break;
    case menuScan5_second:  pollGapMSec = 5000;               break;
    case menuScan2_second:  pollGapMSec = 2000;               break;
    case menuScan1_second:  pollGapMSec = 1000;               break;
    case menuScan_5_second: pollGapMSec = 500;                break;
    case menuScan_2_second: pollGapMSec = 200;                break;
    case menuScan_1_second: pollGapMSec = 100;                break;
    default:                pollGapMSec = 200;                break;
  }
  desiredPollsPerSec = (1000.0 / pollGapMSec);

  // parse record's INP or OUT field
  char *instioStr = pLink->value.instio.string;
  char in_host[128];
  char in_community[128];
  char in_oidStr[128];
  char in_mask[128];
  char set_format[128];
  int in_data_len = 0;
  int n = sscanf(instioStr, "%s %s %s %s %d %c",
                            in_host,
                            in_community,
                            in_oidStr,
                            in_mask,
                            &in_data_len,
                            set_format);
  if (n < 5) {
    printf("devSnmp: error parsing %s '%s'\n",pRecord->name,instioStr);
    printf("         format is 'host community oidname mask maxdatalength [settype]'\n");
    return;
  }
  host       = dup_string(in_host);
  community  = dup_string(in_community);
  oid.Name   = dup_string(in_oidStr);
  oid.OidLen = MAX_OID_LEN;
  mask       = dup_string(in_mask);

  // allow data_len to be over-ridden only if is larger than current default
  if (in_data_len > data_len) setDataLength(in_data_len);

  // parse optional set format type
  set_type = (n >= 6) ? set_format[0] : 's';

  // validate oid name
  if (! get_node(oid.Name, oid.Oid, (size_t *)&oid.OidLen)) {
    if (! read_objid(oid.Name,oid.Oid,(size_t *)&oid.OidLen)) {
      snmp_perror("devSnmp_device::devSnmp_device");
      return;
    }
  }

  (*okay) = TRUE;
}
//--------------------------------------------------------------------
devSnmp_device::~devSnmp_device(void)
{
  if (host)      delete [] host;
  if (community) delete [] community;
  if (oid.Name)  delete [] oid.Name;
  if (mask)      delete [] mask;
  if (read_data) delete [] read_data;
  if (our_type)  delete [] our_type;

#ifdef KEEP_MULTIPLE_SETTINGS
  if (settingsQueue) {
    for (int ii = settingsQueue->count()-1; ii >= 0; ii--) {
      char *pTmp = (char *) settingsQueue->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete settingsQueue;
    settingsQueue = NULL;
  }
#else
  if (setting_to_send) {
    delete [] setting_to_send;
    setting_to_send = NULL;
  }
  epicsMutexDestroy(setMutex);
#endif

  epicsMutexDestroy(valMutex);
}
//--------------------------------------------------------------------
bool devSnmp_device::getValueString(char *str, int maxsize)
{
  // if we have no valid reading, return NULL
  // (having a reading doesn't mean it's a reading value the record will
  //  like when it tries to parse it, or that the specified mask matches,
  //  just that we got a non-timeout response from the remote host with
  //  a string value)
  if (! has_reading) return(FALSE);

  epicsMutexLock(valMutex);

  char *pc = snmpStrStr(read_data, mask);
  if (! pc) {
    epicsMutexUnlock(valMutex);
    return(FALSE);
  }

  // skip mask
  pc += strlen(mask);

  // skip whitespace
  while (isspace(*pc) && ((*pc) != 0)) pc++;

  // copy string to caller's buffer
  int len = strlen(pc);
  if (len < maxsize)
    strcpy(str,pc);
  else {
    strncpy(str,pc,maxsize-1);
    str[maxsize-1] = 0;
  }

  epicsMutexUnlock(valMutex);
  return(TRUE);
}
//--------------------------------------------------------------------
bool devSnmp_device::getRawValueString(char *str, int maxsize)
{
  if (! has_reading) return(FALSE);
  epicsMutexLock(valMutex);
  int len = strlen(read_data);
  if (len < maxsize)
    strcpy(str,read_data);
  else {
    strncpy(str,read_data,maxsize-1);
    str[maxsize-1] = 0;
  }
  epicsMutexUnlock(valMutex);
  return(TRUE);
}
//--------------------------------------------------------------------
void devSnmp_device::periodicProcessing(void)
{
  // call periodic callback if is time to
  if (pPeriodicFunction) {
    if (lastPeriodicFuncCall.elapsedSeconds() >= periodicSeconds) {
      lastPeriodicFuncCall.start();
      (*pPeriodicFunction)(this);
    }
  }
}
//--------------------------------------------------------------------
void devSnmp_device::setReplyProcessing
     (devSnmp_session       *pSession,
      int                    op,
      long                   errCode,
      netsnmp_variable_list *var)
{
  if (op == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
    // a session for us received a message
    if (errCode == SNMP_ERR_NOERROR) {
      // no error code for us
      if (var) {
        // we got a variable passed to us, so our set succeeded
        // (nothing to do)
      }
    } else {
      // our set failed
      errorCount++;
      if (devSnmpDebug) printf("----- SET %s failed : %ld (%s)\n",
                                recordName(), errCode, snmp_errstring(errCode));
    }
  } else {
    // no message from session (timed out, got disconnected, etc)
    errorCount++;
    if (devSnmpDebug) {
      printf("----- SET %s failed : op=%d\n",recordName(), op);
    }

    // we DON'T try to re-send setting that failed, it is lost
    // (don't want to be sending old stale settings if remote host
    // is down for a couple days...)
  }
}
//--------------------------------------------------------------------
void devSnmp_device::readReplyProcessing
     (devSnmp_session       *pSession,
      int                    op,
      long                   errCode,
      netsnmp_variable_list *var)
{
  // clear have-reading flag, it will be set below if a reading came in
  has_reading = FALSE;

  if (op == NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE) {
    // a session for us received a message
    if (errCode == SNMP_ERR_NOERROR) {
      // no error code for us
      if (var) {
        // our reading succeeded, pull our data out of the variable given
        char buffer[1024];
        snprint_value(buffer, sizeof(buffer), var->name, var->name_length, var);
        epicsMutexLock(valMutex);
        sprintf(read_data, "%.*s", data_len, buffer);
        epicsMutexUnlock(valMutex);
        has_reading = TRUE;
      } else {
        // no error but DIDN'T get a variable, so some other device in our
        // session caused an error and as a result WE don't get our data!
        //
        // clear our lastPollSent time so we're polled again at the
        // next available opportunity
        lastPollSent.clear();
      }
      // this counts as a poll reply, even if we got no data because
      // some other device in our session had an error
      pollReplyCount++;
      lastPollReply.start();
    } else {
      // WE were the device that caused our session to fail
      errorCount++;
      if (devSnmpDebug > 2) {
        printf("*** %s (GET) error = %ld (%s)\n",
                recordName(), errCode, snmp_errstring(errCode));
      }
      // if was because no such oid, set flag so we don't get polled again
      if (errCode == SNMP_ERR_NOSUCHNAME) {
        flagged_read_bad = errCode;
        printf("*** %s (GET) flagged bad, error = %ld (%s)\n",
                recordName(), errCode, snmp_errstring(errCode));
      }
    }
  } else {
    // no message from session (timed out, is disconnected, etc)
    errorCount++;
  }
}
//--------------------------------------------------------------------
void devSnmp_device::replyProcessing
     (devSnmp_session       *pSession,
      int                    op,
      long                   errCode,
      netsnmp_variable_list *var)
{
  if (pSession->isSetting())
    setReplyProcessing(pSession,op,errCode,var);
  else
    readReplyProcessing(pSession,op,errCode,var);
}
//--------------------------------------------------------------------
void devSnmp_device::settingSendStatus(bool state)
{
  if (state) {
    lastSetSent.start();
  } else {
    errorCount++;
  }
}
//--------------------------------------------------------------------
void devSnmp_device::readingSendStatus(bool state)
{
  if (state) {
    pollSendCount++;
    lastPollSent.start();
  } else {
    errorCount++;
  }
}
//--------------------------------------------------------------------
void devSnmp_device::set(char *str)
{
  // add setting to our queue of settings
  char *copy = dup_string(str);

#ifdef KEEP_MULTIPLE_SETTINGS
  settingsQueue->append(copy);
#else
  epicsMutexLock(setMutex);
  if (setting_to_send) {
    delete [] setting_to_send;
    setting_to_send = NULL;
  }
  setting_to_send = copy;
  epicsMutexUnlock(setMutex);
#endif

  if (devSnmpDebug > 1) {
    printf("%s setting type:'%c' value:'%s'\n",pRecord->name, set_type, copy);
  }
}
//--------------------------------------------------------------------
void devSnmp_device::setGetQueued(bool state)
{
  queued_for_get = state;
}
//--------------------------------------------------------------------
OID *devSnmp_device::getOid(void)
{
  return(&oid);
}
//--------------------------------------------------------------------
char devSnmp_device::getSetType(void)
{
  return(set_type);
}
//--------------------------------------------------------------------
char *devSnmp_device::getNextSetting(void)
{
  char *return_setting = NULL;

#ifdef KEEP_MULTIPLE_SETTINGS
  if ((! settingsQueue) || (settingsQueue->count() <= 0))
    return_setting = NULL;
  else
    return_setting = (char *) settingsQueue->removeItemAt(0);
#else
  epicsMutexLock(setMutex);
  return_setting = setting_to_send;
  if (setting_to_send) {
    // we don't delete setting_to_send here
    // that gets done elsewhere after set is completed
    setting_to_send = NULL;
  }
  epicsMutexUnlock(setMutex);
#endif

  return(return_setting);
}
//--------------------------------------------------------------------
void devSnmp_device::recalcPollWeight(void)
{
  // don't poll us if:
  //     we're already queued for a poll
  //     or we're still waiting for a reply from a previous poll
  //     or we've been flagged as a bad oid in some previous poll
  if ((queued_for_get) || (flagged_read_bad != SNMP_ERR_NOERROR)) {
    pollWeight = DoNotPollWeight;
    return;
  }

  //
  // calculate poll weight based on time until we need to be polled again
  //
  // a poll weight of:
  //   < 0 : device is past due for poll (smaller num = more overdue)
  //   = 0 : device is due for poll right now
  //   > 0 : device isn't due for a poll yet (larger num = more time until due)
  //
  double raw_weight = pollGapMSec - lastPollSent.elapsedMilliseconds();
  // prevent weight from overflowing an int
  if (raw_weight < -32000) raw_weight = -32000;
  if (raw_weight >  32000) raw_weight =  32000;
  pollWeight = (int) (raw_weight);
}
//--------------------------------------------------------------------
int devSnmp_device::getPollWeight(void)
{
  return(pollWeight);
}
//--------------------------------------------------------------------
bool devSnmp_device::needsSet(void)
{
  // return whether there's a setting waiting to go out
#ifdef KEEP_MULTIPLE_SETTINGS
  return( (settingsQueue) && (settingsQueue->count() > 0) );
#else
  epicsMutexLock(setMutex);
  bool need = (setting_to_send != NULL);
  epicsMutexUnlock(setMutex);
  return(need);
#endif
}
//--------------------------------------------------------------------
int devSnmp_device::getDataLength(void)
{
  return(data_len);
}
//--------------------------------------------------------------------
void devSnmp_device::setDataLength(int length)
{
  data_len = length;

  // set length of val field
  if (! read_data) {
    read_data = new char[data_len];
    read_data[0] = 0;
  } else {
    char *newval = new char[data_len];
    int len = strlen(read_data);
    if (len < data_len)
      strcpy(newval,read_data);
    else {
      strncpy(newval,read_data,data_len-1);
      newval[data_len-1] = 0;
    }
    delete [] read_data;
    read_data = newval;
  }
}
//--------------------------------------------------------------------
void devSnmp_device::declareValueValid(void)
{
  lastValidFlagSet.start();
}
//--------------------------------------------------------------------
bool devSnmp_device::validTimeout(void)
{
  return( (lastValidFlagSet.elapsedMilliseconds() > ValidTimeoutMSec) ? TRUE : FALSE );
}
//--------------------------------------------------------------------
bool devSnmp_device::wasSetRecently(void)
{
  return( (lastSetSent.elapsedMilliseconds() < SetSkipReadbackMSec) ? TRUE : FALSE );
}
//--------------------------------------------------------------------
int devSnmp_device::getPollGapMSec(void)
{
  return(pollGapMSec);
}
//--------------------------------------------------------------------
void devSnmp_device::setPollGapMSec(int msec)
{
  // prevent gap from being less than 100 msec (10 poll/sec)
  pollGapMSec = (msec < 100) ? 100 : msec;
  desiredPollsPerSec = (1000.0 / pollGapMSec);
}
//--------------------------------------------------------------------
bool devSnmp_device::doingProcess(void)
{
  return(in_rec_process);
}
//--------------------------------------------------------------------
void devSnmp_device::processRecord(void)
{
  dbScanLock(pRecord);
  in_rec_process = TRUE;
  dbProcess(pRecord);
  in_rec_process = FALSE;
  dbScanUnlock(pRecord);
}
//--------------------------------------------------------------------
devSnmp_manager *devSnmp_device::getManager(void)
{
  return(pOurMgr);
}
//--------------------------------------------------------------------
devSnmp_group *devSnmp_device::getGroup(void)
{
  return(pOurGroup);
}
//--------------------------------------------------------------------
void devSnmp_device::setGroup(devSnmp_group *pGroup)
{
  pOurGroup = pGroup;
}
//--------------------------------------------------------------------
void devSnmp_device::setPeriodicCallback(DEVSNMP_DEVFUNC procFunc, double seconds)
{
  pPeriodicFunction = procFunc;
  periodicSeconds = seconds;
  lastPeriodicFuncCall.start();
}
//--------------------------------------------------------------------
const char *devSnmp_device::hostName(void)
{
  return(host);
}
//--------------------------------------------------------------------
const char *devSnmp_device::communityName(void)
{
  return(community);
}
//--------------------------------------------------------------------
const char *devSnmp_device::recordName(void)
{
  return(pRecord->name);
}
//--------------------------------------------------------------------
struct dbCommon *devSnmp_device::record(void)
{
  return(pRecord);
}
//--------------------------------------------------------------------
void devSnmp_device::zeroCounters(void)
{
  pollSendCount = 0;
  pollReplyCount = 0;
  errorCount = 0;
  pollStart.start();
}
//--------------------------------------------------------------------
double devSnmp_device::pollsPerSecond(void)
{
  if (pollReplyCount == 0) return(0.0);
  double diffsec = pollStart.elapsedSeconds();
  if (diffsec <= 0.0) return(0.0);
  return(pollReplyCount / diffsec);
}
//--------------------------------------------------------------------
void devSnmp_device::report(int level)
{
  if (level < 1) return;  // we don't report by-device data unless level is 1+

  double pps, diffsec;
  long age;
  char timestr[40];

  if (pollReplyCount == 0) {
    pps = 0.0;
    age = 0;
  } else {
    diffsec = pollStart.elapsedSeconds();
    pps = pollReplyCount / diffsec;
    diffsec = lastPollReply.elapsedSeconds();
    age = (long) (diffsec * 1000);
  }

  if (level == 1) {
    printf("  %10ld  %10ld  %9.2lf  %9.2lf  %10ld  %10ld  %s\n",
           pollSendCount,
           pollReplyCount,
           desiredPollsPerSec,
           pps,
           age,
           errorCount,
           pRecord->name);
  } else {
    lastPollReply.toDateTimeString(timestr);
    char vstr[128];
    if (! getValueString(vstr,sizeof(vstr))) strcpy(vstr,"n/a");
    printf("  %s\n",REPORT_BAR);
    printf("  Record name      : %s\n",    pRecord->name);
    printf("  Record type      : %s\n",    our_type);
    printf("  Remote host      : %s\n",    host);
    printf("  Community        : %s\n",    community);
    printf("  SNMP version     : %s\n",    snmpVersionString(pOurGroup->snmpVersion()));
    printf("  OID name         : %s\n",    oid.Name);
    printf("  Set type         : '%c'\n",  set_type);
    printf("  Poll weight      : %d\n",    pollWeight);
    printf("  Polls sent       : %ld\n",   pollSendCount);
    printf("  Poll replies     : %ld\n",   pollReplyCount);
    printf("  Errors           : %ld\n",   errorCount);
    printf("  Poll gap (msec)  : %d\n",    pollGapMSec);
    printf("  Poll age (msec)  : %ld\n",   age);
    printf("  Polls/sec        : %.2lf\n", pps);
    printf("  Last reply at    : %s\n",    timestr);
    printf("  Max data length  : %d\n",    data_len);
    printf("  Raw value string : '%s'\n",  read_data);
    printf("  Reply mask       : '%s'\n",  mask);
    printf("  Value string     : '%s'\n",  vstr);
    if (flagged_read_bad != SNMP_ERR_NOERROR)
      printf("*** Read flag bad  : %d (%s)\n",
             flagged_read_bad, snmp_errstring(flagged_read_bad));
  }
}
//--------------------------------------------------------------------
// class devSnmp_hostdata
//--------------------------------------------------------------------
devSnmp_hostdata::devSnmp_hostdata(char *host)
{
  ourHostName = dup_string(host);
  ourSnmpVersion = SNMP_VERSION_2c;
  ourMaxOidsPerReq = DEFAULT_MAX_OIDS_PER_REQ;
}
//--------------------------------------------------------------------
devSnmp_hostdata::~devSnmp_hostdata(void)
{
  if (ourHostName) {
    delete [] ourHostName;
    ourHostName = NULL;
  }
}
//--------------------------------------------------------------------
const char *devSnmp_hostdata::hostName(void)
{
  return(ourHostName);
}
//--------------------------------------------------------------------
void devSnmp_hostdata::setSnmpVersion(int version)
{
  ourSnmpVersion = version;
}
//--------------------------------------------------------------------
int devSnmp_hostdata::snmpVersion(void)
{
  return(ourSnmpVersion);
}
//--------------------------------------------------------------------
void devSnmp_hostdata::setMaxOidsPerReq(int maxoids)
{
  ourMaxOidsPerReq = (maxoids > 0) ? maxoids : 1;
}
//--------------------------------------------------------------------
int devSnmp_hostdata::maxOidsPerReq(void)
{
  return(ourMaxOidsPerReq);
}
//--------------------------------------------------------------------
// class devSnmp_manager
//--------------------------------------------------------------------
devSnmp_manager::devSnmp_manager(void)
{
  init_snmp("asynchapp");
  add_mibdir("/usr/share/snmp/mibs");
  init_mib();

  started           = FALSE;
  snmpSendTaskAbort = FALSE;
  snmpReadTaskAbort = FALSE;
  readTaskLoops     = 0;
  sendTaskLoops     = 0;
  readTaskId        = 0;
  sendTaskId        = 0;
  activeRequests    = 0;
  sessionMutex      = epicsMutexCreate();
  snmpHostList      = new pointerList();
  snmpHostDataList  = new pointerList();
}
//--------------------------------------------------------------------
devSnmp_manager::~devSnmp_manager(void)
{
  timeObject T;

  // stop send task
  T.start();
  snmpSendTaskAbort = TRUE;
  while ((snmpSendTaskAbort) && (T.elapsedMilliseconds() < 3000)) {
    epicsThreadSleep(ThreadSleepMsec / 1000.0);
  }

  // stop read task
  T.start();
  snmpReadTaskAbort = TRUE;
  while ((snmpReadTaskAbort) && (T.elapsedMilliseconds() < 3000)) {
    epicsThreadSleep(ThreadSleepMsec / 1000.0);
  }

  // delete host/version list items
  if (snmpHostDataList) {
    for (int ii = snmpHostDataList->count()-1; ii >= 0; ii--) {
      devSnmp_hostdata *pTmp = (devSnmp_hostdata *) snmpHostDataList->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete snmpHostDataList;
    snmpHostDataList = NULL;
  }

  // delete host list
  if (snmpHostList) {
    for (int ii = snmpHostList->count()-1; ii >= 0; ii--) {
      devSnmp_host *pTmp = (devSnmp_host *) snmpHostList->removeItemAt(ii);
      if (! pTmp) continue;
      delete pTmp;
    }
    delete snmpHostList;
    snmpHostList = NULL;
  }
}
//--------------------------------------------------------------------
int devSnmp_manager::start(void)
{
  if (started) {
    printf("devSnmp: error, already started\n");
    return(epicsError);
  }

  sendTaskId = epicsThreadCreate("snmpSendTask",
                                 76,
                                 epicsThreadGetStackSize(epicsThreadStackBig),
                                 (EPICSTHREADFUNC) snmpSendTask,
                                 this);

  readTaskId = epicsThreadCreate("snmpReadTask",
                                 77,
                                 epicsThreadGetStackSize(epicsThreadStackBig),
                                 (EPICSTHREADFUNC) snmpReadTask,
                                 this);

  started = TRUE;

  return(epicsOk);
}
//--------------------------------------------------------------------
int devSnmp_manager::readTask(void)
{
  readTaskStart.start();
  snmpReadTaskAbort = FALSE;
  readTaskLoops = 0;
  while (! snmpReadTaskAbort) {
    if (activeRequests) {

      int fds = 0, block = 1;
      fd_set fdset;
      struct timeval timeout;

      sessionMutexLock();

      FD_ZERO(&fdset);
      memset(&timeout,0,sizeof(struct timeval));
      snmp_select_info(&fds, &fdset, &timeout, &block);
      fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
      if (fds < 0) {
        perror("select failed");
        exit(1);
      }
      if (fds)
        snmp_read(&fdset);
      else
        snmp_timeout();

      sessionMutexUnlock();

    }
    epicsThreadSleep(ThreadSleepMsec / 1000.0);
    readTaskLoops++;
  }
  snmpReadTaskAbort = FALSE;  // clear flag to signal we've exited
  return(epicsError);
}
//--------------------------------------------------------------------
int devSnmp_manager::sendTask(void)
{
  sendTaskStart.start();
  snmpSendTaskAbort = FALSE;
  sendTaskLoops = 0;
  while (! snmpSendTaskAbort) {
    // do processing
    processing();

    // wait a bit, so as not to hog CPU
    epicsThreadSleep(ThreadSleepMsec / 1000.0);
    sendTaskLoops++;
  }
  snmpSendTaskAbort = FALSE;  // clear flag to signal we've exited
  return(epicsError);
}
//--------------------------------------------------------------------
void devSnmp_manager::sessionMutexLock(void)
{
  epicsMutexLock(sessionMutex);
}
//--------------------------------------------------------------------
void devSnmp_manager::sessionMutexUnlock(void)
{
  epicsMutexUnlock(sessionMutex);
}
//--------------------------------------------------------------------
void devSnmp_manager::incActiveRequests(void)
{
  activeRequests++;
}
//--------------------------------------------------------------------
void devSnmp_manager::decActiveRequests(void)
{
  if (activeRequests) activeRequests--;
}
//--------------------------------------------------------------------
void devSnmp_manager::setHostSnmpVersion(char *host, char *versionStr)
{
  // create and fill in devSnmp_hostdata structure
  int vers;
  if (strcmp(versionStr,"SNMP_VERSION_1") == 0)
    vers = SNMP_VERSION_1;
  else if (strcmp(versionStr,"SNMP_VERSION_2") == 0)
    vers = SNMP_VERSION_2c;
  else if (strcmp(versionStr,"SNMP_VERSION_2c") == 0)
    vers = SNMP_VERSION_2c;
  else if (strcmp(versionStr,"SNMP_VERSION_3") == 0)
    vers = SNMP_VERSION_3;
  else
    vers = SNMP_VERSION_2c;

  // see if we have a matching item in our snmpHostDataList
  devSnmp_hostdata *pFound = findHostData(host);
  if (! pFound) {
    // if no match found, create new item and append to list
    pFound = new devSnmp_hostdata(host);
    snmpHostDataList->append(pFound);
  }

  // set SNMP version for found (or created) list item
  pFound->setSnmpVersion(vers);
}
//--------------------------------------------------------------------
int devSnmp_manager::getHostSnmpVersion(char *host)
{
  // return version in snmpHostDataList if matching host found
  devSnmp_hostdata *pFound = findHostData(host);
  if (pFound)
    return(pFound->snmpVersion());
  else
    // if no item found in list, use default
    return(SNMP_VERSION_2c);
}
//--------------------------------------------------------------------
void devSnmp_manager::setMaxOidsPerReq(char *host, int maxoids)
{
  // minimum max oids is 1
  if (maxoids < 1) maxoids = 1;

  // see if we have a matching item in our snmpHostDataList
  devSnmp_hostdata *pFound = findHostData(host);
  if (! pFound) {
    // if no match found, create new item and append to list
    pFound = new devSnmp_hostdata(host);
    snmpHostDataList->append(pFound);
  }

  // set max oids for found (or created) list item
  pFound->setMaxOidsPerReq(maxoids);

  // if we already have a host object for this host name, apply maxoids
  // to this also (this is so max oids can be changed on-the-fly on the IOC
  // and not only before the host object is created)
  devSnmp_host *pHost = findHost(host);
  if (pHost) pHost->setMaxOidsPerReq( pFound->maxOidsPerReq() );
}
//--------------------------------------------------------------------
int devSnmp_manager::getHostMaxOidsPerReq(char *host)
{
  // return max oids in snmpHostDataList if matching host found
  devSnmp_hostdata *pFound = findHostData(host);
  if (pFound)
    return(pFound->maxOidsPerReq());
  else
    // if no item found in list, use default
    return(DEFAULT_MAX_OIDS_PER_REQ);
}
//--------------------------------------------------------------------
devSnmp_device *devSnmp_manager::createDevice
     (struct dbCommon *pRec,
      struct link     *pLink,
      char            *type)
{

  printf("devSnmp_manager::createDevice: %s", type);

  // create device object (send NULL for group, for now)
  bool okay;
  devSnmp_device *pDevice = new devSnmp_device(this,NULL,pRec,pLink,type,&okay);
  if (! pDevice) return(NULL);
  if (! okay) {
    delete pDevice;
    return(NULL);
  }

  // point record's dpvt at created device object
  pRec->dpvt = pDevice;

  // try to locate existing host device belongs to
  devSnmp_host *pHost = findHost((char *)pDevice->hostName());

  if (! pHost) {
    // existing host NOT found, need to create it
    pHost = createHost((char *)pDevice->hostName());
    if (! pHost) {
      printf("FAILED to created host %s\n",pDevice->hostName());
      delete pDevice;
      return(NULL);
    }
  }

  // host found (or created), add device to it
  pHost->addDevice(pDevice);

  return(pDevice);
}
//--------------------------------------------------------------------
devSnmp_host *devSnmp_manager::createHost(char *host)
{
  // create host structure
  bool okay;
  devSnmp_host *pHost = new devSnmp_host(this,host,&okay);
  if (! pHost) return(NULL);
  if (! okay) {
    delete pHost;
    return(NULL);
  }

  // add created host to host list
  snmpHostList->append(pHost);

  // keep host list sorted
  // (no real reason, just makes reports nicer)
  snmpHostList->sort(devSnmp_host_compare);

  // done
  return(pHost);
}
//--------------------------------------------------------------------
devSnmp_host *devSnmp_manager::findHost(char *host)
{
  for (int ii = 0; ii < snmpHostList->count(); ii++) {
    devSnmp_host *pHost = (devSnmp_host *) snmpHostList->itemAt(ii);
    if (! pHost) continue;
    if (strcmp(host, pHost->hostName()) == 0) return(pHost);
  }
  return(NULL);
}
//--------------------------------------------------------------------
devSnmp_hostdata *devSnmp_manager::findHostData(char *host)
{
  for (int ii = 0; ii < snmpHostDataList->count(); ii++) {
    devSnmp_hostdata *pHostData = (devSnmp_hostdata *) snmpHostDataList->itemAt(ii);
    if (! pHostData) continue;
    if (strcmp(host, pHostData->hostName()) == 0) return(pHostData);
  }
  return(NULL);
}
//--------------------------------------------------------------------
void devSnmp_manager::processing(void)
{
  // roll through each host, having it process
  for (int host_idx = 0; host_idx < snmpHostList->count(); host_idx++) {
    devSnmp_host *pHost = (devSnmp_host *) snmpHostList->itemAt(host_idx);
    if (! pHost) continue;
    pHost->processing();
  }
}
//--------------------------------------------------------------------
void devSnmp_manager::zeroCounters(void)
{
  // clear task counters
  readTaskStart.start();
  sendTaskStart.start();
  readTaskLoops = 0;
  sendTaskLoops = 0;

  // roll through each host, having it zero counters
  for (int host_idx = 0; host_idx < snmpHostList->count(); host_idx++) {
    devSnmp_host *pHost = (devSnmp_host *) snmpHostList->itemAt(host_idx);
    if (! pHost) continue;
    pHost->zeroCounters();
  }
}
//--------------------------------------------------------------------
void devSnmp_manager::report(int level, char *match)
{
  if ((! snmpHostList) || (snmpHostList->count() == 0)) {
    printf("no SNMP devices.\n");
    return;
  }

  // show task counter data
  printf("%s\n",GROUP_BAR);
  double sec = readTaskStart.elapsedSeconds();
  double loopps = (sec < 0.01) ? 0.0 : (readTaskLoops / sec);
  printf("read task: %ld loops (%.1lf per sec)\n",readTaskLoops,loopps);
  sec = sendTaskStart.elapsedSeconds();
  loopps = (sec < 0.01) ? 0.0 : (sendTaskLoops / sec);
  printf("send task: %ld loops (%.1lf per sec)\n",sendTaskLoops,loopps);
  printf("\n");

  // roll through each host/community group, having it report
  for (int host_idx = 0; host_idx < snmpHostList->count(); host_idx++) {
    devSnmp_host *pHost = (devSnmp_host *) snmpHostList->itemAt(host_idx);
    if (! pHost) continue;
    pHost->report(level,match);
  }
}
//--------------------------------------------------------------------
// class pollAggregate
//--------------------------------------------------------------------
pollAggregate::pollAggregate(void)
{
  lists = new pointerList();
}
//--------------------------------------------------------------------
pollAggregate::~pollAggregate(void)
{
  if (lists) {
    for (int ii = lists->count()-1; ii >= 0; ii--) {
      pointerList *pList = (pointerList *) lists->removeItemAt(ii);
      if (! pList) continue;
      delete pList;
    }
    delete lists;
    lists = NULL;
  }
}
//--------------------------------------------------------------------
void pollAggregate::addDevice(devSnmp_device *pDevice)
{
  unsigned long msec = pDevice->getPollGapMSec();
  pointerList *pList = findList(msec);
  if (! pList) {
    pList = new pointerList();
    pList->setContext((void *)msec);
    lists->append(pList);
  }
  pList->append(pDevice);
}
//--------------------------------------------------------------------
pointerList *pollAggregate::findList(unsigned long msec)
{
  for (int ii = 0; ii < lists->count(); ii++) {
    pointerList *pList = (pointerList *) lists->itemAt(ii);
    if (! pList) continue;
    unsigned long thisctxt = (unsigned long) pList->context();
    if (thisctxt == msec) return(pList);
  }
  return(NULL);
}
//--------------------------------------------------------------------
void pollAggregate::report(void)
{
  if ((! lists) || (lists->count() == 0)) return;
  lists->sort(list_context_compare);
  printf("  Poll rate  Devices  Avg rate\n");
  printf("  ---------  -------  ---------\n");
  for (int ii = 0; ii < lists->count(); ii++) {
    pointerList *pList = (pointerList *) lists->itemAt(ii);
    if (! pList) continue;
    unsigned long msec = (unsigned long) pList->context();
    double rate = (msec == 0) ? 0.0 : (1000.0 / msec);
    double tot = 0.0;
    for (int jj = 0; jj < pList->count(); jj++) {
      devSnmp_device *pDevice = (devSnmp_device *) pList->itemAt(jj);
      if (! pDevice) continue;
      tot += pDevice->pollsPerSecond();
    }
    double avgr = tot / pList->count();
    printf("  %9.3lf  %7d  %9.3lf\n",rate,pList->count(),avgr);
  }
}
//--------------------------------------------------------------------


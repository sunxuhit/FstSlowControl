
#ifndef INCscalcoutWAITH
#define INCscalcoutWAITH
typedef enum {
	scalcoutWAIT_NoWait,
	scalcoutWAIT_Wait
}scalcoutWAIT;
#endif /*INCscalcoutWAITH*/

#ifndef INCscalcoutOOPTH
#define INCscalcoutOOPTH
typedef enum {
	scalcoutOOPT_Every_Time,
	scalcoutOOPT_On_Change,
	scalcoutOOPT_When_Zero,
	scalcoutOOPT_When_Non_zero,
	scalcoutOOPT_Transition_To_Zero,
	scalcoutOOPT_Transition_To_Non_zero,
	scalcoutOOPT_Never
}scalcoutOOPT;
#endif /*INCscalcoutOOPTH*/

#ifndef INCscalcoutINAVH
#define INCscalcoutINAVH
typedef enum {
	scalcoutINAV_EXT_NC,
	scalcoutINAV_EXT,
	scalcoutINAV_LOC,
	scalcoutINAV_CON
}scalcoutINAV;
#endif /*INCscalcoutINAVH*/

#ifndef INCscalcoutINAPH
#define INCscalcoutINAPH
typedef enum {
	scalcoutINAP_No,
	scalcoutINAP_Yes
}scalcoutINAP;
#endif /*INCscalcoutINAPH*/

#ifndef INCscalcoutDOPTH
#define INCscalcoutDOPTH
typedef enum {
	scalcoutDOPT_Use_VAL,
	scalcoutDOPT_Use_OVAL
}scalcoutDOPT;
#endif /*INCscalcoutDOPTH*/
#ifndef INCscalcoutH
#define INCscalcoutH
#include "epicsTypes.h"
#include "link.h"
#include "epicsMutex.h"
#include "ellLib.h"
#include "epicsTime.h"
typedef struct scalcoutRecord {
	char		name[61];	/* Record Name */
	char		desc[41];	/* Descriptor */
	char		asg[29];	/* Access Security Group */
	epicsEnum16	scan;	/* Scan Mechanism */
	epicsEnum16	pini;	/* Process at iocInit */
	epicsInt16	phas;	/* Scan Phase */
	epicsInt16	evnt;	/* Event Number */
	epicsInt16	tse;	/* Time Stamp Event */
	DBLINK		tsel;	/* Time Stamp Link */
	epicsEnum16	dtyp;	/* Device Type */
	epicsInt16	disv;	/* Disable Value */
	epicsInt16	disa;	/* Disable */
	DBLINK		sdis;	/* Scanning Disable */
	epicsMutexId	mlok;	/* Monitor lock */
	ELLLIST		mlis;	/* Monitor List */
	epicsUInt8	disp;	/* Disable putField */
	epicsUInt8	proc;	/* Force Processing */
	epicsEnum16	stat;	/* Alarm Status */
	epicsEnum16	sevr;	/* Alarm Severity */
	epicsEnum16	nsta;	/* New Alarm Status */
	epicsEnum16	nsev;	/* New Alarm Severity */
	epicsEnum16	acks;	/* Alarm Ack Severity */
	epicsEnum16	ackt;	/* Alarm Ack Transient */
	epicsEnum16	diss;	/* Disable Alarm Sevrty */
	epicsUInt8	lcnt;	/* Lock Count */
	epicsUInt8	pact;	/* Record active */
	epicsUInt8	putf;	/* dbPutField process */
	epicsUInt8	rpro;	/* Reprocess  */
	struct asgMember *asp;	/* Access Security Pvt */
	struct putNotify *ppn;	/* addr of PUTNOTIFY */
	struct putNotifyRecord *ppnr;	/* pputNotifyRecord */
	struct scan_element *spvt;	/* Scan Private */
	struct rset	*rset;	/* Address of RSET */
	struct dset	*dset;	/* DSET address */
	void		*dpvt;	/* Device Private */
	struct dbRecordType *rdes;	/* Address of dbRecordType */
	struct lockRecord *lset;	/* Lock Set */
	epicsEnum16	prio;	/* Scheduling Priority */
	epicsUInt8	tpro;	/* Trace Processing */
	char bkpt;	/* Break Point */
	epicsUInt8	udf;	/* Undefined */
	epicsTimeStamp	time;	/* Time */
	DBLINK		flnk;	/* Forward Process Link */
	epicsFloat64	vers;	/* Code Version */
	void *rpvt;	/* Record Private */
	epicsFloat64	val;	/* Result */
	char		sval[40];	/* String result */
	epicsFloat64	pval;	/* Previous Value */
	char		psvl[40];	/* Previous string result */
	char		calc[40];	/* Calculation */
	epicsInt32	clcv;	/* CALC Valid */
	DBLINK		inpa;	/* Input A */
	DBLINK		inpb;	/* Input B */
	DBLINK		inpc;	/* Input C */
	DBLINK		inpd;	/* Input D */
	DBLINK		inpe;	/* Input E */
	DBLINK		inpf;	/* Input F */
	DBLINK		inpg;	/* Input G */
	DBLINK		inph;	/* Input H */
	DBLINK		inpi;	/* Input I */
	DBLINK		inpj;	/* Input J */
	DBLINK		inpk;	/* Input K */
	DBLINK		inpl;	/* Input L */
	DBLINK		inaa;	/* String input AA */
	DBLINK		inbb;	/* String input BB */
	DBLINK		incc;	/* String input CC */
	DBLINK		indd;	/* String input DD */
	DBLINK		inee;	/* String input EE */
	DBLINK		inff;	/* String input FF */
	DBLINK		ingg;	/* String input GG */
	DBLINK		inhh;	/* String input HH */
	DBLINK		inii;	/* String input II */
	DBLINK		injj;	/* String input JJ */
	DBLINK		inkk;	/* String input KK */
	DBLINK		inll;	/* String input LL */
	DBLINK		out;	/* Output Link */
	epicsEnum16	inav;	/* INPA PV Status */
	epicsEnum16	inbv;	/* INPB PV Status */
	epicsEnum16	incv;	/* INPC PV Status */
	epicsEnum16	indv;	/* INPD PV Status */
	epicsEnum16	inev;	/* INPE PV Status */
	epicsEnum16	infv;	/* INPF PV Status */
	epicsEnum16	ingv;	/* INPG PV Status */
	epicsEnum16	inhv;	/* INPH PV Status */
	epicsEnum16	iniv;	/* INPI PV Status */
	epicsEnum16	injv;	/* INPJ PV Status */
	epicsEnum16	inkv;	/* INPK PV Status */
	epicsEnum16	inlv;	/* INPL PV Status */
	epicsEnum16	iaav;	/* INAA PV Status */
	epicsEnum16	ibbv;	/* INBB PV Status */
	epicsEnum16	iccv;	/* INCC PV Status */
	epicsEnum16	iddv;	/* INDD PV Status */
	epicsEnum16	ieev;	/* INEE PV Status */
	epicsEnum16	iffv;	/* INFF PV Status */
	epicsEnum16	iggv;	/* INGG PV Status */
	epicsEnum16	ihhv;	/* INHH PV Status */
	epicsEnum16	iiiv;	/* INII PV Status */
	epicsEnum16	ijjv;	/* INJJ PV Status */
	epicsEnum16	ikkv;	/* INKK PV Status */
	epicsEnum16	illv;	/* INLL PV Status */
	epicsEnum16	outv;	/* OUT PV Status */
	epicsEnum16	oopt;	/* Output Execute Opt */
	epicsFloat64	odly;	/* Output Execute Delay */
	epicsEnum16	wait;	/* Wait for completion? */
	epicsUInt16	dlya;	/* Output Delay Active */
	epicsEnum16	dopt;	/* Output Data Opt */
	char		ocal[36];	/* Output Calculation */
	epicsInt32	oclv;	/* OCAL Valid */
	epicsUInt16	oevt;	/* Event To Issue */
	epicsEnum16	ivoa;	/* INVALID output action */
	epicsFloat64	ivov;	/* INVALID output value */
	char		egu[16];	/* Units Name */
	epicsInt16	prec;	/* Display Precision */
	epicsFloat64	hopr;	/* High Operating Rng */
	epicsFloat64	lopr;	/* Low Operating Range */
	epicsFloat64	hihi;	/* Hihi Alarm Limit */
	epicsFloat64	lolo;	/* Lolo Alarm Limit */
	epicsFloat64	high;	/* High Alarm Limit */
	epicsFloat64	low;	/* Low Alarm Limit */
	epicsEnum16	hhsv;	/* Hihi Severity */
	epicsEnum16	llsv;	/* Lolo Severity */
	epicsEnum16	hsv;	/* High Severity */
	epicsEnum16	lsv;	/* Low Severity */
	epicsFloat64	hyst;	/* Alarm Deadband */
	epicsFloat64	adel;	/* Archive Deadband */
	epicsFloat64	mdel;	/* Monitor Deadband */
	epicsFloat64	a;	/* Value of Input A */
	epicsFloat64	b;	/* Value of Input B */
	epicsFloat64	c;	/* Value of Input C */
	epicsFloat64	d;	/* Value of Input D */
	epicsFloat64	e;	/* Value of Input E */
	epicsFloat64	f;	/* Value of Input F */
	epicsFloat64	g;	/* Value of Input G */
	epicsFloat64	h;	/* Value of Input H */
	epicsFloat64	i;	/* Value of Input I */
	epicsFloat64	j;	/* Value of Input J */
	epicsFloat64	k;	/* Value of Input K */
	epicsFloat64	l;	/* Value of Input L */
	char **strs;	/* Array of string pointers */
	char		aa[40];	/* Value of string input AA */
	char		bb[40];	/* Value of string input BB */
	char		cc[40];	/* Value of string input CC */
	char		dd[40];	/* Value of string input DD */
	char		ee[40];	/* Value of string input EE */
	char		ff[40];	/* Value of string input FF */
	char		gg[40];	/* Value of string input GG */
	char		hh[40];	/* Value of string input HH */
	char		ii[40];	/* Value of string input II */
	char		jj[40];	/* Value of string input JJ */
	char		kk[40];	/* Value of string input KK */
	char		ll[40];	/* Value of string input LL */
	char *paa;	/* Prev Value of AA */
	char *pbb;	/* Prev Value of BB */
	char *pcc;	/* Prev Value of CC */
	char *pdd;	/* Prev Value of DD */
	char *pee;	/* Prev Value of EE */
	char *pff;	/* Prev Value of FF */
	char *pgg;	/* Prev Value of GG */
	char *phh;	/* Prev Value of HH */
	char *pii;	/* Prev Value of II */
	char *pjj;	/* Prev Value of JJ */
	char *pkk;	/* Prev Value of KK */
	char *pll;	/* Prev Value of LL */
	epicsFloat64	oval;	/* Output Value */
	char		osv[40];	/* Output string value */
	char		posv[40];	/* Previous output string value */
	epicsFloat64	pa;	/* Prev Value of A */
	epicsFloat64	pb;	/* Prev Value of B */
	epicsFloat64	pc;	/* Prev Value of C */
	epicsFloat64	pd;	/* Prev Value of D */
	epicsFloat64	pe;	/* Prev Value of E */
	epicsFloat64	pf;	/* Prev Value of F */
	epicsFloat64	pg;	/* Prev Value of G */
	epicsFloat64	ph;	/* Prev Value of H */
	epicsFloat64	pi;	/* Prev Value of I */
	epicsFloat64	pj;	/* Prev Value of J */
	epicsFloat64	pk;	/* Prev Value of K */
	epicsFloat64	pl;	/* Prev Value of L */
	epicsFloat64	povl;	/* Prev Value of OVAL */
	epicsFloat64	lalm;	/* Last Value Alarmed */
	epicsFloat64	alst;	/* Last Value Archived */
	epicsFloat64	mlst;	/* Last Val Monitored */
	char	rpcl[240];	/* Reverse Polish Calc */
	char	orpc[240];	/* Reverse Polish OCalc */
} scalcoutRecord;
#define scalcoutRecordNAME	0
#define scalcoutRecordDESC	1
#define scalcoutRecordASG	2
#define scalcoutRecordSCAN	3
#define scalcoutRecordPINI	4
#define scalcoutRecordPHAS	5
#define scalcoutRecordEVNT	6
#define scalcoutRecordTSE	7
#define scalcoutRecordTSEL	8
#define scalcoutRecordDTYP	9
#define scalcoutRecordDISV	10
#define scalcoutRecordDISA	11
#define scalcoutRecordSDIS	12
#define scalcoutRecordMLOK	13
#define scalcoutRecordMLIS	14
#define scalcoutRecordDISP	15
#define scalcoutRecordPROC	16
#define scalcoutRecordSTAT	17
#define scalcoutRecordSEVR	18
#define scalcoutRecordNSTA	19
#define scalcoutRecordNSEV	20
#define scalcoutRecordACKS	21
#define scalcoutRecordACKT	22
#define scalcoutRecordDISS	23
#define scalcoutRecordLCNT	24
#define scalcoutRecordPACT	25
#define scalcoutRecordPUTF	26
#define scalcoutRecordRPRO	27
#define scalcoutRecordASP	28
#define scalcoutRecordPPN	29
#define scalcoutRecordPPNR	30
#define scalcoutRecordSPVT	31
#define scalcoutRecordRSET	32
#define scalcoutRecordDSET	33
#define scalcoutRecordDPVT	34
#define scalcoutRecordRDES	35
#define scalcoutRecordLSET	36
#define scalcoutRecordPRIO	37
#define scalcoutRecordTPRO	38
#define scalcoutRecordBKPT	39
#define scalcoutRecordUDF	40
#define scalcoutRecordTIME	41
#define scalcoutRecordFLNK	42
#define scalcoutRecordVERS	43
#define scalcoutRecordRPVT	44
#define scalcoutRecordVAL	45
#define scalcoutRecordSVAL	46
#define scalcoutRecordPVAL	47
#define scalcoutRecordPSVL	48
#define scalcoutRecordCALC	49
#define scalcoutRecordCLCV	50
#define scalcoutRecordINPA	51
#define scalcoutRecordINPB	52
#define scalcoutRecordINPC	53
#define scalcoutRecordINPD	54
#define scalcoutRecordINPE	55
#define scalcoutRecordINPF	56
#define scalcoutRecordINPG	57
#define scalcoutRecordINPH	58
#define scalcoutRecordINPI	59
#define scalcoutRecordINPJ	60
#define scalcoutRecordINPK	61
#define scalcoutRecordINPL	62
#define scalcoutRecordINAA	63
#define scalcoutRecordINBB	64
#define scalcoutRecordINCC	65
#define scalcoutRecordINDD	66
#define scalcoutRecordINEE	67
#define scalcoutRecordINFF	68
#define scalcoutRecordINGG	69
#define scalcoutRecordINHH	70
#define scalcoutRecordINII	71
#define scalcoutRecordINJJ	72
#define scalcoutRecordINKK	73
#define scalcoutRecordINLL	74
#define scalcoutRecordOUT	75
#define scalcoutRecordINAV	76
#define scalcoutRecordINBV	77
#define scalcoutRecordINCV	78
#define scalcoutRecordINDV	79
#define scalcoutRecordINEV	80
#define scalcoutRecordINFV	81
#define scalcoutRecordINGV	82
#define scalcoutRecordINHV	83
#define scalcoutRecordINIV	84
#define scalcoutRecordINJV	85
#define scalcoutRecordINKV	86
#define scalcoutRecordINLV	87
#define scalcoutRecordIAAV	88
#define scalcoutRecordIBBV	89
#define scalcoutRecordICCV	90
#define scalcoutRecordIDDV	91
#define scalcoutRecordIEEV	92
#define scalcoutRecordIFFV	93
#define scalcoutRecordIGGV	94
#define scalcoutRecordIHHV	95
#define scalcoutRecordIIIV	96
#define scalcoutRecordIJJV	97
#define scalcoutRecordIKKV	98
#define scalcoutRecordILLV	99
#define scalcoutRecordOUTV	100
#define scalcoutRecordOOPT	101
#define scalcoutRecordODLY	102
#define scalcoutRecordWAIT	103
#define scalcoutRecordDLYA	104
#define scalcoutRecordDOPT	105
#define scalcoutRecordOCAL	106
#define scalcoutRecordOCLV	107
#define scalcoutRecordOEVT	108
#define scalcoutRecordIVOA	109
#define scalcoutRecordIVOV	110
#define scalcoutRecordEGU	111
#define scalcoutRecordPREC	112
#define scalcoutRecordHOPR	113
#define scalcoutRecordLOPR	114
#define scalcoutRecordHIHI	115
#define scalcoutRecordLOLO	116
#define scalcoutRecordHIGH	117
#define scalcoutRecordLOW	118
#define scalcoutRecordHHSV	119
#define scalcoutRecordLLSV	120
#define scalcoutRecordHSV	121
#define scalcoutRecordLSV	122
#define scalcoutRecordHYST	123
#define scalcoutRecordADEL	124
#define scalcoutRecordMDEL	125
#define scalcoutRecordA	126
#define scalcoutRecordB	127
#define scalcoutRecordC	128
#define scalcoutRecordD	129
#define scalcoutRecordE	130
#define scalcoutRecordF	131
#define scalcoutRecordG	132
#define scalcoutRecordH	133
#define scalcoutRecordI	134
#define scalcoutRecordJ	135
#define scalcoutRecordK	136
#define scalcoutRecordL	137
#define scalcoutRecordSTRS	138
#define scalcoutRecordAA	139
#define scalcoutRecordBB	140
#define scalcoutRecordCC	141
#define scalcoutRecordDD	142
#define scalcoutRecordEE	143
#define scalcoutRecordFF	144
#define scalcoutRecordGG	145
#define scalcoutRecordHH	146
#define scalcoutRecordII	147
#define scalcoutRecordJJ	148
#define scalcoutRecordKK	149
#define scalcoutRecordLL	150
#define scalcoutRecordPAA	151
#define scalcoutRecordPBB	152
#define scalcoutRecordPCC	153
#define scalcoutRecordPDD	154
#define scalcoutRecordPEE	155
#define scalcoutRecordPFF	156
#define scalcoutRecordPGG	157
#define scalcoutRecordPHH	158
#define scalcoutRecordPII	159
#define scalcoutRecordPJJ	160
#define scalcoutRecordPKK	161
#define scalcoutRecordPLL	162
#define scalcoutRecordOVAL	163
#define scalcoutRecordOSV	164
#define scalcoutRecordPOSV	165
#define scalcoutRecordPA	166
#define scalcoutRecordPB	167
#define scalcoutRecordPC	168
#define scalcoutRecordPD	169
#define scalcoutRecordPE	170
#define scalcoutRecordPF	171
#define scalcoutRecordPG	172
#define scalcoutRecordPH	173
#define scalcoutRecordPI	174
#define scalcoutRecordPJ	175
#define scalcoutRecordPK	176
#define scalcoutRecordPL	177
#define scalcoutRecordPOVL	178
#define scalcoutRecordLALM	179
#define scalcoutRecordALST	180
#define scalcoutRecordMLST	181
#define scalcoutRecordRPCL	182
#define scalcoutRecordORPC	183
#endif /*INCscalcoutH*/
#ifdef GEN_SIZE_OFFSET
#ifdef __cplusplus
extern "C" {
#endif
#include <epicsExport.h>
static int scalcoutRecordSizeOffset(dbRecordType *pdbRecordType)
{
    scalcoutRecord *prec = 0;
  pdbRecordType->papFldDes[0]->size=sizeof(prec->name);
  pdbRecordType->papFldDes[0]->offset=(short)((char *)&prec->name - (char *)prec);
  pdbRecordType->papFldDes[1]->size=sizeof(prec->desc);
  pdbRecordType->papFldDes[1]->offset=(short)((char *)&prec->desc - (char *)prec);
  pdbRecordType->papFldDes[2]->size=sizeof(prec->asg);
  pdbRecordType->papFldDes[2]->offset=(short)((char *)&prec->asg - (char *)prec);
  pdbRecordType->papFldDes[3]->size=sizeof(prec->scan);
  pdbRecordType->papFldDes[3]->offset=(short)((char *)&prec->scan - (char *)prec);
  pdbRecordType->papFldDes[4]->size=sizeof(prec->pini);
  pdbRecordType->papFldDes[4]->offset=(short)((char *)&prec->pini - (char *)prec);
  pdbRecordType->papFldDes[5]->size=sizeof(prec->phas);
  pdbRecordType->papFldDes[5]->offset=(short)((char *)&prec->phas - (char *)prec);
  pdbRecordType->papFldDes[6]->size=sizeof(prec->evnt);
  pdbRecordType->papFldDes[6]->offset=(short)((char *)&prec->evnt - (char *)prec);
  pdbRecordType->papFldDes[7]->size=sizeof(prec->tse);
  pdbRecordType->papFldDes[7]->offset=(short)((char *)&prec->tse - (char *)prec);
  pdbRecordType->papFldDes[8]->size=sizeof(prec->tsel);
  pdbRecordType->papFldDes[8]->offset=(short)((char *)&prec->tsel - (char *)prec);
  pdbRecordType->papFldDes[9]->size=sizeof(prec->dtyp);
  pdbRecordType->papFldDes[9]->offset=(short)((char *)&prec->dtyp - (char *)prec);
  pdbRecordType->papFldDes[10]->size=sizeof(prec->disv);
  pdbRecordType->papFldDes[10]->offset=(short)((char *)&prec->disv - (char *)prec);
  pdbRecordType->papFldDes[11]->size=sizeof(prec->disa);
  pdbRecordType->papFldDes[11]->offset=(short)((char *)&prec->disa - (char *)prec);
  pdbRecordType->papFldDes[12]->size=sizeof(prec->sdis);
  pdbRecordType->papFldDes[12]->offset=(short)((char *)&prec->sdis - (char *)prec);
  pdbRecordType->papFldDes[13]->size=sizeof(prec->mlok);
  pdbRecordType->papFldDes[13]->offset=(short)((char *)&prec->mlok - (char *)prec);
  pdbRecordType->papFldDes[14]->size=sizeof(prec->mlis);
  pdbRecordType->papFldDes[14]->offset=(short)((char *)&prec->mlis - (char *)prec);
  pdbRecordType->papFldDes[15]->size=sizeof(prec->disp);
  pdbRecordType->papFldDes[15]->offset=(short)((char *)&prec->disp - (char *)prec);
  pdbRecordType->papFldDes[16]->size=sizeof(prec->proc);
  pdbRecordType->papFldDes[16]->offset=(short)((char *)&prec->proc - (char *)prec);
  pdbRecordType->papFldDes[17]->size=sizeof(prec->stat);
  pdbRecordType->papFldDes[17]->offset=(short)((char *)&prec->stat - (char *)prec);
  pdbRecordType->papFldDes[18]->size=sizeof(prec->sevr);
  pdbRecordType->papFldDes[18]->offset=(short)((char *)&prec->sevr - (char *)prec);
  pdbRecordType->papFldDes[19]->size=sizeof(prec->nsta);
  pdbRecordType->papFldDes[19]->offset=(short)((char *)&prec->nsta - (char *)prec);
  pdbRecordType->papFldDes[20]->size=sizeof(prec->nsev);
  pdbRecordType->papFldDes[20]->offset=(short)((char *)&prec->nsev - (char *)prec);
  pdbRecordType->papFldDes[21]->size=sizeof(prec->acks);
  pdbRecordType->papFldDes[21]->offset=(short)((char *)&prec->acks - (char *)prec);
  pdbRecordType->papFldDes[22]->size=sizeof(prec->ackt);
  pdbRecordType->papFldDes[22]->offset=(short)((char *)&prec->ackt - (char *)prec);
  pdbRecordType->papFldDes[23]->size=sizeof(prec->diss);
  pdbRecordType->papFldDes[23]->offset=(short)((char *)&prec->diss - (char *)prec);
  pdbRecordType->papFldDes[24]->size=sizeof(prec->lcnt);
  pdbRecordType->papFldDes[24]->offset=(short)((char *)&prec->lcnt - (char *)prec);
  pdbRecordType->papFldDes[25]->size=sizeof(prec->pact);
  pdbRecordType->papFldDes[25]->offset=(short)((char *)&prec->pact - (char *)prec);
  pdbRecordType->papFldDes[26]->size=sizeof(prec->putf);
  pdbRecordType->papFldDes[26]->offset=(short)((char *)&prec->putf - (char *)prec);
  pdbRecordType->papFldDes[27]->size=sizeof(prec->rpro);
  pdbRecordType->papFldDes[27]->offset=(short)((char *)&prec->rpro - (char *)prec);
  pdbRecordType->papFldDes[28]->size=sizeof(prec->asp);
  pdbRecordType->papFldDes[28]->offset=(short)((char *)&prec->asp - (char *)prec);
  pdbRecordType->papFldDes[29]->size=sizeof(prec->ppn);
  pdbRecordType->papFldDes[29]->offset=(short)((char *)&prec->ppn - (char *)prec);
  pdbRecordType->papFldDes[30]->size=sizeof(prec->ppnr);
  pdbRecordType->papFldDes[30]->offset=(short)((char *)&prec->ppnr - (char *)prec);
  pdbRecordType->papFldDes[31]->size=sizeof(prec->spvt);
  pdbRecordType->papFldDes[31]->offset=(short)((char *)&prec->spvt - (char *)prec);
  pdbRecordType->papFldDes[32]->size=sizeof(prec->rset);
  pdbRecordType->papFldDes[32]->offset=(short)((char *)&prec->rset - (char *)prec);
  pdbRecordType->papFldDes[33]->size=sizeof(prec->dset);
  pdbRecordType->papFldDes[33]->offset=(short)((char *)&prec->dset - (char *)prec);
  pdbRecordType->papFldDes[34]->size=sizeof(prec->dpvt);
  pdbRecordType->papFldDes[34]->offset=(short)((char *)&prec->dpvt - (char *)prec);
  pdbRecordType->papFldDes[35]->size=sizeof(prec->rdes);
  pdbRecordType->papFldDes[35]->offset=(short)((char *)&prec->rdes - (char *)prec);
  pdbRecordType->papFldDes[36]->size=sizeof(prec->lset);
  pdbRecordType->papFldDes[36]->offset=(short)((char *)&prec->lset - (char *)prec);
  pdbRecordType->papFldDes[37]->size=sizeof(prec->prio);
  pdbRecordType->papFldDes[37]->offset=(short)((char *)&prec->prio - (char *)prec);
  pdbRecordType->papFldDes[38]->size=sizeof(prec->tpro);
  pdbRecordType->papFldDes[38]->offset=(short)((char *)&prec->tpro - (char *)prec);
  pdbRecordType->papFldDes[39]->size=sizeof(prec->bkpt);
  pdbRecordType->papFldDes[39]->offset=(short)((char *)&prec->bkpt - (char *)prec);
  pdbRecordType->papFldDes[40]->size=sizeof(prec->udf);
  pdbRecordType->papFldDes[40]->offset=(short)((char *)&prec->udf - (char *)prec);
  pdbRecordType->papFldDes[41]->size=sizeof(prec->time);
  pdbRecordType->papFldDes[41]->offset=(short)((char *)&prec->time - (char *)prec);
  pdbRecordType->papFldDes[42]->size=sizeof(prec->flnk);
  pdbRecordType->papFldDes[42]->offset=(short)((char *)&prec->flnk - (char *)prec);
  pdbRecordType->papFldDes[43]->size=sizeof(prec->vers);
  pdbRecordType->papFldDes[43]->offset=(short)((char *)&prec->vers - (char *)prec);
  pdbRecordType->papFldDes[44]->size=sizeof(prec->rpvt);
  pdbRecordType->papFldDes[44]->offset=(short)((char *)&prec->rpvt - (char *)prec);
  pdbRecordType->papFldDes[45]->size=sizeof(prec->val);
  pdbRecordType->papFldDes[45]->offset=(short)((char *)&prec->val - (char *)prec);
  pdbRecordType->papFldDes[46]->size=sizeof(prec->sval);
  pdbRecordType->papFldDes[46]->offset=(short)((char *)&prec->sval - (char *)prec);
  pdbRecordType->papFldDes[47]->size=sizeof(prec->pval);
  pdbRecordType->papFldDes[47]->offset=(short)((char *)&prec->pval - (char *)prec);
  pdbRecordType->papFldDes[48]->size=sizeof(prec->psvl);
  pdbRecordType->papFldDes[48]->offset=(short)((char *)&prec->psvl - (char *)prec);
  pdbRecordType->papFldDes[49]->size=sizeof(prec->calc);
  pdbRecordType->papFldDes[49]->offset=(short)((char *)&prec->calc - (char *)prec);
  pdbRecordType->papFldDes[50]->size=sizeof(prec->clcv);
  pdbRecordType->papFldDes[50]->offset=(short)((char *)&prec->clcv - (char *)prec);
  pdbRecordType->papFldDes[51]->size=sizeof(prec->inpa);
  pdbRecordType->papFldDes[51]->offset=(short)((char *)&prec->inpa - (char *)prec);
  pdbRecordType->papFldDes[52]->size=sizeof(prec->inpb);
  pdbRecordType->papFldDes[52]->offset=(short)((char *)&prec->inpb - (char *)prec);
  pdbRecordType->papFldDes[53]->size=sizeof(prec->inpc);
  pdbRecordType->papFldDes[53]->offset=(short)((char *)&prec->inpc - (char *)prec);
  pdbRecordType->papFldDes[54]->size=sizeof(prec->inpd);
  pdbRecordType->papFldDes[54]->offset=(short)((char *)&prec->inpd - (char *)prec);
  pdbRecordType->papFldDes[55]->size=sizeof(prec->inpe);
  pdbRecordType->papFldDes[55]->offset=(short)((char *)&prec->inpe - (char *)prec);
  pdbRecordType->papFldDes[56]->size=sizeof(prec->inpf);
  pdbRecordType->papFldDes[56]->offset=(short)((char *)&prec->inpf - (char *)prec);
  pdbRecordType->papFldDes[57]->size=sizeof(prec->inpg);
  pdbRecordType->papFldDes[57]->offset=(short)((char *)&prec->inpg - (char *)prec);
  pdbRecordType->papFldDes[58]->size=sizeof(prec->inph);
  pdbRecordType->papFldDes[58]->offset=(short)((char *)&prec->inph - (char *)prec);
  pdbRecordType->papFldDes[59]->size=sizeof(prec->inpi);
  pdbRecordType->papFldDes[59]->offset=(short)((char *)&prec->inpi - (char *)prec);
  pdbRecordType->papFldDes[60]->size=sizeof(prec->inpj);
  pdbRecordType->papFldDes[60]->offset=(short)((char *)&prec->inpj - (char *)prec);
  pdbRecordType->papFldDes[61]->size=sizeof(prec->inpk);
  pdbRecordType->papFldDes[61]->offset=(short)((char *)&prec->inpk - (char *)prec);
  pdbRecordType->papFldDes[62]->size=sizeof(prec->inpl);
  pdbRecordType->papFldDes[62]->offset=(short)((char *)&prec->inpl - (char *)prec);
  pdbRecordType->papFldDes[63]->size=sizeof(prec->inaa);
  pdbRecordType->papFldDes[63]->offset=(short)((char *)&prec->inaa - (char *)prec);
  pdbRecordType->papFldDes[64]->size=sizeof(prec->inbb);
  pdbRecordType->papFldDes[64]->offset=(short)((char *)&prec->inbb - (char *)prec);
  pdbRecordType->papFldDes[65]->size=sizeof(prec->incc);
  pdbRecordType->papFldDes[65]->offset=(short)((char *)&prec->incc - (char *)prec);
  pdbRecordType->papFldDes[66]->size=sizeof(prec->indd);
  pdbRecordType->papFldDes[66]->offset=(short)((char *)&prec->indd - (char *)prec);
  pdbRecordType->papFldDes[67]->size=sizeof(prec->inee);
  pdbRecordType->papFldDes[67]->offset=(short)((char *)&prec->inee - (char *)prec);
  pdbRecordType->papFldDes[68]->size=sizeof(prec->inff);
  pdbRecordType->papFldDes[68]->offset=(short)((char *)&prec->inff - (char *)prec);
  pdbRecordType->papFldDes[69]->size=sizeof(prec->ingg);
  pdbRecordType->papFldDes[69]->offset=(short)((char *)&prec->ingg - (char *)prec);
  pdbRecordType->papFldDes[70]->size=sizeof(prec->inhh);
  pdbRecordType->papFldDes[70]->offset=(short)((char *)&prec->inhh - (char *)prec);
  pdbRecordType->papFldDes[71]->size=sizeof(prec->inii);
  pdbRecordType->papFldDes[71]->offset=(short)((char *)&prec->inii - (char *)prec);
  pdbRecordType->papFldDes[72]->size=sizeof(prec->injj);
  pdbRecordType->papFldDes[72]->offset=(short)((char *)&prec->injj - (char *)prec);
  pdbRecordType->papFldDes[73]->size=sizeof(prec->inkk);
  pdbRecordType->papFldDes[73]->offset=(short)((char *)&prec->inkk - (char *)prec);
  pdbRecordType->papFldDes[74]->size=sizeof(prec->inll);
  pdbRecordType->papFldDes[74]->offset=(short)((char *)&prec->inll - (char *)prec);
  pdbRecordType->papFldDes[75]->size=sizeof(prec->out);
  pdbRecordType->papFldDes[75]->offset=(short)((char *)&prec->out - (char *)prec);
  pdbRecordType->papFldDes[76]->size=sizeof(prec->inav);
  pdbRecordType->papFldDes[76]->offset=(short)((char *)&prec->inav - (char *)prec);
  pdbRecordType->papFldDes[77]->size=sizeof(prec->inbv);
  pdbRecordType->papFldDes[77]->offset=(short)((char *)&prec->inbv - (char *)prec);
  pdbRecordType->papFldDes[78]->size=sizeof(prec->incv);
  pdbRecordType->papFldDes[78]->offset=(short)((char *)&prec->incv - (char *)prec);
  pdbRecordType->papFldDes[79]->size=sizeof(prec->indv);
  pdbRecordType->papFldDes[79]->offset=(short)((char *)&prec->indv - (char *)prec);
  pdbRecordType->papFldDes[80]->size=sizeof(prec->inev);
  pdbRecordType->papFldDes[80]->offset=(short)((char *)&prec->inev - (char *)prec);
  pdbRecordType->papFldDes[81]->size=sizeof(prec->infv);
  pdbRecordType->papFldDes[81]->offset=(short)((char *)&prec->infv - (char *)prec);
  pdbRecordType->papFldDes[82]->size=sizeof(prec->ingv);
  pdbRecordType->papFldDes[82]->offset=(short)((char *)&prec->ingv - (char *)prec);
  pdbRecordType->papFldDes[83]->size=sizeof(prec->inhv);
  pdbRecordType->papFldDes[83]->offset=(short)((char *)&prec->inhv - (char *)prec);
  pdbRecordType->papFldDes[84]->size=sizeof(prec->iniv);
  pdbRecordType->papFldDes[84]->offset=(short)((char *)&prec->iniv - (char *)prec);
  pdbRecordType->papFldDes[85]->size=sizeof(prec->injv);
  pdbRecordType->papFldDes[85]->offset=(short)((char *)&prec->injv - (char *)prec);
  pdbRecordType->papFldDes[86]->size=sizeof(prec->inkv);
  pdbRecordType->papFldDes[86]->offset=(short)((char *)&prec->inkv - (char *)prec);
  pdbRecordType->papFldDes[87]->size=sizeof(prec->inlv);
  pdbRecordType->papFldDes[87]->offset=(short)((char *)&prec->inlv - (char *)prec);
  pdbRecordType->papFldDes[88]->size=sizeof(prec->iaav);
  pdbRecordType->papFldDes[88]->offset=(short)((char *)&prec->iaav - (char *)prec);
  pdbRecordType->papFldDes[89]->size=sizeof(prec->ibbv);
  pdbRecordType->papFldDes[89]->offset=(short)((char *)&prec->ibbv - (char *)prec);
  pdbRecordType->papFldDes[90]->size=sizeof(prec->iccv);
  pdbRecordType->papFldDes[90]->offset=(short)((char *)&prec->iccv - (char *)prec);
  pdbRecordType->papFldDes[91]->size=sizeof(prec->iddv);
  pdbRecordType->papFldDes[91]->offset=(short)((char *)&prec->iddv - (char *)prec);
  pdbRecordType->papFldDes[92]->size=sizeof(prec->ieev);
  pdbRecordType->papFldDes[92]->offset=(short)((char *)&prec->ieev - (char *)prec);
  pdbRecordType->papFldDes[93]->size=sizeof(prec->iffv);
  pdbRecordType->papFldDes[93]->offset=(short)((char *)&prec->iffv - (char *)prec);
  pdbRecordType->papFldDes[94]->size=sizeof(prec->iggv);
  pdbRecordType->papFldDes[94]->offset=(short)((char *)&prec->iggv - (char *)prec);
  pdbRecordType->papFldDes[95]->size=sizeof(prec->ihhv);
  pdbRecordType->papFldDes[95]->offset=(short)((char *)&prec->ihhv - (char *)prec);
  pdbRecordType->papFldDes[96]->size=sizeof(prec->iiiv);
  pdbRecordType->papFldDes[96]->offset=(short)((char *)&prec->iiiv - (char *)prec);
  pdbRecordType->papFldDes[97]->size=sizeof(prec->ijjv);
  pdbRecordType->papFldDes[97]->offset=(short)((char *)&prec->ijjv - (char *)prec);
  pdbRecordType->papFldDes[98]->size=sizeof(prec->ikkv);
  pdbRecordType->papFldDes[98]->offset=(short)((char *)&prec->ikkv - (char *)prec);
  pdbRecordType->papFldDes[99]->size=sizeof(prec->illv);
  pdbRecordType->papFldDes[99]->offset=(short)((char *)&prec->illv - (char *)prec);
  pdbRecordType->papFldDes[100]->size=sizeof(prec->outv);
  pdbRecordType->papFldDes[100]->offset=(short)((char *)&prec->outv - (char *)prec);
  pdbRecordType->papFldDes[101]->size=sizeof(prec->oopt);
  pdbRecordType->papFldDes[101]->offset=(short)((char *)&prec->oopt - (char *)prec);
  pdbRecordType->papFldDes[102]->size=sizeof(prec->odly);
  pdbRecordType->papFldDes[102]->offset=(short)((char *)&prec->odly - (char *)prec);
  pdbRecordType->papFldDes[103]->size=sizeof(prec->wait);
  pdbRecordType->papFldDes[103]->offset=(short)((char *)&prec->wait - (char *)prec);
  pdbRecordType->papFldDes[104]->size=sizeof(prec->dlya);
  pdbRecordType->papFldDes[104]->offset=(short)((char *)&prec->dlya - (char *)prec);
  pdbRecordType->papFldDes[105]->size=sizeof(prec->dopt);
  pdbRecordType->papFldDes[105]->offset=(short)((char *)&prec->dopt - (char *)prec);
  pdbRecordType->papFldDes[106]->size=sizeof(prec->ocal);
  pdbRecordType->papFldDes[106]->offset=(short)((char *)&prec->ocal - (char *)prec);
  pdbRecordType->papFldDes[107]->size=sizeof(prec->oclv);
  pdbRecordType->papFldDes[107]->offset=(short)((char *)&prec->oclv - (char *)prec);
  pdbRecordType->papFldDes[108]->size=sizeof(prec->oevt);
  pdbRecordType->papFldDes[108]->offset=(short)((char *)&prec->oevt - (char *)prec);
  pdbRecordType->papFldDes[109]->size=sizeof(prec->ivoa);
  pdbRecordType->papFldDes[109]->offset=(short)((char *)&prec->ivoa - (char *)prec);
  pdbRecordType->papFldDes[110]->size=sizeof(prec->ivov);
  pdbRecordType->papFldDes[110]->offset=(short)((char *)&prec->ivov - (char *)prec);
  pdbRecordType->papFldDes[111]->size=sizeof(prec->egu);
  pdbRecordType->papFldDes[111]->offset=(short)((char *)&prec->egu - (char *)prec);
  pdbRecordType->papFldDes[112]->size=sizeof(prec->prec);
  pdbRecordType->papFldDes[112]->offset=(short)((char *)&prec->prec - (char *)prec);
  pdbRecordType->papFldDes[113]->size=sizeof(prec->hopr);
  pdbRecordType->papFldDes[113]->offset=(short)((char *)&prec->hopr - (char *)prec);
  pdbRecordType->papFldDes[114]->size=sizeof(prec->lopr);
  pdbRecordType->papFldDes[114]->offset=(short)((char *)&prec->lopr - (char *)prec);
  pdbRecordType->papFldDes[115]->size=sizeof(prec->hihi);
  pdbRecordType->papFldDes[115]->offset=(short)((char *)&prec->hihi - (char *)prec);
  pdbRecordType->papFldDes[116]->size=sizeof(prec->lolo);
  pdbRecordType->papFldDes[116]->offset=(short)((char *)&prec->lolo - (char *)prec);
  pdbRecordType->papFldDes[117]->size=sizeof(prec->high);
  pdbRecordType->papFldDes[117]->offset=(short)((char *)&prec->high - (char *)prec);
  pdbRecordType->papFldDes[118]->size=sizeof(prec->low);
  pdbRecordType->papFldDes[118]->offset=(short)((char *)&prec->low - (char *)prec);
  pdbRecordType->papFldDes[119]->size=sizeof(prec->hhsv);
  pdbRecordType->papFldDes[119]->offset=(short)((char *)&prec->hhsv - (char *)prec);
  pdbRecordType->papFldDes[120]->size=sizeof(prec->llsv);
  pdbRecordType->papFldDes[120]->offset=(short)((char *)&prec->llsv - (char *)prec);
  pdbRecordType->papFldDes[121]->size=sizeof(prec->hsv);
  pdbRecordType->papFldDes[121]->offset=(short)((char *)&prec->hsv - (char *)prec);
  pdbRecordType->papFldDes[122]->size=sizeof(prec->lsv);
  pdbRecordType->papFldDes[122]->offset=(short)((char *)&prec->lsv - (char *)prec);
  pdbRecordType->papFldDes[123]->size=sizeof(prec->hyst);
  pdbRecordType->papFldDes[123]->offset=(short)((char *)&prec->hyst - (char *)prec);
  pdbRecordType->papFldDes[124]->size=sizeof(prec->adel);
  pdbRecordType->papFldDes[124]->offset=(short)((char *)&prec->adel - (char *)prec);
  pdbRecordType->papFldDes[125]->size=sizeof(prec->mdel);
  pdbRecordType->papFldDes[125]->offset=(short)((char *)&prec->mdel - (char *)prec);
  pdbRecordType->papFldDes[126]->size=sizeof(prec->a);
  pdbRecordType->papFldDes[126]->offset=(short)((char *)&prec->a - (char *)prec);
  pdbRecordType->papFldDes[127]->size=sizeof(prec->b);
  pdbRecordType->papFldDes[127]->offset=(short)((char *)&prec->b - (char *)prec);
  pdbRecordType->papFldDes[128]->size=sizeof(prec->c);
  pdbRecordType->papFldDes[128]->offset=(short)((char *)&prec->c - (char *)prec);
  pdbRecordType->papFldDes[129]->size=sizeof(prec->d);
  pdbRecordType->papFldDes[129]->offset=(short)((char *)&prec->d - (char *)prec);
  pdbRecordType->papFldDes[130]->size=sizeof(prec->e);
  pdbRecordType->papFldDes[130]->offset=(short)((char *)&prec->e - (char *)prec);
  pdbRecordType->papFldDes[131]->size=sizeof(prec->f);
  pdbRecordType->papFldDes[131]->offset=(short)((char *)&prec->f - (char *)prec);
  pdbRecordType->papFldDes[132]->size=sizeof(prec->g);
  pdbRecordType->papFldDes[132]->offset=(short)((char *)&prec->g - (char *)prec);
  pdbRecordType->papFldDes[133]->size=sizeof(prec->h);
  pdbRecordType->papFldDes[133]->offset=(short)((char *)&prec->h - (char *)prec);
  pdbRecordType->papFldDes[134]->size=sizeof(prec->i);
  pdbRecordType->papFldDes[134]->offset=(short)((char *)&prec->i - (char *)prec);
  pdbRecordType->papFldDes[135]->size=sizeof(prec->j);
  pdbRecordType->papFldDes[135]->offset=(short)((char *)&prec->j - (char *)prec);
  pdbRecordType->papFldDes[136]->size=sizeof(prec->k);
  pdbRecordType->papFldDes[136]->offset=(short)((char *)&prec->k - (char *)prec);
  pdbRecordType->papFldDes[137]->size=sizeof(prec->l);
  pdbRecordType->papFldDes[137]->offset=(short)((char *)&prec->l - (char *)prec);
  pdbRecordType->papFldDes[138]->size=sizeof(prec->strs);
  pdbRecordType->papFldDes[138]->offset=(short)((char *)&prec->strs - (char *)prec);
  pdbRecordType->papFldDes[139]->size=sizeof(prec->aa);
  pdbRecordType->papFldDes[139]->offset=(short)((char *)&prec->aa - (char *)prec);
  pdbRecordType->papFldDes[140]->size=sizeof(prec->bb);
  pdbRecordType->papFldDes[140]->offset=(short)((char *)&prec->bb - (char *)prec);
  pdbRecordType->papFldDes[141]->size=sizeof(prec->cc);
  pdbRecordType->papFldDes[141]->offset=(short)((char *)&prec->cc - (char *)prec);
  pdbRecordType->papFldDes[142]->size=sizeof(prec->dd);
  pdbRecordType->papFldDes[142]->offset=(short)((char *)&prec->dd - (char *)prec);
  pdbRecordType->papFldDes[143]->size=sizeof(prec->ee);
  pdbRecordType->papFldDes[143]->offset=(short)((char *)&prec->ee - (char *)prec);
  pdbRecordType->papFldDes[144]->size=sizeof(prec->ff);
  pdbRecordType->papFldDes[144]->offset=(short)((char *)&prec->ff - (char *)prec);
  pdbRecordType->papFldDes[145]->size=sizeof(prec->gg);
  pdbRecordType->papFldDes[145]->offset=(short)((char *)&prec->gg - (char *)prec);
  pdbRecordType->papFldDes[146]->size=sizeof(prec->hh);
  pdbRecordType->papFldDes[146]->offset=(short)((char *)&prec->hh - (char *)prec);
  pdbRecordType->papFldDes[147]->size=sizeof(prec->ii);
  pdbRecordType->papFldDes[147]->offset=(short)((char *)&prec->ii - (char *)prec);
  pdbRecordType->papFldDes[148]->size=sizeof(prec->jj);
  pdbRecordType->papFldDes[148]->offset=(short)((char *)&prec->jj - (char *)prec);
  pdbRecordType->papFldDes[149]->size=sizeof(prec->kk);
  pdbRecordType->papFldDes[149]->offset=(short)((char *)&prec->kk - (char *)prec);
  pdbRecordType->papFldDes[150]->size=sizeof(prec->ll);
  pdbRecordType->papFldDes[150]->offset=(short)((char *)&prec->ll - (char *)prec);
  pdbRecordType->papFldDes[151]->size=sizeof(prec->paa);
  pdbRecordType->papFldDes[151]->offset=(short)((char *)&prec->paa - (char *)prec);
  pdbRecordType->papFldDes[152]->size=sizeof(prec->pbb);
  pdbRecordType->papFldDes[152]->offset=(short)((char *)&prec->pbb - (char *)prec);
  pdbRecordType->papFldDes[153]->size=sizeof(prec->pcc);
  pdbRecordType->papFldDes[153]->offset=(short)((char *)&prec->pcc - (char *)prec);
  pdbRecordType->papFldDes[154]->size=sizeof(prec->pdd);
  pdbRecordType->papFldDes[154]->offset=(short)((char *)&prec->pdd - (char *)prec);
  pdbRecordType->papFldDes[155]->size=sizeof(prec->pee);
  pdbRecordType->papFldDes[155]->offset=(short)((char *)&prec->pee - (char *)prec);
  pdbRecordType->papFldDes[156]->size=sizeof(prec->pff);
  pdbRecordType->papFldDes[156]->offset=(short)((char *)&prec->pff - (char *)prec);
  pdbRecordType->papFldDes[157]->size=sizeof(prec->pgg);
  pdbRecordType->papFldDes[157]->offset=(short)((char *)&prec->pgg - (char *)prec);
  pdbRecordType->papFldDes[158]->size=sizeof(prec->phh);
  pdbRecordType->papFldDes[158]->offset=(short)((char *)&prec->phh - (char *)prec);
  pdbRecordType->papFldDes[159]->size=sizeof(prec->pii);
  pdbRecordType->papFldDes[159]->offset=(short)((char *)&prec->pii - (char *)prec);
  pdbRecordType->papFldDes[160]->size=sizeof(prec->pjj);
  pdbRecordType->papFldDes[160]->offset=(short)((char *)&prec->pjj - (char *)prec);
  pdbRecordType->papFldDes[161]->size=sizeof(prec->pkk);
  pdbRecordType->papFldDes[161]->offset=(short)((char *)&prec->pkk - (char *)prec);
  pdbRecordType->papFldDes[162]->size=sizeof(prec->pll);
  pdbRecordType->papFldDes[162]->offset=(short)((char *)&prec->pll - (char *)prec);
  pdbRecordType->papFldDes[163]->size=sizeof(prec->oval);
  pdbRecordType->papFldDes[163]->offset=(short)((char *)&prec->oval - (char *)prec);
  pdbRecordType->papFldDes[164]->size=sizeof(prec->osv);
  pdbRecordType->papFldDes[164]->offset=(short)((char *)&prec->osv - (char *)prec);
  pdbRecordType->papFldDes[165]->size=sizeof(prec->posv);
  pdbRecordType->papFldDes[165]->offset=(short)((char *)&prec->posv - (char *)prec);
  pdbRecordType->papFldDes[166]->size=sizeof(prec->pa);
  pdbRecordType->papFldDes[166]->offset=(short)((char *)&prec->pa - (char *)prec);
  pdbRecordType->papFldDes[167]->size=sizeof(prec->pb);
  pdbRecordType->papFldDes[167]->offset=(short)((char *)&prec->pb - (char *)prec);
  pdbRecordType->papFldDes[168]->size=sizeof(prec->pc);
  pdbRecordType->papFldDes[168]->offset=(short)((char *)&prec->pc - (char *)prec);
  pdbRecordType->papFldDes[169]->size=sizeof(prec->pd);
  pdbRecordType->papFldDes[169]->offset=(short)((char *)&prec->pd - (char *)prec);
  pdbRecordType->papFldDes[170]->size=sizeof(prec->pe);
  pdbRecordType->papFldDes[170]->offset=(short)((char *)&prec->pe - (char *)prec);
  pdbRecordType->papFldDes[171]->size=sizeof(prec->pf);
  pdbRecordType->papFldDes[171]->offset=(short)((char *)&prec->pf - (char *)prec);
  pdbRecordType->papFldDes[172]->size=sizeof(prec->pg);
  pdbRecordType->papFldDes[172]->offset=(short)((char *)&prec->pg - (char *)prec);
  pdbRecordType->papFldDes[173]->size=sizeof(prec->ph);
  pdbRecordType->papFldDes[173]->offset=(short)((char *)&prec->ph - (char *)prec);
  pdbRecordType->papFldDes[174]->size=sizeof(prec->pi);
  pdbRecordType->papFldDes[174]->offset=(short)((char *)&prec->pi - (char *)prec);
  pdbRecordType->papFldDes[175]->size=sizeof(prec->pj);
  pdbRecordType->papFldDes[175]->offset=(short)((char *)&prec->pj - (char *)prec);
  pdbRecordType->papFldDes[176]->size=sizeof(prec->pk);
  pdbRecordType->papFldDes[176]->offset=(short)((char *)&prec->pk - (char *)prec);
  pdbRecordType->papFldDes[177]->size=sizeof(prec->pl);
  pdbRecordType->papFldDes[177]->offset=(short)((char *)&prec->pl - (char *)prec);
  pdbRecordType->papFldDes[178]->size=sizeof(prec->povl);
  pdbRecordType->papFldDes[178]->offset=(short)((char *)&prec->povl - (char *)prec);
  pdbRecordType->papFldDes[179]->size=sizeof(prec->lalm);
  pdbRecordType->papFldDes[179]->offset=(short)((char *)&prec->lalm - (char *)prec);
  pdbRecordType->papFldDes[180]->size=sizeof(prec->alst);
  pdbRecordType->papFldDes[180]->offset=(short)((char *)&prec->alst - (char *)prec);
  pdbRecordType->papFldDes[181]->size=sizeof(prec->mlst);
  pdbRecordType->papFldDes[181]->offset=(short)((char *)&prec->mlst - (char *)prec);
  pdbRecordType->papFldDes[182]->size=sizeof(prec->rpcl);
  pdbRecordType->papFldDes[182]->offset=(short)((char *)&prec->rpcl - (char *)prec);
  pdbRecordType->papFldDes[183]->size=sizeof(prec->orpc);
  pdbRecordType->papFldDes[183]->offset=(short)((char *)&prec->orpc - (char *)prec);
    pdbRecordType->rec_size = sizeof(*prec);
    return(0);
}
epicsExportRegistrar(scalcoutRecordSizeOffset);
#ifdef __cplusplus
}
#endif
#endif /*GEN_SIZE_OFFSET*/

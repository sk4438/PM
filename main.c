#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include "PDSUs.h"
#include "PDSU.h"
#include "MeasValue.h"
#include "SubCounterIndexType.h"
#include "SubCounterListType.h"
#include <time.h>
#include <assert.h>
#include <stdbool.h>



const long M = 1000000;
const long K = 1000;

static int write_out(const void *buffer, size_t size, void *app_key) {
FILE *out_fp = app_key;
size_t wrote = fwrite(buffer, 1, size, out_fp);
return (wrote == size) ? 0 : -1;
}

typedef struct linkedlist {
    int data;
    struct linkedlist *next;
} list;


int compare(char *buf1,char *buf2,size_t size)
{
    for(int i =0;i<size;i++)
    {
        if(buf1[i]==buf2[i])
        {
            continue;
        }else
        {
            return 0;
        }
    }
    return 1;
}

void CUUP(PDSUs_t *pdsus_CUUP,int *cc_qos_DL,int *cc_snssai_DL,int *cc_plmns_DL,int noOfplmns,int noOfCUUPeachplmn,int noOfSnssaieachDU,int noOfDUseachplmn,int QoS,int c,OCTET_STRING_t *plmnids,struct sNSSAI *snssaiids)
{
   int sid = 1;
   int noOfStreams = 1;
   while(sid<=noOfStreams)
   {  
       int *streamid;
       streamid= calloc(noOfCUUPeachplmn*noOfplmns,sizeof(int));
       for(int i =0;i<noOfCUUPeachplmn*noOfplmns;i++)
       {
           streamid[i] |= 2<<20;
           streamid[i] |= (i+1)<<12;
           streamid[i] |= c<<8;
       }
       int ret;
       int id = 0;

       while(id<noOfCUUPeachplmn*noOfplmns)
       {
           int S = 0;
           int k = id/noOfCUUPeachplmn;
           int noOfsnssai = noOfSnssaieachDU*noOfDUseachplmn;
           for(int i = id*noOfsnssai;i<(id+1)*noOfsnssai;i++)
           {
               int s = 0;
               for(int j = 0;j<QoS;j++)
               {
                   PDSU_t *pdsu_dl;
                   pdsu_dl = calloc(1,sizeof(PDSU_t));
                   pdsu_dl->streamId = streamid[id];
                   MeasValue_t *measvalue;
                   measvalue = calloc(1,sizeof(MeasValue_t));
                   measvalue->present = 3;
                   SubCounterListType_t *plmn;
                   plmn = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *plmnid;
                   plmnid = calloc(1,sizeof(SubCounterIndexType_t));
                   plmnid->present = 7;
                   plmnid->choice.plMN = plmnids[k];
                   plmn->subCounterIndex = *plmnid;
                   MeasValue_t *measvalue1;
                   measvalue1 = calloc(1,sizeof(MeasValue_t));
                   measvalue1->present = 3;
                   SubCounterListType_t *snssai;
                   snssai = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *snssaiid;
                   snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
                   snssaiid->present = 8;
                   snssaiid->choice.sNSSAI.sd = snssaiids[k*noOfsnssai + i - id*noOfsnssai].sd;
                   snssaiid->choice.sNSSAI.sst = snssaiids[k*noOfsnssai + i - id*noOfsnssai].sst;
                   snssai->subCounterIndex = *snssaiid;
                   MeasValue_t *measvalue2;
                   measvalue2 = calloc(1,sizeof(MeasValue_t));
                   measvalue2->present = 3;
                   SubCounterListType_t *qos;
                   qos = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *qoslevel;
                   qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
                   qoslevel->present = 3;
                   qoslevel->choice.qOS_5QI = j + 1;
                   qos->subCounterIndex = *qoslevel;
                   int t = rand()/M;
                   s += t;
                   MeasValue_t *measvalue3;
                   measvalue3 = calloc(1,sizeof(MeasValue_t));
                   measvalue3->present = 1;
                   measvalue3->choice.integerValue = cc_qos_DL[i*QoS + j] + t;
                   cc_qos_DL[i*QoS + j] += t;
                   qos->subCounterValue = measvalue3;
                   measvalue2->choice.subCounters = qos;
                   snssai->subCounterValue = measvalue2;
                   measvalue1->choice.subCounters = snssai;
                   plmn->subCounterValue = measvalue1;
                   measvalue->choice.subCounters = plmn;
                   ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
                   assert(ret==0);
                   //choice
                    PDSU_t *pdsu1;
                    pdsu1 = calloc(1,sizeof(PDSU_t));
                    pdsu1->streamId = streamid[id];
                    MeasValue_t *meas;
                    meas = calloc(1,sizeof(MeasValue_t));
                    meas->present = 3;
                    SubCounterListType_t *snssailst;
                    snssailst = calloc(1,sizeof(SubCounterListType_t));
                    snssailst->subCounterIndex = *snssaiid;
                    snssailst->subCounterValue = measvalue2;
                    meas->choice.subCounters = snssailst;
                   ret = ASN_SEQUENCE_ADD(&pdsu1->standardizedMeasResults,meas);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu1);
                   assert(ret==0);
               }
               PDSU_t *pdsu_dl;
               pdsu_dl = calloc(1,sizeof(PDSU_t));
               pdsu_dl->streamId = streamid[id];
               MeasValue_t *measvalue;
               measvalue = calloc(1,sizeof(MeasValue_t));
               measvalue->present = 3;
               SubCounterListType_t *plmn;
               plmn = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *plmnid;
               plmnid = calloc(1,sizeof(SubCounterIndexType_t));
               plmnid->present = 7;
               plmnid->choice.plMN = plmnids[k];
               plmn->subCounterIndex = *plmnid;
               MeasValue_t *measvalue1;
               measvalue1 = calloc(1,sizeof(MeasValue_t));
               measvalue1->present = 3;
               SubCounterListType_t *snssai;
               snssai = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *snssaiid;
               snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
               snssaiid->present = 8;
               snssaiid->choice.sNSSAI.sd = snssaiids[k*noOfsnssai + i - id*noOfsnssai].sd;
               snssaiid->choice.sNSSAI.sst = snssaiids[k*noOfsnssai + i - id*noOfsnssai].sst;
               snssai->subCounterIndex = *snssaiid;
               MeasValue_t *measvalue2;
               measvalue2 = calloc(1,sizeof(MeasValue_t));
               measvalue2->present = 1;
               measvalue2->choice.integerValue = cc_snssai_DL[i] + s;
               cc_snssai_DL[i] += s;
               S += s;
               snssai->subCounterValue = measvalue2;
               measvalue1->choice.subCounters = snssai;
               plmn->subCounterValue = measvalue1;
               measvalue->choice.subCounters = plmn;
               ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
               assert(ret==0);
               ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
               assert(ret==0);
                //choice
                PDSU_t *pdsu;
                pdsu = calloc(1,sizeof(PDSU_t));
                pdsu->streamId = streamid[id];
                MeasValue_t *measval;
                measval = calloc(1,sizeof(MeasValue_t));
                measval->present = 3;
                SubCounterListType_t *snssailt;
                snssailt = calloc(1,sizeof(SubCounterListType_t));
                snssailt->subCounterIndex = *snssaiid;
                MeasValue_t *measval1;
                measval1 = calloc(1,sizeof(MeasValue_t));
                measval1->present = 1;
                measval1->choice.integerValue = cc_snssai_DL[i];
                snssailt->subCounterValue = measval1;
                measval->choice.subCounters = snssailt;
                ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
                assert(ret==0);
                ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
                assert(ret==0);

           }
           PDSU_t *pdsu_dl;
           pdsu_dl = calloc(1,sizeof(PDSU_t));
           pdsu_dl->streamId = streamid[id];
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[k];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = cc_plmns_DL[id] + S;
           cc_plmns_DL[id] += S;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
           assert(ret==0);
           ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
           assert(ret==0);
           //choice
           for(int i = 0;i<QoS;i++)
           {
              int s = 0;
              for(int j = 0;j<noOfsnssai;j++)
              {
                  s += cc_qos_DL[id*QoS*noOfsnssai + j*QoS + i];
              }
              PDSU_t *pdsu;
              pdsu = calloc(1,sizeof(PDSU_t));
              pdsu->streamId = streamid[id];
              MeasValue_t *measval;
              measval = calloc(1,sizeof(MeasValue_t));
              measval->present = 3;
              SubCounterListType_t *plmnlt;
              plmnlt = calloc(1,sizeof(SubCounterListType_t));
              plmnlt->subCounterIndex = *plmnid;
              MeasValue_t *measval1;
              measval1 = calloc(1,sizeof(MeasValue_t));
              measval1->present = 3;
              SubCounterListType_t *qoslt;
              qoslt = calloc(1,sizeof(SubCounterListType_t));
              SubCounterIndexType_t *qoslevel;
              qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
              qoslevel->present = 3;
              qoslevel->choice.qOS_5QI = i + 1;
              qoslt->subCounterIndex = *qoslevel;
              MeasValue_t *measval2;
              measval2 = calloc(1,sizeof(MeasValue_t));
              measval2->present = 1;
              measval2->choice.integerValue = s;
              qoslt->subCounterValue = measval2;
              measval1->choice.subCounters = qoslt;
              plmnlt->subCounterValue = measval1;
              measval->choice.subCounters = plmnlt;
              ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
              assert(ret==0);
              ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
              assert(ret==0); 
           }
           for(int i = 0;i<QoS;i++)
           {
              int s = 0;
              for(int j = 0;j<noOfsnssai;j++)
              {
                  s += cc_qos_DL[id*QoS*noOfsnssai + j*QoS + i];
              }
              PDSU_t *pdsu;
              pdsu = calloc(1,sizeof(PDSU_t));
              pdsu->streamId = streamid[id];
              MeasValue_t *measval;
              measval = calloc(1,sizeof(MeasValue_t));
              measval->present = 3;
              SubCounterListType_t *qoslt;
              qoslt = calloc(1,sizeof(SubCounterListType_t));
              SubCounterIndexType_t *qoslevel;
              qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
              qoslevel->present = 3;
              qoslevel->choice.qOS_5QI = i + 1;
              qoslt->subCounterIndex = *qoslevel;
              MeasValue_t *measval2;
              measval2 = calloc(1,sizeof(MeasValue_t));
              measval2->present = 1;
              measval2->choice.integerValue = s;
              qoslt->subCounterValue = measval2;
              measval->choice.subCounters = qoslt;
              ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
              assert(ret==0);
              ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
              assert(ret==0); 
           }
           id++;
       }
       sid++;
    }
}

//PDSUs_t* CUUP_PM(int noOfStreams,int noOfnrcells,int *cc_plmn,int *cc_snssai)

PDSU_t* DU_Prb(int streamid,int f)
{
    PDSU_t *pdsu;
    pdsu = calloc(1,sizeof(PDSU_t));
    pdsu->streamId = streamid;
    int ret;
    MeasValue_t *measvalue;
    measvalue = calloc(1,sizeof(MeasValue_t));
    measvalue->present = 1;
    measvalue->choice.integerValue = f?rand()%101:rand();
    ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
    assert(ret==0);
    return pdsu;
}


/*
int write(const void *buf,size_t size,void *fp)
{
    FILE *out = fp;
    return (fwrite(buf,1,size,out)==size)?size:-1;
}
*/
int* config()
{
    FILE *fp = fopen("val.config","r");
    if(!fp)
    {
        perror("val.config");
        exit(1);
    }
    int *val;
    val = calloc(8,sizeof(int));
    char buf[1024];
    int i =0;
    while(fscanf(fp,"%s %d[^\n]",buf,val+i)!=EOF)
    {
        i++;
    }
    return val;
}

long* data(int noOfplmns,int noOfDuseachplmn,int noOfsnssaieachDU,int Qos,int c)
{
    long *qos;
    qos = calloc(noOfplmns*noOfDuseachplmn*noOfsnssaieachDU*Qos,sizeof(long));
    for(int i = 0;i<noOfDuseachplmn*noOfplmns;i++)
    {
        for(int j = 0;j<noOfsnssaieachDU;j++)
        {
            for(int k = 0;k<Qos;k++)
            {
                qos[(i*noOfsnssaieachDU + j)*Qos+k] = c>6?rand()/K:rand();
            }
        }
    }
    return qos;
}

void DU_Prb2(PDSUs_t *pdsus_DU,int noOfplmns,int noOfDuseachplmn,int noOfSnssaieachDu,int QoS,int c,OCTET_STRING_t *plmnids,struct sNSSAI *snssaiids)
{
   int noOfDus = noOfDuseachplmn*noOfplmns;
   int ret;
   long *c_snssai,*c_Du;
   c_snssai = calloc(noOfDus*noOfSnssaieachDu,sizeof(long));
   c_Du = calloc(noOfDus,sizeof(long));
   long *qos;
   qos = data(noOfplmns,noOfDuseachplmn,noOfSnssaieachDu,QoS,c);
   for(int i =0;i<noOfplmns*noOfDuseachplmn*noOfSnssaieachDu*QoS;i++)
    {
        fprintf(stderr,"%ld\n",qos[i]);
    }
   int *streamid;
   streamid = calloc(noOfDus,sizeof(int));
   for(int i =0;i<noOfDus;i++)
   {
       streamid[i] |= 3<<20;
       streamid[i] |= (i+1)<<12;
       streamid[i] |= c<<8;
   }
   int id = 0;
   while(id<noOfDus)
   {
      long *c_snssai;
      c_snssai = calloc(noOfSnssaieachDu,sizeof(long));
      for(int i = 0;i<QoS;i++)
      {
        long s = 0;
        for(int j =0;j<noOfSnssaieachDu;j++)
        {
            s += qos[(id*noOfSnssaieachDu + j)*QoS + i];
        }
        PDSU_t *pdsu;
        pdsu = calloc(1,sizeof(PDSU_t));
        pdsu->streamId = streamid[id];
        MeasValue_t *measvalue;
        measvalue = calloc(1,sizeof(MeasValue_t));
        measvalue->present = 3;
        SubCounterIndexType_t *qosid;
        qosid = calloc(1,sizeof(SubCounterIndexType_t));
        qosid->present = 3;
        qosid->choice.qOS_5QI = i+1;
        SubCounterListType_t *Qos;
        Qos = calloc(1,sizeof(SubCounterListType_t));
        Qos->subCounterIndex = *qosid;
        MeasValue_t *measvalue1;
        measvalue1 = calloc(1,sizeof(MeasValue_t));
        measvalue1->present = 1;
        measvalue1->choice.integerValue = s/(noOfSnssaieachDu);
        Qos->subCounterValue = measvalue1;
        measvalue->choice.subCounters = Qos;
        ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
        assert(ret==0);
        ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
        assert(ret==0);
      }
      for(int i = 0;i<noOfSnssaieachDu;i++)
      {
          long s = 0;
          for(int j = 0;j<QoS;j++)
          {
              s += qos[(id*noOfSnssaieachDu + i)*QoS + j];
          }
          PDSU_t *pdsu;
          pdsu = calloc(1,sizeof(PDSU_t));
          pdsu->streamId = streamid[id];
          MeasValue_t *measvalue;
          measvalue = calloc(1,sizeof(MeasValue_t));
          measvalue->present = 3;
          SubCounterIndexType_t *snssaiid;
          snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
          snssaiid->present = 8;
          snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
          snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
          SubCounterListType_t *snssai;
          snssai = calloc(1,sizeof(SubCounterListType_t));
          snssai->subCounterIndex = *snssaiid;
          MeasValue_t *measvalue1;
          measvalue1 = calloc(1,sizeof(MeasValue_t));
          measvalue1->present = 1;
          measvalue1->choice.integerValue = s/QoS;
          c_snssai[i] += s;
          snssai->subCounterValue = measvalue1;
          measvalue->choice.subCounters = snssai;
          ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
          assert(ret==0);
          ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
          assert(ret==0); 
      }
      PDSU_t *pdsu;
      pdsu = calloc(1,sizeof(int));
      pdsu->streamId = streamid[id];
      long s = 0;
      for(int j =0;j<noOfSnssaieachDu;j++)
       {
           s += c_snssai[j];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 1;
       measvalue->choice.integerValue = s/(noOfSnssaieachDu*QoS);
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0);
       //plmn
       PDSU_t *pdsu1;
       pdsu1 = calloc(1,sizeof(int));
       pdsu1->streamId = streamid[id];
       MeasValue_t *measvalue1;
       measvalue1 = calloc(1,sizeof(MeasValue_t));
       measvalue1->present = 3;
       SubCounterIndexType_t *plmnid;
       plmnid = calloc(1,sizeof(SubCounterIndexType_t));
       plmnid->present = 7;
       plmnid->choice.plMN = plmnids[id/noOfDuseachplmn];
       SubCounterListType_t *plmn;
       plmn = calloc(1,sizeof(SubCounterListType_t));
       plmn->subCounterIndex = *plmnid;
       MeasValue_t *measvalue2;
       measvalue2 = calloc(1,sizeof(MeasValue_t));
       measvalue2->present = 1;
       measvalue2->choice.integerValue = s/(noOfSnssaieachDu*QoS);
       plmn->subCounterValue = measvalue2;
       measvalue1->choice.subCounters = plmn;
       ret = ASN_SEQUENCE_ADD(&pdsu1->standardizedMeasResults,measvalue1);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu1);
       assert(ret==0);
       id++;
   }
   /*
   for(int i = 0;i<noOfSnssaieachDu*noOfDus;i++)
   {
       int id = i/noOfSnssaieachDu;
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(PDSU_t));
       pdsu->streamId = streamid[id];
       long s = 0;
       for(int j =0;j<QoS;j++)
       {
           s += qos[i*QoS + j];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 3;
       SubCounterIndexType_t *snssaiid;
       snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
       snssaiid->present = 8;
       snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
       snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
       SubCounterListType_t *snssai;
       snssai = calloc(1,sizeof(SubCounterListType_t));
       snssai->subCounterIndex = *snssaiid;
       MeasValue_t *measvalue1;
       measvalue1 = calloc(1,sizeof(MeasValue_t));
       measvalue1->present = 1;
       measvalue1->choice.integerValue = s/QoS;
       c_snssai[i] += s;
       snssai->subCounterValue = measvalue1;
       measvalue->choice.subCounters = snssai;
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0); 
   }
   for(int i =0;i<noOfDus;i++)
   {
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(int));
       pdsu->streamId = streamid[i];
       long s = 0;
       for(int j =0;j<noOfSnssaieachDu;j++)
       {
           s += c_snssai[i*noOfSnssaieachDu + j];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 1;
       measvalue->choice.integerValue = s/noOfSnssaieachDu;
       c_Du[i] = s;
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0);
   }
   for(int i=0;i<QoS;i++)
   {
       int streamid = 0;
       streamid |= 3<<20;
       streamid |= c<<8;
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(int));
       pdsu->streamId = streamid;
       long s = 0;
       for(int j =0;j<noOfSnssaieachDu*noOfDus;j++)
       {
           s += qos[j*noOfSnssaieachDu*noOfDus + i];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 3;
       SubCounterIndexType_t *qosid;
       qosid = calloc(1,sizeof(SubCounterIndexType_t));
       qosid->present = 4;
       qosid->choice.qOS_5QI = i+1;
       SubCounterListType_t *Qos;
       Qos = calloc(1,sizeof(SubCounterListType_t));
       Qos->subCounterIndex = *qosid;
       MeasValue_t *measvalue1;
       measvalue1 = calloc(1,sizeof(MeasValue_t));
       measvalue1->present = 1;
       measvalue1->choice.integerValue = s/(noOfSnssaieachDu*noOfDus);
       Qos->subCounterValue = measvalue1;
       measvalue->choice.subCounters = Qos;
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0); 
   }
   for(int i =0;i<noOfplmns;i++)
   {
       int streamid = 0;
       streamid |= 3<<20;
       streamid |= c<<8;
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(int));
       pdsu->streamId = streamid;
       long s = 0;
       for(int j =0;j<noOfDuseachplmn;j++)
       {
           s += c_Du[i*noOfDuseachplmn + j];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 3;
       SubCounterIndexType_t *plmnid;
       plmnid = calloc(1,sizeof(SubCounterIndexType_t));
       plmnid->present = 7;
       plmnid->choice.plMN = plmnids[i];
       SubCounterListType_t *plmn;
       plmn = calloc(1,sizeof(SubCounterListType_t));
       plmn->subCounterIndex = *plmnid;
       MeasValue_t *measvalue1;
       measvalue1 = calloc(1,sizeof(MeasValue_t));
       measvalue1->present = 1;
       measvalue1->choice.integerValue = s/noOfDuseachplmn;
       plmn->subCounterValue = measvalue1;
       measvalue->choice.subCounters = plmn;
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0);   
   }
   */

}

int main(int argc,char *argv[])
{
    srand(time(0));
    int noOfNrcells,QoS,noOfDuseachplmn,noOfCUCPeachplmn;
    int noOfCUUPeachplmn,noOfSnssaieachDU,noOfNrcellseachCUCP,noOfNrcellseachCUUP;
    int noOfStreams = 1;
    int *val = config();
    for(int i =0;i<8;i++)
    {
        switch(i)
        {
            case 0:
                noOfNrcells = val[i];
                break;
            case 1:
                noOfCUCPeachplmn = val[i];
                break;
            case 2:
                noOfCUUPeachplmn = val[i];
                break;
            case 3:
                noOfDuseachplmn = val[i];
                break;
            case 4:
                noOfNrcellseachCUCP = val[i];
                break;
            case 5:
                noOfNrcellseachCUUP = val[i];
                break;
            case 6:
                noOfSnssaieachDU = val[i];
                break;
            case 7:
                QoS = val[i];
                break;
        }
    }
    int sid = 1;
    int noOfplmns = noOfNrcells/(noOfCUCPeachplmn*noOfNrcellseachCUCP);
    //fprintf(stderr,"%d\n", noOfplmns);
    //char str[noOfplmns][25];
    OCTET_STRING_t *plmnids;
    plmnids = calloc(noOfplmns,sizeof(OCTET_STRING_t));
    for(int i =0;i<noOfplmns;i++)
    {
        uint8_t *buf;
        buf = calloc(3,sizeof(uint8_t));
        //int r = 0;
        for(int j = 0;j<3;j++)
        {
            int a = rand()%10,b = rand()%10;
            /*
            r |= (a)<<(2*j*4);
            r |= (b)<<((2*j+1)*4);
            */
            buf[j] |= (a);
            buf[j] |= (b)<<4;
        }
        //printf("%s\n", (char*)buf);
        //int r = rand()%(1000000);
        //fprintf(stderr,"%d\n",r);
        /*
        sprintf(str[i],"%d",r);
        */
        
        int ret = OCTET_STRING_fromString(plmnids+i,(char *)buf);
        //fprintf(stderr,"%d\n",plmnids[i].size);
        //fprintf(stderr,"%d  %s\n", plmnids[i].size,str[i]);
    }
    /*
    list* complmn;
    complmn = calloc(noOfNrcells,sizeof(list));
    for(int i= 0;i<noOfNrcells;i++)
    {
        if(complmn[i].data==-1)
        {
            continue;
        }
        complmn[i].data = i;
        list* head = complmn + i;
        for(int j =i+1;j<noOfNrcells;j++)
        {
            if(plmnids[i].size==plmnids[j].size&&compare(plmnids[i].buf,plmnids[j].buf,plmnids[i].size))
            {
                list* temp = calloc(1,sizeof(list));
                temp->data = j;
                head->next = temp;
                head = temp;
                complmn[j].data = -1;
                complmn[j].next =NULL;
            }
        }
        head->next = NULL;
    }
    */
    struct sNSSAI *snssaiids;
    snssaiids = calloc(noOfSnssaieachDU*noOfDuseachplmn*noOfplmns,sizeof(struct sNSSAI));
    for(int i =0;i<noOfSnssaieachDU*noOfDuseachplmn*noOfplmns;i++)
    {
        /*
        if(complmn[i].data==-1)
        {
            continue;
        }
        */
        uint8_t *bufsst;
        bufsst = calloc(1,1);
        bufsst[0] = rand()%256;
        uint8_t *bufsd;
        bufsd = calloc(3,1);
        for(int j = 0;j<3;j++)
        {
            int a = rand()%256,b = rand()%256;
            /*
            r |= (a)<<(2*j*4);
            r |= (b)<<((2*j+1)*4);
            */
            bufsd[j] |= a;
        }
        snssaiids[i].sd.buf = bufsd;
        snssaiids[i].sd.size = 3;
        snssaiids[i].sst.buf = bufsst;
        snssaiids[i].sst.size = 1;
    }
    int noOfSnssaieachplmn = noOfSnssaieachDU*noOfDuseachplmn;
    int *cc_plmns_DL,*cc_plmns_UL;
    cc_plmns_DL = calloc(noOfplmns*noOfCUUPeachplmn,sizeof(int));
    cc_plmns_UL = calloc(noOfplmns*noOfCUUPeachplmn,sizeof(int));
    int *cc_snssai_DL,*cc_snssai_UL;
    cc_snssai_DL = calloc(noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(int));
    cc_snssai_UL = calloc(noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(int));
    int *cc_qos_DL,*cc_qos_UL;
    cc_qos_DL = calloc(QoS*noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(int));
    cc_qos_UL = calloc(QoS*noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(int));
    PDSUs_t *pdsus_CUCP;
    pdsus_CUCP = calloc(1,sizeof(PDSUs_t));
    while(sid<=noOfStreams)
    {
        //CU-CP
        int ret;
        int vnrcells[noOfNrcells];
        int mnrcells[noOfNrcells];
        int *streamid_nrcells_mean,*streamid_nrcells_max;
        int *streamid_plmns_mean,*streamid_plmns_max;
        streamid_nrcells_mean = calloc(noOfNrcells,sizeof(int));
        streamid_plmns_mean = calloc(noOfNrcells,sizeof(int));
        streamid_nrcells_max = calloc(noOfNrcells,sizeof(int));
        streamid_plmns_max = calloc(noOfNrcells,sizeof(int));
        for(int j =0;j<noOfCUCPeachplmn*noOfplmns;j++)
        {
            for(int k =0;k<noOfNrcellseachCUCP;k++)
            {
                int index = j*noOfNrcellseachCUCP + k;
                streamid_nrcells_mean[index] |= (1<<20);
                streamid_nrcells_mean[index] |= ((j+1)<<12); 
                streamid_nrcells_mean[index] |= (1<<8);
                streamid_nrcells_max[index] |= (1<<20);
                streamid_nrcells_max[index] |= ((j+1)<<12); 
                streamid_nrcells_max[index] |= (2<<8);
                streamid_plmns_mean[index] = streamid_nrcells_mean[index];
                streamid_plmns_max[index] = streamid_nrcells_max[index];
                streamid_nrcells_mean[index] |= index + 1;
                streamid_nrcells_max[index] |= index + 1;
            }
        }
        for(int i =0;i<noOfNrcells;i++)
        {
            PDSU_t *pdsu_mean;
            pdsu_mean = calloc(1,sizeof(PDSU_t));
            pdsu_mean->streamId = streamid_nrcells_mean[i];
            PDSU_t *pdsu_max;
            pdsu_max = calloc(1,sizeof(PDSU_t));
            pdsu_max->streamId = streamid_nrcells_max[i];
            MeasValue_t *measvalue_mean;
            measvalue_mean = calloc(1,sizeof(MeasValue_t));
            measvalue_mean->present = 1;
            measvalue_mean->choice.integerValue = rand()%M;
            vnrcells[i] = measvalue_mean->choice.integerValue;
            MeasValue_t *measvalue_max;
            measvalue_max = calloc(1,sizeof(MeasValue_t));
            measvalue_max->present = 1;
            measvalue_max->choice.integerValue = vnrcells[i] + rand()%M;
            mnrcells[i] = measvalue_max->choice.integerValue;
            //xer_fprint(stdout,&asn_DEF_MeasValue,measvalue);
            ret = ASN_SEQUENCE_ADD(&pdsu_mean->standardizedMeasResults,measvalue_mean);
            assert(ret==0);
            ret = ASN_SEQUENCE_ADD(&pdsu_max->standardizedMeasResults, measvalue_max);
            assert(ret==0);
            ret = ASN_SEQUENCE_ADD(pdsus_CUCP,pdsu_mean);
            assert(ret==0);
            ret = ASN_SEQUENCE_ADD(pdsus_CUCP,pdsu_max);
            assert(ret==0);
        }
        for(int i =0;i<noOfplmns;i++)
        {
           /*
           if(complmn[i].data==-1)
           {
               continue;
           }
           int s  = 0,n = 0;
           list *temp = complmn + i;
           while(1)
           {
               if(temp==NULL)
               {
                   break;
               }
               s = s + vnrcells[temp->data];
               temp = temp->next;
               n++;
           }
           */
           int n = noOfCUCPeachplmn*noOfNrcellseachCUCP;
           int s = 0;
           for(int j = i*n;j<(i+1)*n;j++)
           {
               s += vnrcells[j];
           }
           PDSU_t *pdsu_mean;
           pdsu_mean = calloc(1,sizeof(PDSU_t));
           pdsu_mean->streamId = streamid_plmns_mean[i];
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[i];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = s/n;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu_mean->standardizedMeasResults,measvalue);
           assert(ret==0);
           ret = ASN_SEQUENCE_ADD(pdsus_CUCP,pdsu_mean);
           assert(ret==0);
        }
        for(int i =0;i<noOfplmns;i++)
        {
            /*
           if(complmn[i].data==-1)
           {
               continue;
           }
           int s  = INT_MIN;
           list *temp = complmn + i;
           while(1)
           {
               if(temp==NULL)
               {
                   break;
               }
               s = s>mnrcells[temp->data]?s:mnrcells[temp->data];
               temp = temp->next;
           }
           */
           int n = noOfCUCPeachplmn*noOfNrcellseachCUCP;
           int s = INT_MIN;
           for(int j = i*n;j<(i+1)*n;j++)
           {
               s  = s>mnrcells[j]?s:mnrcells[j];
           }
           PDSU_t *pdsu_max;
           pdsu_max = calloc(1,sizeof(PDSU_t));
           pdsu_max->streamId = streamid_plmns_max[i];
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[i];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = s;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu_max->standardizedMeasResults,measvalue);
           assert(ret==0);
           ret = ASN_SEQUENCE_ADD(pdsus_CUCP,pdsu_max);
           assert(ret==0);
        }
        sid++;
    }

   PDSUs_t *pdsus_CUUP;
   pdsus_CUUP = calloc(1,sizeof(PDSUs_t));
   CUUP(pdsus_CUUP,cc_qos_DL,cc_snssai_DL,cc_plmns_DL,noOfplmns,noOfCUUPeachplmn,noOfSnssaieachDU,noOfDuseachplmn,QoS,1,plmnids,snssaiids);
   CUUP(pdsus_CUUP,cc_qos_UL,cc_snssai_UL,cc_plmns_UL,noOfplmns,noOfCUUPeachplmn,noOfSnssaieachDU,noOfDuseachplmn,QoS,2,plmnids,snssaiids);
   /*
   sid = 1;
   while(sid<=noOfStreams)
   {  
       int *streamid_DL,*streamid_UL;
       streamid_DL = calloc(noOfCUUPeachplmn*noOfplmns,sizeof(int));
       streamid_UL = calloc(noOfCUCPeachplmn*noOfplmns,sizeof(int));
       for(int i =0;i<noOfCUUPeachplmn*noOfplmns;i++)
       {
           streamid_DL[i] |= 2<<20;
           streamid_DL[i] |= (i+1)<<12;
           streamid_UL[i] = streamid_DL[i];
           streamid_DL[i] |= 1<<8;
           streamid_UL[i] |= 2<<8;
       }
       int ret;
       int id = 0;

       while(id<noOfCUUPeachplmn*noOfplmns)
       {
           int S = 0;
           int noOfsnssai = noOfSnssaieachDU*noOfDuseachplmn;
           for(int i = id*noOfsnssai;i<(id+1)*noOfsnssai;i++)
           {
               int s = 0;
               for(int j = 0;j<QoS;j++)
               {
                   PDSU_t *pdsu_dl;
                   pdsu_dl = calloc(1,sizeof(PDSU_t));
                   pdsu_dl->streamId = streamid_DL[id];
                   MeasValue_t *measvalue;
                   measvalue = calloc(1,sizeof(MeasValue_t));
                   measvalue->present = 3;
                   SubCounterListType_t *plmn;
                   plmn = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *plmnid;
                   plmnid = calloc(1,sizeof(SubCounterIndexType_t));
                   plmnid->present = 7;
                   plmnid->choice.plMN = plmnids[k];
                   plmn->subCounterIndex = *plmnid;
                   MeasValue_t *measvalue1;
                   measvalue1 = calloc(1,sizeof(MeasValue_t));
                   measvalue1->present = 3;
                   SubCounterListType_t *snssai;
                   snssai = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *snssaiid;
                   snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
                   snssaiid->present = 8;
                   snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
                   snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
                   snssai->subCounterIndex = *snssaiid;
                   MeasValue_t *measvalue2;
                   measvalue2 = calloc(1,sizeof(MeasValue_t));
                   measvalue2->present = 3;
                   SubCounterListType_t *qos;
                   qos = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *qoslevel;
                   qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
                   qoslevel->present = 3;
                   qoslevel->choice.qOS_5QI = j;
                   qos->subCounterIndex = *qoslevel;
                   int t = rand()/M;
                   s += t;
                   MeasValue_t *measvalue3;
                   measvalue3 = calloc(1,sizeof(MeasValue_t));
                   measvalue3->present = 1;
                   measvalue3->choice.integerValue = t;
                   cc_qos_DL[i*noOfsnssai + j] += t;
                   qos->subCounterValue = measvalue3;
                   measvalue2->choice.subCounters = qos;
                   snssai->subCounterValue = measvalue2;
                   measvalue1->choice.subCounters = snssai;
                   plmn->subCounterValue = measvalue1;
                   measvalue->choice.subCounters = plmn;
                   ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
                   assert(ret==0);
                   //choice
                   PDSU_t *pdsu;
                   pdsu = calloc(1,sizeof(PDSU_t));
                   pdsu->streamId = streamid_DL[id];
                   MeasValue_t *measval;
                   measval = calloc(1,sizeof(MeasValue_t));
                   measval->present = 3;
                   SubCounterListType_t *qoslt;
                   qoslt = calloc(1,sizeof(SubCounterListType_t));
                   qoslt->subCounterIndex = *qoslevel;
                   MeasValue_t *measval1;
                   measval1 = calloc(1,sizeof(MeasValue_t));
                   measval1->present = 1;
                   measval1->choice.integerValue = cc_qos_DL[i*noOfsnssai + j];
                   qoslt->subCounterValue = measval1;
                   measval->choice.subCounters = qoslt;
                   ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
                   assert(ret==0);
                   //choice
                    PDSU_t *pdsu1;
                    pdsu1 = calloc(1,sizeof(PDSU_t));
                    pdsu1->streamId = streamid_DL[id];
                    MeasValue_t *meas;
                    meas = calloc(1,sizeof(MeasValue_t));
                    meas->present = 3;
                    SubCounterListType_t *snssailst;
                    snssailst = calloc(1,sizeof(SubCounterListType_t));
                    snssailst->subCounterIndex = *snssaiid;
                    snssailst->subCounterValue = measvalue2;
                    meas->choice.subCounters = snssailst;
                   ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,meas);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu1);
                   assert(ret==0);
               }
               PDSU_t *pdsu_dl;
               pdsu_dl = calloc(1,sizeof(PDSU_t));
               pdsu_dl->streamId = streamid_DL[id];
               MeasValue_t *measvalue;
               measvalue = calloc(1,sizeof(MeasValue_t));
               measvalue->present = 3;
               SubCounterListType_t *plmn;
               plmn = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *plmnid;
               plmnid = calloc(1,sizeof(SubCounterIndexType_t));
               plmnid->present = 7;
               plmnid->choice.plMN = plmnids[k];
               plmn->subCounterIndex = *plmnid;
               MeasValue_t *measvalue1;
               measvalue1 = calloc(1,sizeof(MeasValue_t));
               measvalue1->present = 3;
               SubCounterListType_t *snssai;
               snssai = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *snssaiid;
               snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
               snssaiid->present = 8;
               snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
               snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
               snssai->subCounterIndex = *snssaiid;
               MeasValue_t *measvalue2;
               measvalue2 = calloc(1,sizeof(MeasValue_t));
               measvalue2->present = 1;
               measvalue2->choice.integerValue = cc_snssai_DL[i] + s;
               cc_snssai_DL[i] += s;
               S += s;
               snssai->subCounterValue = measvalue2;
               measvalue1->choice.subCounters = snssai;
               plmn->subCounterValue = measvalue1;
               measvalue->choice.subCounters = plmn;
               ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
               assert(ret==0);
               ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
               assert(ret==0);
                //choice
                PDSU_t *pdsu;
                pdsu = calloc(1,sizeof(PDSU_t));
                pdsu->streamId = streamid_DL[id];
                MeasValue_t *measval;
                measval = calloc(1,sizeof(MeasValue_t));
                measval->present = 3;
                SubCounterListType_t *snssailt;
                snssailt = calloc(1,sizeof(SubCounterListType_t));
                snssailt->subCounterIndex = snnsaiid;
                MeasValue_t *measval1;
                measval1 = calloc(1,sizeof(MeasValue_t));
                measval1->present = 1;
                measval1->choice.integerValue = cc_snssai_DL[i];
                snssailt->subCounterValue = measval1;
                measval->choice.subCounters = qoslt;
                ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
                assert(ret==0);
                ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
                assert(ret==0);

           }
           PDSU_t *pdsu_dl;
           pdsu_dl = calloc(1,sizeof(PDSU_t));
           pdsu_dl->streamId = streamid_DL[id];
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[k];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = cc_plmns_DL[k] + S;
           cc_plmns_DL[k] += S;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu_dl->standardizedMeasResults,measvalue);
           assert(ret==0);
           ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_dl);
           assert(ret==0);
           //choice
           for(int i = 0;i<QoS;i++)
           {
              int s = 0;
              for(int j = 0;j<noOfCUUPeachplmn;j++)
              {
                  s += cc_qos_DL[j*Qos*noOfsnssai + i]
              }
              PDSU_t *pdsu;
              pdsu = calloc(1,sizeof(PDSU_t));
              pdsu->streamId = streamid_DL[id];
              MeasValue_t *measval;
              measval = calloc(1,sizeof(MeasValue_t));
              measval->present = 3;
              SubCounterListType_t *plmnlt;
              plmnlt = calloc(1,sizeof(SubCounterListType_t));
              plmnlt->subCounterIndex = plmnid;
              MeasValue_t *measval1;
              measval1 = calloc(1,sizeof(MeasValue_t));
              measval1->present = 3;
              SubCounterListType_t *qoslt;
              qoslt = calloc(1,sizeof(SubCounterListType_t));
              SubCounterIndexType_t *qoslevel;
              qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
              qoslevel->present = 3;
              qoslevel->choice.qOS_5QI = i;
              qoslt->subCounterIndex = *qoslevel;
              MeasValue_t *measval2;
              measval2 = calloc(1,sizeof(MeasValue_t));
              measval2->present = 1;
              measval2->choice.integerValue = s;
              qoslt->subCounterValue = measval2;
              measval1->choice.subCounters = qoslt;
              plmnlt->subCounterValue = measval1;
              measval->choice.subCounters = qoslt;
              ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measval);
              assert(ret==0);
              ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu);
              assert(ret==0); 
           }
           id++;
       }
       id = 0;
       while(id<noOfCUUPeachplmn*noOfplmns)
       {
           int S = 0;
           int k = id/noOfCUUPeachplmn;
           int noOfsnssai = noOfSnssaieachDU*noOfDuseachplmn;
           for(int i = k*noOfsnssai;i<(k+1)*noOfsnssai;i++)
           {
               int s = 0;
               for(int j = 0;j<QoS;j++)
               {
                   PDSU_t *pdsu_ul;
                   pdsu_ul = calloc(1,sizeof(PDSU_t));
                   pdsu_ul->streamId = streamid_UL[id];
                   MeasValue_t *measvalue;
                   measvalue = calloc(1,sizeof(MeasValue_t));
                   measvalue->present = 3;
                   SubCounterListType_t *plmn;
                   plmn = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *plmnid;
                   plmnid = calloc(1,sizeof(SubCounterIndexType_t));
                   plmnid->present = 7;
                   plmnid->choice.plMN = plmnids[k];
                   plmn->subCounterIndex = *plmnid;
                   MeasValue_t *measvalue1;
                   measvalue1 = calloc(1,sizeof(MeasValue_t));
                   measvalue1->present = 3;
                   SubCounterListType_t *snssai;
                   snssai = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *snssaiid;
                   snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
                   snssaiid->present = 8;
                   snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
                   snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
                   snssai->subCounterIndex = *snssaiid;
                   MeasValue_t *measvalue2;
                   measvalue2 = calloc(1,sizeof(MeasValue_t));
                   measvalue2->present = 3;
                   SubCounterListType_t *qos;
                   qos = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *qoslevel;
                   qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
                   qoslevel->present = 3;
                   qoslevel->choice.qOS_5QI = j;
                   qos->subCounterIndex = *qoslevel;
                   int t = rand()/M;
                   s += t;
                   MeasValue_t *measvalue3;
                   measvalue3 = calloc(1,sizeof(MeasValue_t));
                   measvalue3->present = 1;
                   measvalue3->choice.integerValue = t;
                   cc_qos_UL[j] += t;
                   qos->subCounterValue = measvalue3;
                   measvalue2->choice.subCounters = qos;
                   snssai->subCounterValue = measvalue2;
                   measvalue1->choice.subCounters = snssai;
                   plmn->subCounterValue = measvalue1;
                   measvalue->choice.subCounters = plmn;
                   ret = ASN_SEQUENCE_ADD(&pdsu_ul->standardizedMeasResults,measvalue);
                   assert(ret==0);
                   ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_ul);
                   assert(ret==0);
               }
               PDSU_t *pdsu_ul;
               pdsu_ul = calloc(1,sizeof(PDSU_t));
               pdsu_ul->streamId = streamid_UL[id];
               MeasValue_t *measvalue;
               measvalue = calloc(1,sizeof(MeasValue_t));
               measvalue->present = 3;
               SubCounterListType_t *plmn;
               plmn = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *plmnid;
               plmnid = calloc(1,sizeof(SubCounterIndexType_t));
               plmnid->present = 7;
               plmnid->choice.plMN = plmnids[k];
               plmn->subCounterIndex = *plmnid;
               MeasValue_t *measvalue1;
               measvalue1 = calloc(1,sizeof(MeasValue_t));
               measvalue1->present = 3;
               SubCounterListType_t *snssai;
               snssai = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *snssaiid;
               snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
               snssaiid->present = 8;
               snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
               snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
               snssai->subCounterIndex = *snssaiid;
               MeasValue_t *measvalue2;
               measvalue2 = calloc(1,sizeof(MeasValue_t));
               measvalue2->present = 1;
               measvalue2->choice.integerValue = cc_snssai_UL[i] + s;
               cc_snssai_UL[i] += s;
               S += s;
               snssai->subCounterValue = measvalue2;
               measvalue1->choice.subCounters = snssai;
               plmn->subCounterValue = measvalue1;
               measvalue->choice.subCounters = plmn;
               ret = ASN_SEQUENCE_ADD(&pdsu_ul->standardizedMeasResults,measvalue);
               assert(ret==0);
               ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_ul);
               assert(ret==0);
           }
           PDSU_t *pdsu_ul;
           pdsu_ul = calloc(1,sizeof(PDSU_t));
           pdsu_ul->streamId = streamid_UL[id];
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[k];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = cc_plmns_UL[k] + S;
           cc_plmns_UL[k] += S;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu_ul->standardizedMeasResults,measvalue);
           assert(ret==0);
           ret = ASN_SEQUENCE_ADD(pdsus_CUUP,pdsu_ul);
           assert(ret==0);
           id++;
           //choice

       }
       /*
       for(int i =0;i<noOfplmns;i++)
       {   /*
           if(complmn[i].data==-1)
           {
               continue;
           }
           int Sum = 0;
           for(int j = 0;j<noOfSnssaieachDU*noOfDuseachplmn;j++)
           {
               int s = 0;
               for(int k = 0;k<QoS;k++)
               {
                   PDSU_t *pdsu_dl;
                   pdsu_dl = calloc(1,sizeof(PDSU_t));
                   pdsu_
                   MeasValue_t *measvalue;
                   measvalue = calloc(1,sizeof(MeasValue_t));
                   measvalue->present = 3;
                   SubCounterListType_t *plmn;
                   plmn = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *plmnid;
                   plmnid = calloc(1,sizeof(SubCounterIndexType_t));
                   plmnid->present = 7;
                   plmnid->choice.plMN = plmnids[i];
                   plmn->subCounterIndex = *plmnid;
                   MeasValue_t *measvalue1;
                   measvalue1 = calloc(1,sizeof(MeasValue_t));
                   measvalue1->present = 3;
                   SubCounterListType_t *snssai;
                   snssai = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *snssaiid;
                   snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
                   snssaiid->present = 8;
                   snssaiid->choice.sNSSAI.sd = snssaiids[2*i + j].sd;
                   snssaiid->choice.sNSSAI.sst = snssaiids[2*i + j].sst;
                   snssai->subCounterIndex = *snssaiid;
                   MeasValue_t *measvalue2;
                   measvalue2 = calloc(1,sizeof(MeasValue_t));
                   measvalue2->present = 3;
                   SubCounterListType_t *qos;
                   qos = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *qoslevel;
                   qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
                   qoslevel->present = 3;
                   qoslevel->choice.qOS_5QI = k;
                   qos->subCounterIndex = *qoslevel;
                   int t = rand()/M;
                   s += t;
                   MeasValue_t *measvalue3;
                   measvalue3 = calloc(1,sizeof(MeasValue_t));
                   measvalue3->present = 1;
                   measvalue3->choice.integerValue = t;
                   cc_qos_DL[k] += t;
                   qos->subCounterValue = measvalue3;
                   measvalue2->choice.subCounters = qos;
                   snssai->subCounterValue = measvalue2;
                   measvalue1->choice.subCounters = snssai;
                   plmn->subCounterValue = measvalue1;
                   measvalue->choice.subCounters = plmn;
                   ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
                   assert(ret==0);
               }
               MeasValue_t *measvalue;
               measvalue = calloc(1,sizeof(MeasValue_t));
               measvalue->present = 3;
               SubCounterListType_t *plmn;
               plmn = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *plmnid;
               plmnid = calloc(1,sizeof(SubCounterIndexType_t));
               plmnid->present = 7;
               plmnid->choice.plMN = plmnids[i];
               plmn->subCounterIndex = *plmnid;
               MeasValue_t *measvalue1;
               measvalue1 = calloc(1,sizeof(MeasValue_t));
               measvalue1->present = 3;
               SubCounterListType_t *snssai;
               snssai = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *snssaiid;
               snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
               snssaiid->present = 8;
               snssaiid->choice.sNSSAI.sd = snssaiids[2*i + j].sd;
               snssaiid->choice.sNSSAI.sst = snssaiids[2*i + j].sst;
               snssai->subCounterIndex = *snssaiid;
               MeasValue_t *measvalue2;
               measvalue2 = calloc(1,sizeof(MeasValue_t));
               measvalue2->present = 1;
               measvalue2->choice.integerValue = cc_snssai_DL[2*i+j] + s;
               cc_snssai_DL[2*i + j] += s;
               Sum += s;
               snssai->subCounterValue = measvalue2;
               measvalue1->choice.subCounters = snssai;
               plmn->subCounterValue = measvalue1;
               measvalue->choice.subCounters = plmn;
               ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           }
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[i];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = cc_plmns_DL[i] + Sum;
           cc_plmns_DL[i] += Sum;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }
       ret = ASN_SEQUENCE_ADD(pdsus_CUUP_DL,pdsu);
        //xer_fprint(stdout,&asn_DEF_PDSU,pdsu);
        assert(ret==0);
        //DU
       sid++;

   }
   */
   /*
   PDSUs_t *pdsus_CUUP_UL;
   pdsus_CUUP_UL = calloc(1,sizeof(PDSUs_t));
   sid = 1;
   while(sid<=noOfStreams)
   {
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(PDSU_t));
       pdsu->streamId = sid;
       int ret;
       for(int i =0;i<noOfNrcells;i++)
       {
           if(complmn[i].data==-1)
           {
               continue;
           }
           int Sum = 0;
           for(int j = 0;j<2;j++)
           {
               int s = 0;
               for(int k = 0;k<QoS;k++)
               {
                   MeasValue_t *measvalue;
                   measvalue = calloc(1,sizeof(MeasValue_t));
                   measvalue->present = 3;
                   SubCounterListType_t *plmn;
                   plmn = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *plmnid;
                   plmnid = calloc(1,sizeof(SubCounterIndexType_t));
                   plmnid->present = 7;
                   plmnid->choice.plMN = plmnids[i];
                   plmn->subCounterIndex = *plmnid;
                   MeasValue_t *measvalue1;
                   measvalue1 = calloc(1,sizeof(MeasValue_t));
                   measvalue1->present = 3;
                   SubCounterListType_t *snssai;
                   snssai = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *snssaiid;
                   snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
                   snssaiid->present = 8;
                   snssaiid->choice.sNSSAI.sd = snssaiids[2*i + j].sd;
                   snssaiid->choice.sNSSAI.sst = snssaiids[2*i + j].sst;
                   snssai->subCounterIndex = *snssaiid;
                   MeasValue_t *measvalue2;
                   measvalue2 = calloc(1,sizeof(MeasValue_t));
                   measvalue2->present = 3;
                   SubCounterListType_t *qos;
                   qos = calloc(1,sizeof(SubCounterListType_t));
                   SubCounterIndexType_t *qoslevel;
                   qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
                   qoslevel->present = 3;
                   qoslevel->choice.qOS_5QI = k;
                   qos->subCounterIndex = *qoslevel;
                   int t = rand()/M;
                   s += t;
                   MeasValue_t *measvalue3;
                   measvalue3 = calloc(1,sizeof(MeasValue_t));
                   measvalue3->present = 1;
                   measvalue3->choice.integerValue = t;
                   cc_qos_UL[k] += t;
                   qos->subCounterValue = measvalue3;
                   measvalue2->choice.subCounters = qos;
                   snssai->subCounterValue = measvalue2;
                   measvalue1->choice.subCounters = snssai;
                   plmn->subCounterValue = measvalue1;
                   measvalue->choice.subCounters = plmn;
                   ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
                   assert(ret==0);
               }
               MeasValue_t *measvalue;
               measvalue = calloc(1,sizeof(MeasValue_t));
               measvalue->present = 3;
               SubCounterListType_t *plmn;
               plmn = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *plmnid;
               plmnid = calloc(1,sizeof(SubCounterIndexType_t));
               plmnid->present = 7;
               plmnid->choice.plMN = plmnids[i];
               plmn->subCounterIndex = *plmnid;
               MeasValue_t *measvalue1;
               measvalue1 = calloc(1,sizeof(MeasValue_t));
               measvalue1->present = 3;
               SubCounterListType_t *snssai;
               snssai = calloc(1,sizeof(SubCounterListType_t));
               SubCounterIndexType_t *snssaiid;
               snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
               snssaiid->present = 8;
               snssaiid->choice.sNSSAI.sd = snssaiids[2*i + j].sd;
               snssaiid->choice.sNSSAI.sst = snssaiids[2*i + j].sst;
               snssai->subCounterIndex = *snssaiid;
               MeasValue_t *measvalue2;
               measvalue2 = calloc(1,sizeof(MeasValue_t));
               measvalue2->present = 1;
               measvalue2->choice.integerValue = cc_snssai_UL[2*i+j] + s;
               cc_snssai_UL[2*i + j] += s;
               Sum += s;
               snssai->subCounterValue = measvalue2;
               measvalue1->choice.subCounters = snssai;
               plmn->subCounterValue = measvalue1;
               measvalue->choice.subCounters = plmn;
               ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           }
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[i];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = cc_plmns_UL[i] + Sum;
           cc_plmns_UL[i] += Sum;
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }
       ret = ASN_SEQUENCE_ADD(pdsus_CUUP_UL,pdsu);
        //xer_fprint(stdout,&asn_DEF_PDSU,pdsu);
        assert(ret==0);
        //DU
       sid++;

   }
   */
   PDSUs_t *pdsus_DU;
   pdsus_DU = calloc(1,sizeof(PDSUs_t));
   for(int i =0;i<noOfplmns*noOfDuseachplmn;i++)
   {
       int *streamid;
       streamid = calloc(1,sizeof(int));
       *streamid |= 3<<20;
       *streamid |= (i+1)<<12;
       *streamid |= 1<<8;
       PDSU_t *pdsu_availdl = DU_Prb(*streamid,0);
       int ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu_availdl);
       assert(ret==0);
   }
   for(int i =0;i<noOfplmns*noOfDuseachplmn;i++)
   {
       int *streamid;
       streamid = calloc(1,sizeof(int));
       *streamid |= 3<<20;
       *streamid |= (i+1)<<12;
       *streamid |= 2<<8;
       PDSU_t *pdsu_availul = DU_Prb(*streamid,0);
       int ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu_availul);
       assert(ret==0);
   }
   for(int i =0;i<noOfplmns*noOfDuseachplmn;i++)
   {
       int *streamid;
       streamid = calloc(1,sizeof(int));
       *streamid |= 3<<20;
       *streamid |= (i+1)<<12;
       *streamid |= 5<<8;
       PDSU_t *pdsu_Totdl = DU_Prb(*streamid,1);
       int ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu_Totdl);
       assert(ret==0);
   }
   for(int i =0;i<noOfplmns*noOfDuseachplmn;i++)
   {
       int *streamid;
       streamid = calloc(1,sizeof(int));
       *streamid |= 3<<20;
       *streamid |= (i+1)<<12;
       *streamid |= 6<<8;
       PDSU_t *pdsu_Totul = DU_Prb(*streamid,1);
       int ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu_Totul);
       assert(ret==0);
   }
   DU_Prb2(pdsus_DU,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,3,plmnids,snssaiids);
   DU_Prb2(pdsus_DU,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,4,plmnids,snssaiids);
   DU_Prb2(pdsus_DU,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,7,plmnids,snssaiids);
   DU_Prb2(pdsus_DU,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,8,plmnids,snssaiids);
   /*
   int noOfDus = noOfDuseachplmn*noOfplmns;
   int *qos;
   qos = data(noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS);
   int *streamid_dl;
   streamid_dl = calloc(noOfDus,sizeof(int));
   for(int i =0;i<noOfDus;i++)
   {
       streamid_dl[i] |= 3<<20;
       streamid_dl[i] |= (i+1)<<12;
       streamid_dl[i] |= 3<<8;
   }
   for(int i = 0;i<noOfSnssaieachDU*noOfDus;i++)
   {
       int id = i/noOfDus;
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(PDSU_t));
       pdsu->streamId = streamid_dl[id];
       int s = 0;
       for(int j =0;j<Qos;j++)
       {
           s += qos[i*Qos + j];
       }
       MeasValue_t *measvalue;
       measvalue = calloc(1,sizeof(MeasValue_t));
       measvalue->present = 3;
       SubCounterIndexType_t *snssaiid;
       snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
       snssaiid->present = 8;
       snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
       snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
       SubCounterListType_t *snssai;
       snssai = calloc(1,sizeof(SubCounterListType_t));
       snssai->subCounterIndex = *snssaiid;
       MeasValue_t *measvalue1;
       measvalue1 = calloc(1,sizeof(MeasValue_t));
       measvalue1->present = 1;
       measvalue1->choice.integerValue = s;
       snssai->subCounterValue = measvalue1;
       measvalue->choice.subCounters = snssai;
       ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
       assert(ret==0);
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
       assert(ret==0);
       
   }
   */
   /*
   int noOfDus = 2*noOfNrcells;
   PDSUs_t *pdsus_DU_PrbAvailDl,*pdsus_DU_PrbAvailUl;
   pdsus_DU_PrbAvailDl = DU_Prb(noOfStreams,noOfDus,0);
   pdsus_DU_PrbAvailUl = DU_Prb(noOfStreams,noOfDus,0);
   PDSUs_t *pdsus_DU_PrbTotDl,*pdsus_DU_PrbTotUl;
   pdsus_DU_PrbTotDl = DU_Prb(noOfStreams,noOfDus,1);
   pdsus_DU_PrbTotUl = DU_Prb(noOfStreams,noOfDus,1);
   /*
   PDSUs_t *pdsus_DU;
   pdsus_DU = calloc(1,sizeof(PDSUs_t));
   int noOfDUs = 10;
   sid = 1;
   while(sid<=noOfStreams)
   {
       PDSU_t *pdsu;
       pdsu = calloc(1,sizeof(PDSU_t));
       pdsu->streamId = sid;
       int t = noOfDUs;
       int ret;
       while(t--)
       {
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 1;
           measvalue->choice.integerValue = rand();
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }
       for(int i =0;i<noOfNrcells;i++)
       {
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *plmnid;
           plmnid = calloc(1,sizeof(SubCounterIndexType_t));
           plmnid->present = 7;
           plmnid->choice.plMN = plmnids[i];
           SubCounterListType_t *plmn;
           plmn = calloc(1,sizeof(SubCounterListType_t));
           plmn->subCounterIndex = *plmnid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = rand();
           plmn->subCounterValue = measvalue1;
           measvalue->choice.subCounters = plmn;
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }
       for(int i =0;i<10;i++)
       {
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *snssaiid;
           snssaiid = calloc(1,sizeof(SubCounterIndexType_t));
           snssaiid->present = 8;
           snssaiid->choice.sNSSAI.sd = snssaiids[i].sd;
           snssaiid->choice.sNSSAI.sst = snssaiids[i].sst;
           SubCounterListType_t *snssai;
           snssai = calloc(1,sizeof(SubCounterListType_t));
           snssai->subCounterIndex = *snssaiid;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = rand();
           snssai->subCounterValue = measvalue1;
           measvalue->choice.subCounters = snssai;
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }

       for(int i =0;i<10;i++)
       {
           MeasValue_t *measvalue;
           measvalue = calloc(1,sizeof(MeasValue_t));
           measvalue->present = 3;
           SubCounterIndexType_t *qoslevel;
           qoslevel = calloc(1,sizeof(SubCounterIndexType_t));
           qoslevel->present = 3;
           qoslevel->choice.qOS_5QI = i;
           SubCounterListType_t *qos;
           qos = calloc(1,sizeof(SubCounterListType_t));
           qos->subCounterIndex = *qoslevel;
           MeasValue_t *measvalue1;
           measvalue1 = calloc(1,sizeof(MeasValue_t));
           measvalue1->present = 1;
           measvalue1->choice.integerValue = rand();
           qos->subCounterValue = measvalue1;
           measvalue->choice.subCounters = qos;
           ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
           assert(ret==0);
       }
       ret = ASN_SEQUENCE_ADD(pdsus_DU,pdsu);
        //xer_fprint(stdout,&asn_DEF_PDSU,pdsu);
        assert(ret==0);
       sid++;
   }
   */

    //xer_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUCP);
    //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUCP);
    //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUUP);
    //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_DU);
    char errbuf[1024];
    size_t errlen = sizeof(errbuf);
    int ret = asn_check_constraints(&asn_DEF_PDSUs,pdsus_CUCP,errbuf,&errlen);
    if(ret)
    {
        fprintf(stderr,"CUCP_Mean Constraints failed : %s\n", errbuf);
    }
   //xer_fprint(stderr,&asn_DEF_PDSUs,pdsus_CUCP);
    if(argc>=2)
    {
        asn_enc_rval_t ec;
        const char *file = argv[1];
        FILE *fp = fopen(file, "wb"); 
            if(!fp) {
            perror(file);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_CUCP, write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_CUCP at (at %s)\n",
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        /*
        char *ext[2] = {"_Mean","_Max"};
        char *filename[2];
        for(int i =0;i<2;i++)
        {
            filename[i] = calloc(1024,sizeof(char));
            strcpy(filename[i],file);
            strcat(filename[i],ext[i]);
        }
        PDSUs_t *pdsus_CUCP[2] = {pdsus_CUCP_Mean,pdsus_CUCP_Max};
        for(int i =0;i<2;i++)
        {
            FILE *fp = fopen(filename[i], "wb"); 
            if(!fp) {
            perror(filename[i]);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_CUCP[i], write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_CUCP at i = %d (at %s)\n",i,
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        }
        */
        //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUCP);
        //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUCP_Max);
    }
    ret = asn_check_constraints(&asn_DEF_PDSUs,pdsus_CUUP,errbuf,&errlen);
    if(ret)
    {
        fprintf(stderr,"CUUP Constraints failed : %s\n", errbuf);
    }
    if(argc>=3)
    {
        asn_enc_rval_t ec;
        const char *file = argv[2];
        FILE *fp = fopen(file, "wb"); 
            if(!fp) {
            perror(file);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_CUUP, write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_CUUP at (at %s)\n",
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUUP);

        /*
        asn_enc_rval_t ec;
        char *file = argv[2];
        char *ext[2] = {"_DL","_UL"};
        char *filename[2];
        for(int i =0;i<2;i++)
        {
            filename[i] = calloc(1024,sizeof(char));
            strcpy(filename[i],file);
            strcat(filename[i],ext[i]);
        }
        PDSUs_t *pdsus_CUUP[2] = {pdsus_CUUP_DL,pdsus_CUUP_UL};
        for(int i =0;i<2;i++)
        {
            FILE *fp = fopen(filename[i], "wb"); 
            if(!fp) {
            perror(filename[i]);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_CUUP[i], write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_CUUP at i = %d (at %s)\n",i,
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        }
        */
        asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUUP);
    }
    ret = asn_check_constraints(&asn_DEF_PDSUs,pdsus_DU,errbuf,&errlen);
    if(ret)
    {
        fprintf(stderr,"DU Constraints failed : %s\n", errbuf);
    }
    if(argc>=4)
    {
        asn_enc_rval_t ec;
        const char *file = argv[3];
        FILE *fp = fopen(file, "wb"); 
            if(!fp) {
            perror(file);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_DU, write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_CUUP at (at %s)\n",
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_DU);
    
    }
    /*
    PDSUs_t *pdsus_DU[4];
    pdsus_DU[0] = pdsus_DU_PrbAvailDl;
    pdsus_DU[1] = pdsus_DU_PrbAvailUl;
    pdsus_DU[2] = pdsus_DU_PrbTotDl;
    pdsus_DU[3] = pdsus_DU_PrbTotUl; 
    for(int i =0;i<4;i++)
    {
        int ret = asn_check_constraints(&asn_DEF_PDSUs,pdsus_DU[i],errbuf,&errlen);
        if(ret)
        {
            fprintf(stderr,"DU at i = %d Constraints failed : %s\n", i,errbuf);
        }
    }
    if(argc>=4)
    {
        asn_enc_rval_t ec;
        char *file = argv[3];
        char *ext[4] = {"_PrbAvailDl","_PrbAvailUl","_PrbTotDl","_PrbTotUl"};
        char *filename[4];
        for(int i =0;i<4;i++)
        {
            filename[i] = calloc(1024,sizeof(char));
            strcpy(filename[i],file);
            strcat(filename[i],ext[i]);
        }
        for(int i =0;i<4;i++)
        {
            FILE *fp = fopen(filename[i], "wb"); 
            if(!fp) {
            perror(filename[i]);
            exit(1);
            }
            ec = uper_encode(&asn_DEF_PDSUs, pdsus_DU[i], write_out, fp);
            fclose(fp);
            if(ec.encoded == -1) {
            fprintf(stderr, "Could not encode PDSUS_DU at i = %d (at %s)\n",i,
            ec.failed_type ? ec.failed_type->name : "unknown");
            exit(1);
            }
        }
    }
    if(argc>=5)
    {
        int i = atoi(argv[4]);
        if(i==1)
        {
            asn_enc_rval_t ec;
            char *file = argv[1];
            char *ext[2] = {"_Mean","_Max"};
            char *filename[2];
            for(int i =0;i<2;i++)
            {
                filename[i] = calloc(1024,sizeof(char));
                strcpy(filename[i],file);
                strcat(filename[i],ext[i]);
            }
            int n = 2;
            PDSUs_t *pdsud[n];
            for(int i =0;i<n;i++)
            {
                pdsud[i] = calloc(1,sizeof(PDSUs_t));
                char buf[1024000];
                FILE *fp = fopen(filename[i],"r");
                size_t size = fread(buf,1,sizeof(buf),fp);
                fclose(fp);
                fprintf(stderr, "size - %lu\n" ,size);
                asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)(pdsud + i),buf,size,0,0);
                if(ret1.code==RC_WMORE)
                {
                    assert(1==0);
                }else if(ret1.code==RC_FAIL)
                {
                    assert(1==2);
                }
                fprintf(stderr,"%s\n", filename[i]);
                asn_fprint(stdout,&asn_DEF_PDSUs,pdsud[i]);
            }
        }else if(i==2)
        {
            asn_enc_rval_t ec;
            char *file = argv[2];
            char *ext[2] = {"_DL","_UL"};
            char *filename[2];
            for(int i =0;i<2;i++)
            {
                filename[i] = calloc(1024,sizeof(char));
                strcpy(filename[i],file);
                strcat(filename[i],ext[i]);
            }
            int n = 2;
            PDSUs_t *pdsud[n];
            for(int i =0;i<n;i++)
            {
                pdsud[i] = calloc(1,sizeof(PDSUs_t*));
                char buf[1024000];
                FILE *fp = fopen(filename[i],"r");
                size_t size = fread(buf,1,sizeof(buf),fp);
                fprintf(stderr, "size - %lu\n" ,size);
                asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)(pdsud + i),buf,size,0,0);
                if(ret1.code==RC_WMORE)
                {
                    assert(1==0);
                }else if(ret1.code==RC_FAIL)
                {
                    assert(1==2);
                }
                fprintf(stderr,"%s\n", filename[i]);
                asn_fprint(stdout,&asn_DEF_PDSUs,pdsud[i]);
            }
        }else if(i==3)
        {
            asn_enc_rval_t ec;
            char *file = argv[3];
            char *ext[4] = {"_PrbAvailDl","_PrbAvailUl","_PrbTotDl","_PrbTotUl"};
            char *filename[4];
            for(int i =0;i<4;i++)
            {
                filename[i] = calloc(1024,sizeof(char));
                strcpy(filename[i],file);
                strcat(filename[i],ext[i]);
            }
            int n = 4;
            PDSUs_t *pdsud[n];
            for(int i =0;i<n;i++)
            {
                pdsud[i] = calloc(1,sizeof(PDSUs_t*));
                char buf[1024000];
                FILE *fp = fopen(filename[i],"r");
                size_t size = fread(buf,1,sizeof(buf),fp);
                fclose(fp);
                fprintf(stderr, "size - %lu\n" ,size);
                asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)(pdsud + i),buf,size,0,0);
                if(ret1.code==RC_WMORE)
                {
                    assert(1==0);
                }else if(ret1.code==RC_FAIL)
                {
                    assert(1==2);
                }
                fprintf(stderr,"%s\n", filename[i]);
                asn_fprint(stdout,&asn_DEF_PDSUs,pdsud[i]);
            }
        }
    }
    */
    return 0;

}
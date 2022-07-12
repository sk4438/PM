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
#include <time.h>
#include <unistd.h>
#include <nats/nats.h>



const long M = 1000000;
const long K = 1000;
const long MX = 1e9;



static int write_out(const void *buffer, size_t size, void *app_key) {
FILE *out_fp = app_key;
size_t wrote = fwrite(buffer, 1, size, out_fp);
return (wrote == size) ? 0 : -1;
}

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

void CUCP(PDSUs_t *pdsus_CUCP,int noOfNrcells,int noOfplmns,int noOfCUCPeachplmn,int noOfNrcellseachCUCP,int noOfStreams,OCTET_STRING_t *plmnids)
{
    int sid = 1;
    while(sid<=noOfStreams)
    {
        //CU-CP
        int ret;
        int vnrcells[noOfNrcells];
        int mnrcells[noOfNrcells];
        int *streamid_nrcells_mean,*streamid_nrcells_max;
        int *streamid_plmns_mean,*streamid_plmns_max;
        streamid_nrcells_mean = calloc(noOfNrcells,sizeof(int));
        streamid_plmns_mean = calloc(noOfplmns,sizeof(int));
        streamid_nrcells_max = calloc(noOfplmns,sizeof(int));
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
                streamid_plmns_mean[index/(noOfCUCPeachplmn*noOfNrcellseachCUCP)] = streamid_nrcells_mean[index];
                streamid_plmns_max[index/(noOfCUCPeachplmn*noOfNrcellseachCUCP)] = streamid_nrcells_max[index];
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
}

void CUUP(PDSUs_t *pdsus_CUUP,long *cc_qos_DL,long *cc_snssai_DL,long *cc_plmns_DL,int noOfplmns,int noOfCUUPeachplmn,int noOfSnssaieachDU,int noOfDUseachplmn,int QoS,int c,OCTET_STRING_t *plmnids,struct sNSSAI *snssaiids)
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
                   //PLMN,SNSSAI,QOS
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
                    //SNSSAI,QOS
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
               //PLMN,SNSSAI
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
                //SNSSAI
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
           //PLMN
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
           //PLMN,QOS
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
           //QOS
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

PDSU_t* DU_Prb(int streamid,int f)
{
    PDSU_t *pdsu;
    pdsu = calloc(1,sizeof(PDSU_t));
    pdsu->streamId = streamid;
    int ret;
    MeasValue_t *measvalue;
    measvalue = calloc(1,sizeof(MeasValue_t));
    measvalue->present = 1;
    measvalue->choice.integerValue = f?rand()%101:rand()%MX;
    ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
    assert(ret==0);
    return pdsu;
}

PDSU_t* PEE_Prb(int streamid,int *dt,int i)
{
    PDSU_t *pdsu;
    pdsu = calloc(1,sizeof(PDSU_t));
    pdsu->streamId = streamid;
    int ret;
    MeasValue_t *measvalue;
    measvalue = calloc(1,sizeof(MeasValue_t));
    measvalue->present = 1;
    measvalue->choice.integerValue = dt[30+i];
    ret = ASN_SEQUENCE_ADD(&pdsu->standardizedMeasResults,measvalue);
    assert(ret==0);
    return pdsu;
}

long* data(int noOfplmns,int noOfDuseachplmn,int noOfsnssaieachDU,int Qos,int c,int* dt)
{
    long *qos;
    qos = calloc(noOfplmns*noOfDuseachplmn*noOfsnssaieachDU*Qos,sizeof(long));
    for(int i = 0;i<noOfDuseachplmn*noOfplmns;i++)
    {
        for(int j = 0;j<noOfsnssaieachDU;j++)
        {
            for(int k = 0;k<Qos;k++)
            {
                if(i==noOfDuseachplmn*noOfplmns-1)
                {
                     qos[(i*noOfsnssaieachDU + j)*Qos+k] = dt[(i*noOfsnssaieachDU + j)*Qos+k + 1];
                }else{
                    qos[(i*noOfsnssaieachDU + j)*Qos+k] = dt[(i*noOfsnssaieachDU + j)*Qos+k];
                }
            }
        }
    }
    return qos;
}

void DU_Prb2(PDSUs_t *pdsus_DU,int noOfplmns,int noOfDuseachplmn,int noOfSnssaieachDu,int QoS,int c,OCTET_STRING_t *plmnids,struct sNSSAI *snssaiids,int *dt)
{
   int noOfDus = noOfDuseachplmn*noOfplmns;
   int ret;
   long *c_snssai,*c_Du;
   c_snssai = calloc(noOfDus*noOfSnssaieachDu,sizeof(long));
   c_Du = calloc(noOfDus,sizeof(long));
   long *qos;
   qos = data(noOfplmns,noOfDuseachplmn,noOfSnssaieachDu,QoS,c,dt);
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
      pdsu = calloc(1,sizeof(PDSU_t));
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
       pdsu1 = calloc(1,sizeof(PDSU_t));
       pdsu1->streamId = streamid[id];
       pdsu1->vendorSpecificMeasResults = NULL;
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
}

int main(int argc,char *argv[])
{   
    srand(time(0));
    int noOfNrcells,QoS,noOfDuseachplmn,noOfCUCPeachplmn;
    int noOfCUUPeachplmn,noOfSnssaieachDU,noOfNrcellseachCUCP,noOfNrcellseachCUUP;
    int noOfStreams = 10000;

    natsConnection      *conn = NULL;
    natsSubscription    *sub  = NULL;
    natsMsg             *msg  = NULL;
    natsStatus          s;
    //Reading Data from Config file
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
    //Generating PLMN IDs
    OCTET_STRING_t *plmnids;
    plmnids = calloc(noOfplmns,sizeof(OCTET_STRING_t));
    for(int i =0;i<noOfplmns;i++)
    {
        uint8_t *buf;
        buf = calloc(3,sizeof(uint8_t));
        for(int j = 0;j<3;j++)
        {
            int a = rand()%10,b = rand()%10;
            buf[j] |= (a);
            buf[j] |= (b)<<4;
        }
        int ret = OCTET_STRING_fromString(plmnids+i,(char *)buf);
    }
    //Generating sNSSAI IDs
    struct sNSSAI *snssaiids;
    snssaiids = calloc(noOfSnssaieachDU*noOfDuseachplmn*noOfplmns,sizeof(struct sNSSAI));
    for(int i =0;i<noOfSnssaieachDU*noOfDuseachplmn*noOfplmns;i++)
    {
        uint8_t *bufsst;
        bufsst = calloc(1,1);
        bufsst[0] = rand()%256;
        uint8_t *bufsd;
        bufsd = calloc(3,1);
        for(int j = 0;j<3;j++)
        {
            int a = rand()%256,b = rand()%256;
            bufsd[j] |= a;
        }
        snssaiids[i].sd.buf = bufsd;
        snssaiids[i].sd.size = 3;
        snssaiids[i].sst.buf = bufsst;
        snssaiids[i].sst.size = 1;
    }
    int noOfSnssaieachplmn = noOfSnssaieachDU*noOfDuseachplmn;
    int noOfgnb = 1;
    //Cumilative Counters For CUUP Node
    while(sid<=noOfStreams){
        clock_t t = clock();
        long *cc_plmns_DL,*cc_plmns_UL;
        cc_plmns_DL = calloc(noOfplmns*noOfCUUPeachplmn,sizeof(long));
        cc_plmns_UL = calloc(noOfplmns*noOfCUUPeachplmn,sizeof(long));
        long *cc_snssai_DL,*cc_snssai_UL;
        cc_snssai_DL = calloc(noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(long));
        cc_snssai_UL = calloc(noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(long));
        long *cc_qos_DL,*cc_qos_UL;
        cc_qos_DL = calloc(QoS*noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(long));
        cc_qos_UL = calloc(QoS*noOfSnssaieachplmn*noOfplmns*noOfCUUPeachplmn,sizeof(long));

        //CUCP Node PMs
        PDSUs_t *pdsus;
        pdsus = calloc(1,sizeof(PDSUs_t));
        //CUCP(pdsus,noOfNrcells,noOfplmns,noOfCUCPeachplmn,noOfNrcellseachCUCP,noOfStreams,plmnids);
        //CUUP Node PMs
        //CUUP(pdsus,cc_qos_DL,cc_snssai_DL,cc_plmns_DL,noOfplmns,noOfCUUPeachplmn,noOfSnssaieachDU,noOfDuseachplmn,QoS,1,plmnids,snssaiids);
        //CUUP(pdsus,cc_qos_UL,cc_snssai_UL,cc_plmns_UL,noOfplmns,noOfCUUPeachplmn,noOfSnssaieachDU,noOfDuseachplmn,QoS,2,plmnids,snssaiids);
        

        
        //DU Node PMs
        /*
        for(int i =0;i<noOfplmns*noOfDuseachplmn;i++)
        {
            int *streamid;
            streamid = calloc(1,sizeof(int));
            *streamid |= 3<<20;
            *streamid |= (i+1)<<12;
            *streamid |= 1<<8;
            PDSU_t *pdsu_availdl = DU_Prb(*streamid,0);
            int ret = ASN_SEQUENCE_ADD(pdsus,pdsu_availdl);
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
            int ret = ASN_SEQUENCE_ADD(pdsus,pdsu_availul);
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
            int ret = ASN_SEQUENCE_ADD(pdsus,pdsu_Totdl);
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
            int ret = ASN_SEQUENCE_ADD(pdsus,pdsu_Totul);
            assert(ret==0);
        }
        */
        //DU_Prb2(pdsus,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,3,plmnids,snssaiids);
        //DU_Prb2(pdsus,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,4,plmnids,snssaiids);
        s = natsConnection_ConnectTo(&conn, NATS_DEFAULT_URL);
        printf("connected\n");
        if (s == NATS_OK)
        {
            // Creates an synchronous subscription on subject "foo".
            s = natsConnection_SubscribeSync(&sub, conn, "foo");
        }
        if (s == NATS_OK)
        {
            // With synchronous subscriptions, one need to poll
            // using this function. A timeout is used to instruct
            // how long we are willing to wait. The wait is in milliseconds.
            // So here, we are going to wait for 5 seconds.
            printf("hello\n");
            s = natsSubscription_NextMsg(&msg, sub, 10000);
        }
        int state;
        int *samp;
        samp = calloc(noOfDuseachplmn*noOfplmns*QoS + noOfgnb,sizeof(int));
        if (s == NATS_OK)
        {
            const char *data = natsMsg_GetData(msg);
            int temp =0;
            int id = 0;
            for(int i =0;i<strlen(data);i++)
            {
                if(data[i]==' ')
                {
                    if(id==0)
                    {
                        state = temp;
                    }else{
                        samp[id-1] = temp;
                    }
                    temp = 0;
                    id++;
                }else
                {
                    temp = temp*10 + data[i]-'0';
                }
            }
            // Need to destroy the message!
            natsMsg_Destroy(msg);
        }
        DU_Prb2(pdsus,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,7,plmnids,snssaiids,samp);
        //DU_Prb2(pdsus,noOfplmns,noOfDuseachplmn,noOfSnssaieachDU,QoS,8,plmnids,snssaiids);
        //PEE.Energy
        for(int i =0;i<noOfgnb;i++)
        {
            int *streamid;
            streamid = calloc(1,sizeof(int));
            *streamid |= 4<<20;
            *streamid |= (i+1)<<12;
            *streamid |= 1<<8;
            PDSU_t *pdsu_energy = PEE_Prb(*streamid,samp,i);
            int ret = ASN_SEQUENCE_ADD(pdsus,pdsu_energy);
            assert(ret==0);
        }

        //Checking Constraints of Generated CUCP PMs
        /*
        char errbuf[1024];
        size_t errlen = sizeof(errbuf);
        int ret = asn_check_constraints(&asn_DEF_PDSUs,pdsus,errbuf,&errlen);
        if(ret)
        {
            fprintf(stderr," Constraints failed : %s\n", errbuf);
        }
        */
        //Encoding Generated CUCP PMs
        if(true)
        {
                asn_enc_rval_t ec;
                const char *file = "pencode";
                FILE *fp = fopen(file, "wb"); 
                if(!fp) {
                perror(file);
                exit(1);
                }
                asn_fprint(stdout,&asn_DEF_PDSUs,pdsus);
                ec = uper_encode(&asn_DEF_PDSUs, pdsus, write_out, fp);
                fclose(fp);
                if(ec.encoded == -1) {
                fprintf(stderr, "Could not encode PDSUS at (at %s)\n",
                ec.failed_type ? ec.failed_type->name : "unknown");
                exit(1);
                }
                //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus);
                //asn_fprint(stdout,&asn_DEF_PDSUs,pdsus_CUCP_Max);
        }
        /*
        if(argc>=5)
        {
            int i = atoi(argv[4]);
            asn_enc_rval_t ec;
            const char *filename = argv[i];
            PDSUs_t *pdsud;
            pdsud = calloc(1,sizeof(PDSUs_t));
            char buf[1024000];
            FILE *fp = fopen(filename,"r");
            size_t size = fread(buf,1,sizeof(buf),fp);
            fclose(fp);
            fprintf(stderr, "size - %lu\n" ,size);
            asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)&pdsud,buf,size,0,0);
            if(ret1.code==RC_WMORE)
            {
                assert(1==0);
            }else if(ret1.code==RC_FAIL)
            {
                assert(1==2);
            }
            asn_fprint(stdout,&asn_DEF_PDSUs,pdsud);
        }
        */
       sleep(2);
       t = clock() -t;
       double tt = ((double)t)/CLOCKS_PER_SEC;
       fprintf(stderr,"time taken %f\n",tt);
       sid++;
    }
    return 0;

}
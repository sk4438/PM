package main

/*
#cgo CFLAGS: -g
#cgo LDFLAGS: -L . -lasn
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

PDSUs_t* decode(int i)
{
    const char *filename = i==1?"decode1":"decode";
    PDSUs_t *pdsud;
    pdsud = calloc(1,sizeof(PDSUs_t));
    char buf[1024000];
    FILE *fp = fopen(filename,"r");
    size_t size = fread(buf,1,sizeof(buf),fp);
    fclose(fp);
    asn_dec_rval_t ret1 = uper_decode(0,&asn_DEF_PDSUs,(void **)&pdsud,buf,size,0,0);
    if(ret1.code==RC_WMORE)
    {
        assert(1==0);
    }else if(ret1.code==RC_FAIL)
    {
        assert(1==2);
    }
	char errbuf[1024];
    size_t errlen = sizeof(errbuf);
    int ret = asn_check_constraints(&asn_DEF_PDSUs,pdsud,errbuf,&errlen);
    if(ret)
    {
        fprintf(stderr," Constraints failed : %s\n", errbuf);
    }
	return pdsud;
}
*/
import "C"

import (
	"cm/pb"
	"context"
	"encoding/binary"
	"flag"
	"fmt"
	"log"
	"os"
	"runtime"
	"strconv"
	"time"
	"unsafe"

	"github.com/golang/protobuf/proto"
	"github.com/gorilla/websocket"
	influxdb2 "github.com/influxdata/influxdb-client-go/v2"
	"github.com/nats-io/nats.go"
)

type PerformanceMetrics struct {
	PerformanceInfo []string
}

type StreamInfo struct {
	JobID       string
	StreamID    string
	StreamType  string
	MeasObjDn   string
	Measurement PerformanceMetrics
}

type SNSSAIID struct {
	sst C.struct_OCTET_STRING
	sd  C.struct_OCTET_STRING
}

type SNSSAI struct {
	sst string
	sd  string
}

type SubCounterIndexType struct {
	present int
	Qos     int
	plmn    string
	snssai  SNSSAI
}

type Subcounterlisttype struct {
	subcounterindex SubCounterIndexType
	meas            *Meas
}

type Meas struct {
	present    int
	integerval uint64
	visiblestr string
	subcounter *Subcounterlisttype
}

type PDSU struct {
	streamId int
	meas     Meas
}

type PDSUs struct {
	pdsu []PDSU
}

type SubCounterListType struct {
	SubCounterIndex C.struct_SubCounterIndexType
	SubCounterValue *C.struct_MeasValue
}
type OCTET struct {
	octet C.struct_OCTET_STRING
}

var filenam = [2]string{"decode", "decode1"}
var mapofid_info = make(map[string]*pb.StreamInfo) //Map of streamid to Streaminfo

func IngestData(client influxdb2.Client, tag_set StreamInfo, optional_tag map[string]string, val int) {

	//client := influxdb2.NewClient("http://localhost:8086", "583UlunwA5q1klguTpzOPEknmLlGS7eMlklJnkI_UmBOWxKD7xNfbYhT9tDzLdej4cVg9xyVdA-kawaYAalhkQ==")
	writeAPI := client.WriteAPIBlocking("nms", "pmsb")

	p := influxdb2.NewPointWithMeasurement(tag_set.Measurement.PerformanceInfo[0]).
		AddTag("JobID", tag_set.JobID).
		AddTag("StreamID", tag_set.StreamID).
		AddTag("StreamType", tag_set.StreamType).
		AddTag("MeasObjDn", tag_set.MeasObjDn).
		AddField("PMValue", val).
		SetTime(time.Now())

	for key, element := range optional_tag {
		p = p.AddTag(key, element)
	}

	writeAPI.WritePoint(context.Background(), p)

}

func IngestStreaminfo(client influxdb2.Client, streaminfo pb.StreamInfo, tim time.Time, present bool) {
	writeAPI := client.WriteAPIBlocking("nms", "pmsb")

	perinfo := streaminfo.GetPerformanceinfo()
	p := influxdb2.NewPointWithMeasurement("Streaminfo").
		AddTag("JobID", perinfo.GetJobId()).
		AddTag("MeasObjDn", perinfo.GetMeasObjDn()).
		AddTag("StreamId", streaminfo.GetStreamID()).
		AddTag("StreamType", streaminfo.GetStreamType()).
		AddTag("PerfomanceMetric", perinfo.GetPerformanceMetrics()[0]).
		AddField("Present", present).
		SetTime(tim)

	writeAPI.WritePoint(context.Background(), p)
}

func sendReply(msg *nats.Msg) {
	map_streammessage := pb.StreamMessageRequest{}

	err := proto.Unmarshal(msg.Data, &map_streammessage)
	if err != nil {
		log.Fatalf("Error on unmarshal: %v", err)
		return
	}
	//After Unmarshalling,we will map the streamid and stream info and add/delete from the map depending on the request type.
	str := map_streammessage.Type
	request_type := (pb.StreamMessageRequest_Type_name[int32(str)])
	//If request type is "ADD"
	if map_streammessage.Type == pb.StreamMessageRequest_ADD {
		st_id := map_streammessage.Streaminfo.StreamID
		var tim time.Time
		var flag bool = false
		present := false
		client := influxdb2.NewClient("http://localhost:8086", "5vgOEDvtHSYxMZLgxj3l2qbpKtmQgBFY2exkYSDGdzvVVaSBB7jsfiv2xUaJzA_NCl4xtaAL9JRmeXITH1rkVQ==")
		defer client.Close()
		queryAPI := client.QueryAPI("nms")
		result, err := queryAPI.Query(context.Background(), `from(bucket:"pmsb")|> range(start: -365d) |>filter(fn: (r)=>(r._measurement=="Streaminfo"))`)
		if err == nil {
			for result.Next() {
				val := result.Record().Values()
				if val["StreamId"] == st_id {
					present = val["_value"].(bool)
					flag = true
					tim = val["_time"].(time.Time)
					break
				}
			}

		} else {
			fmt.Println(err)
		}
		if !present {
			var st_obj pb.StreamInfo
			st_obj.StreamID = map_streammessage.Streaminfo.StreamID
			st_obj.StreamType = map_streammessage.Streaminfo.StreamType
			st_obj.SerializationFormat = map_streammessage.Streaminfo.SerializationFormat

			st_obj.Info = map_streammessage.Streaminfo.Info

			if !flag {
				tim = time.Now()
			}
			IngestStreaminfo(client, st_obj, tim, true)

			reply := pb.StreamMessageResponse{
				StreamID: st_id,
				Type:     request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
		} else {
			reply := pb.StreamMessageResponse{
				StreamID: st_id + " This streamId is already present in the map.",
				Type:     request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
		}
	}
	//If request type is "DELETE"
	if map_streammessage.Type == pb.StreamMessageRequest_DELETE {

		st_id := map_streammessage.Streaminfo.StreamID
		var tim time.Time
		present := false
		client := influxdb2.NewClient("http://localhost:8086", "5vgOEDvtHSYxMZLgxj3l2qbpKtmQgBFY2exkYSDGdzvVVaSBB7jsfiv2xUaJzA_NCl4xtaAL9JRmeXITH1rkVQ==")
		defer client.Close()
		queryAPI := client.QueryAPI("nms")
		result, err := queryAPI.Query(context.Background(), `from(bucket:"pmsb")|> range(start: -365d) |>filter(fn: (r)=>(r._measurement=="Streaminfo"))`)
		if err == nil {
			for result.Next() {
				val := result.Record().Values()

				if val["StreamId"] == st_id {
					present = val["_value"].(bool)
					tim = val["_time"].(time.Time)
					break
				}
			}

		} else {
			fmt.Println(err)
		}

		if present {
			var st_obj pb.StreamInfo
			st_obj.StreamID = map_streammessage.Streaminfo.StreamID
			st_obj.StreamType = map_streammessage.Streaminfo.StreamType
			st_obj.SerializationFormat = map_streammessage.Streaminfo.SerializationFormat

			st_obj.Info = map_streammessage.Streaminfo.Info
			IngestStreaminfo(client, st_obj, tim, false)
			reply := pb.StreamMessageResponse{
				StreamID: st_id,
				Type:     request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)

		} else {
			reply := pb.StreamMessageResponse{
				StreamID: "StreamInfo with ID: " + st_id + " is not present in the map. Please enter proper StreamId",
				Type:     request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)

		}

	}
}

func requestHandler() {
	var urls = flag.String("s", nats.DefaultURL, "nats server URLs separated by comma")
	log.Printf("Waiting to add a streammessage")
	nc, err := nats.Connect(*urls)
	if err != nil {
		log.Fatal(err)
	}
	nc.Subscribe("Streamconfig", sendReply)
	nc.Flush()
	runtime.Goexit()
}

func main() {
	fmt.Println("I am running")
	go requestHandler()
	conn, _, err := websocket.DefaultDialer.Dial("ws://127.0.0.1:8080/stream", nil)
	if err != nil {
		log.Fatal("Unable to connect %s", err.Error())
	}
	var i = 1
	var fp *os.File
	for {
		i = 1 - i
		_, p, err := conn.ReadMessage()
		fmt.Println("data recived")
		fp, err = os.OpenFile(filenam[i], os.O_CREATE|os.O_WRONLY, 0644)
		_, err = fp.Write(p)
		fmt.Println("data recived written to the file")
		if err != nil {
			fmt.Println("Unable to write to the file : ", err.Error())
			return
		}
		fmt.Println("a Stream is finished and decoding start", i)
		var pdsus = *C.decode(C.int(i))
		client := influxdb2.NewClient("http://localhost:8086", "5vgOEDvtHSYxMZLgxj3l2qbpKtmQgBFY2exkYSDGdzvVVaSBB7jsfiv2xUaJzA_NCl4xtaAL9JRmeXITH1rkVQ==")
		var count = int((pdsus).list.count)
		st := unsafe.Pointer(((pdsus).list.array))
		size := unsafe.Sizeof(*((pdsus).list.array))
		for j := 0; j < count; j++ {
			pdsu := *(**C.struct_PDSU)(unsafe.Pointer(uintptr(st) + size*uintptr(j)))
			var stid = int(pdsu.streamId)
			var mp map[string]interface{}
			present := false
			queryAPI := client.QueryAPI("nms")
			result, err := queryAPI.Query(context.Background(), `from(bucket:"pmsb")|> range(start: -365d) |>filter(fn: (r)=>r._measurement=="Streaminfo")`)
			if err == nil {
				for result.Next() {
					val := result.Record().Values()
					fmt.Println(val)
					if val["StreamId"] == strconv.Itoa(stid) {
						present = val["_value"].(bool)
						mp = val
						break
					}
				}

			} else {
				fmt.Println(err)
			}
			if present {
				//fmt.Println("Present")
				stmeas := **(pdsu.standardizedMeasResults.list.array)
				//fmt.Println(pdsu.standardizedMeasResults.list.count)
				//fmt.Println(pdsu.standardizedMeasResults.list.size)
				fmt.Println(stid)
				stminfo := StreamInfo{
					StreamID:    mp["StreamId"].(string),
					StreamType:  mp["StreamType"].(string),
					JobID:       mp["JobID"].(string),
					MeasObjDn:   mp["MeasObjDn"].(string),
					Measurement: PerformanceMetrics{PerformanceInfo: []string{mp["PerfomanceMetric"].(string)}},
				}
				present := 1
				subtype := make(map[string]string)
				for {
					if stmeas.present == 1 {
						if present == 1 {
							val := binary.LittleEndian.Uint64(stmeas.choice[:])
							IngestData(client, stminfo, subtype, int(val))
						} else if present == 3 {
							val := binary.LittleEndian.Uint64(stmeas.choice[:])
							IngestData(client, stminfo, subtype, int(val))
						}
						break
					} else if stmeas.present == 3 {
						present = 3
						var sublst = *(**SubCounterListType)(unsafe.Pointer(&stmeas.choice[0]))
						subindx := (*sublst).SubCounterIndex
						//fmt.Printf("%#v\n", subindx)
						if subindx.present == 3 {
							qos := binary.LittleEndian.Uint64(subindx.choice[:])
							subtype["QoS"] = strconv.Itoa(int(qos))
						} else if subindx.present == 7 {
							var oct = (*C.struct_OCTET_STRING)(unsafe.Pointer(&subindx.choice[0]))
							plmnid := ""
							size := unsafe.Sizeof(*((*oct).buf))
							st := unsafe.Pointer(((*oct).buf))
							for j := 0; j < int((*oct).size); j++ {
								k := *(*uint8)(unsafe.Pointer(uintptr(st) + size*uintptr(j)))
								plmnid += fmt.Sprintf("%x", int(k))
							}
							subtype["plmn"] = plmnid
						} else if subindx.present == 8 {
							var snssai = (*SNSSAIID)(unsafe.Pointer(&subindx.choice[0]))
							var sst = (*snssai).sst
							sstid := ""
							size := unsafe.Sizeof(*((sst).buf))
							st := unsafe.Pointer(((sst).buf))
							for j := 0; j < int((sst).size); j++ {
								k := *(*uint8)(unsafe.Pointer(uintptr(st) + size*uintptr(j)))
								sstid += fmt.Sprintf("%x", int(k))
							}
							var sd = (*snssai).sd
							sdid := ""
							size = unsafe.Sizeof(*((sd).buf))
							st = unsafe.Pointer(((sd).buf))
							for j := 0; j < int((sd).size); j++ {
								k := *(*uint8)(unsafe.Pointer(uintptr(st) + size*uintptr(j)))
								sdid += fmt.Sprintf("%x", int(k))
							}
							subtype["sst"] = sstid
							subtype["sd"] = sdid
						}
						stmeas = *((*sublst).SubCounterValue)
					}
				}

			}
		}
		fmt.Println("Data Sent")
		queryAPI := client.QueryAPI("nms")
		// Get parser flux query result
		result, err := queryAPI.Query(context.Background(), `from(bucket:"pmsb")|> range(start: -1m)`)
		if err == nil {
			// Use Next() to iterate over query result lines
			for result.Next() {
				// read result
				fmt.Printf("row: %s\n", result.Record().String())
			}
			if result.Err() != nil {
				fmt.Printf("Query error: %s\n", result.Err().Error())
			}
		}
		client.Close()
	}
	defer func() {
		conn.Close()
	}()
}

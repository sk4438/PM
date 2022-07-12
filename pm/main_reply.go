package main

import (
	"cm/pb"

	"flag"
	"fmt"
	"log"
	"runtime"
	
	"github.com/golang/protobuf/proto"
	"github.com/nats-io/nats.go"
)

var mapofid_info=make(map[string]*pb.StreamInfo)//Map of streamid to Streaminfo

func sendReply(msg *nats.Msg) {
	map_streammessage := pb.StreamMessageRequest{}
	
	err := proto.Unmarshal(msg.Data,&map_streammessage)
	if err != nil {
		log.Fatalf("Error on unmarshal: %v", err)
		return
	}
	//After Unmarshalling,we will map the streamid and stream info and add/delete from the map depending on the request type.
	str:=map_streammessage.Type
	request_type:=(pb.StreamMessageRequest_Type_name[int32(str)])
	//If request type is "ADD"
	if	map_streammessage.Type==0 {
		st_id :=map_streammessage.Streaminfo.StreamID
		_,present:=mapofid_info[st_id]
		if(!present){
			var st_obj pb.StreamInfo
			st_obj.StreamID =	map_streammessage.Streaminfo.StreamID
			st_obj.StreamType =	map_streammessage.Streaminfo.StreamType
			st_obj.SerializationFormat =map_streammessage.Streaminfo.SerializationFormat
			
			
			st_obj.Info=map_streammessage.Streaminfo.Info
			
			mapofid_info[st_id] = &st_obj
		
			reply := pb.StreamMessageResponse{
				StreamID: st_id,
				Type:request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
		}else{
			reply := pb.StreamMessageResponse{
				StreamID: st_id + " This streamId is already present in the map.",
				Type:request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
		}
	}
	//If request type is "ADD"
	if	map_streammessage.Type==1{
		
		st_id :=map_streammessage.Streaminfo.StreamID
		
		_,present:=mapofid_info[st_id]
		if(present){
			delete(mapofid_info,st_id)
		    reply := pb.StreamMessageResponse{
				StreamID: st_id,
				Type:request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
			
		}else{
			reply := pb.StreamMessageResponse{
				StreamID: "StreamInfo with ID: " + st_id + " is not present in the map. Please enter proper StreamId",
				Type:request_type,
			}
			data, _ := proto.Marshal(&reply)
			msg.Respond(data)
			
		}
		
		
	}
	fmt.Println(mapofid_info)
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
	requestHandler();
}
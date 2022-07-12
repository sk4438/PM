package main

import (
	pb "cm/pb"
	"flag"
	"fmt"
	"log"
	"time"

	"github.com/golang/protobuf/proto"
	"github.com/nats-io/nats.go"
)

func processReply(msg *nats.Msg) {
	// struct to collect response
	reply := pb.StreamMessageResponse{}

	// unmarshal response using proto buff
	err := proto.Unmarshal(msg.Data, &reply)
	if err != nil {
		log.Fatalf("Error on unmarshal: %v", err)
		return
	}
	log.Printf("\n %v", reply.StreamID)

	fmt.Println(reply.Type)
	// //fmt.Println(reply.Mapping["121101"])
	// // print reply

}
func configstreammessages(nc *nats.Conn) {
	//adding the stream messages and sending over nats server
	performinfo := pb.PerformanceInfo{
		JobId:              "123",
		PerformanceMetrics: []string{"UEThpDl"},
		MeasObjDn:          "ManagedElement=GNB-1,GNBDUFunction=1,NRCellDU=4",
	}
	sinfo_pinfo := pb.StreamInfo_Performanceinfo{
		Performanceinfo: &performinfo,
	}
	stinfo := pb.StreamInfo{
		StreamID:            "3163904",
		StreamType:          "Performance",
		SerializationFormat: "Asn",
		Info:                &sinfo_pinfo,
	}
	addmsg := pb.StreamMessageRequest{
		Streaminfo: &stinfo,
		Type:       0,
	}
	data, _ := proto.Marshal(&addmsg)
	subject := "Streamconfig"

	// send data and wait for response
	msg, err := nc.Request(subject, data, 100000*time.Millisecond)
	if err != nil {
		log.Fatalf("Error on nats.Request", err)
		return
	}

	// process reply received
	processReply(msg)

}

func main() {
	// url to connect nats server
	var urls = flag.String("s", nats.DefaultURL, "Nat server URLs, separated by comma")

	// connect to nats
	nc, err := nats.Connect(*urls)
	if err != nil {
		log.Fatal(err)
	}
	defer nc.Close()
	log.Println("Connected to " + nats.DefaultURL)

	// request to add stream messages (send data using nats)
	configstreammessages(nc)
}

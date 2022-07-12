package main

import (
	"cm/pb"
	"encoding/xml"
	"flag"
	"fmt"
	"log"
	"runtime"

	"github.com/golang/protobuf/proto"
	"github.com/nats-io/nats.go"
	
	"gorm.io/gorm"
)

type ManagedElement struct {
	XMLName xml.Name `xml:"ManagedElement" gorm:"-"`
	Xmlns           string            `xml:"xmlns,attr"`
	Id              string            `xml:"id" gorm:"primaryKey"`
	Prioritylabel   string            `xml:"priorityLabel"`
	GNBCUCPFunctions []GNBCUCPFunction `xml:"GNBCUCPFunction" gorm:"many2many:managedelement_gnbcucpfunctions"`
}


type GNBCUCPFunction struct {
	XMLName xml.Name `xml:"GNBCUCPFunction" gorm:"-"`
	Xmlns               string                `xml:"xmlns,attr"`
	Id                  string                `xml:"id" gorm:"primaryKey"`
	Prioritylabel       string                `xml:"priorityLabel"`
	ResourceType        string                `xml:"resourceType"`
	RRMPolicyMemberLists []RRMPolicyMemberList `xml:"rRMPolicyMemberList" gorm:"many2many:gnbcucpfunction_rrmpolicymemberlists"`
	GNBId               string                `xml:"gNBId"`
	GNBIdLength         string                `xml:"gNBIdLength"`
	GNBCUName           string                `xml:"gNBCUName"`
	PLMNIds              []PLMNId              `xml:"pLMNId" gorm:"many2many:gnbcucpfunction_plmnids"`
	NRCellCUs            []NRCellCU            `xml:"NRCellCU" gorm:"many2many:gnbcucpfunction_nrcellcus"`
}

type RRMPolicyMemberList struct {
	gorm.Model
	XMLName xml.Name `xml:"rRMPolicyMemberList" gorm:"-"`
	Mcc     string   `xml:"mcc"`
	Mnc     string   `xml:"mnc"`
	Sd      string   `xml:"sd"`
	Sst     string   `xml:"sst"`
}

type PLMNId struct {
	gorm.Model
	XMLName xml.Name `xml:"pLMNId" gorm:"-" `
	Mcc     string   `xml:"mcc"`
	Mnc     string   `xml:"mnc"` 
}

type NRCellCU struct {
	XMLName xml.Name `xml:"NRCellCU" gorm:"-" `
	Xmlns         string         `xml:"xmlns,attr"`
	Id            string         `xml:"id" gorm:"primaryKey"`
	Prioritylabel string         `xml:"priorityLabel"`
	CellLocalId   string         `xml:"cellLocalId"`
	PLMNInfoLists  []PLMNInfoList `xml:"pLMNInfoList" gorm:"many2many:nrcellcu_plmninfolists"`
}

type PLMNInfoList struct {
	gorm.Model
	XMLName xml.Name `xml:"pLMNInfoList" gorm:"-"`
	Mcc     string   `xml:"mcc"`
	Mnc     string   `xml:"mnc"`
	Sd      string   `xml:"sd"`
	Sst     string   `xml:"sst"`
}

func setup_db(db *gorm.DB) {
	db.AutoMigrate(&PLMNInfoList{}, &NRCellCU{},  &PLMNId{}, &RRMPolicyMemberList{}, &GNBCUCPFunction{}, &ManagedElement{})
}

func sendReply(msg *nats.Msg) {
	reqRx := pb.RpcRequest{}
	err := proto.Unmarshal(msg.Data, &reqRx)
	if err != nil {
		log.Fatalf("Error on unmarshal: %v", err)
		return
	}
	log.Printf("Received request OranNodeIp %s msg %s", reqRx.OranNodeIp, reqRx.RpcMsg)

	// perform operations on the received data
	// database connection
	// dsn := "root:root@tcp(127.0.0.1:3306)/gorm?charset=utf8mb4&parseTime=True&loc=Local"
  	// db, err := gorm.Open(mysql.Open(dsn), &gorm.Config{})
	// if err != nil {
	// 	log.Fatal(err)
	// }

	// setup database
	//setup_db(db)

	// structure for unmarshalling
	var v ManagedElement
	// function call to unmarshal
    err = xml.Unmarshal([]byte(reqRx.RpcMsg), &v)
    if err != nil {
        fmt.Printf("error: %v", err)
        return
    }
	fmt.Println(v)
	
	// insert data
	//db.Create(&v)


	reply := pb.RpcResponse{
		OranNodeIp: reqRx.RpcMsg,
		RpcReply:   "reply OK",
	}
	data, _ := proto.Marshal(&reply)
	msg.Respond(data)
}
func requestHandler() {
	var urls = flag.String("s", nats.DefaultURL, "nats server URLs separated by comma")
	log.Printf("Waiting for a RPC request")
	nc, err := nats.Connect(*urls)
	if err != nil {
		log.Fatal(err)
	}
	nc.Subscribe("anRpc", sendReply)
	nc.Flush()
	runtime.Goexit()
}

func main() {
	requestHandler();
}

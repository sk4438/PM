package main

import (
	"fmt"
	"io/ioutil"
	"log"
	"net/http"
	"os"
	"time"

	"github.com/gorilla/websocket"
)

var filepath = "pencode"
var upgrader = websocket.Upgrader{
	ReadBufferSize:  1024,
	WriteBufferSize: 1024,
}
var filePeriod = 5 * time.Second
var lastMod = time.Now()

func websock(w http.ResponseWriter, r *http.Request) {
	conn, err := upgrader.Upgrade(w, r, nil)
	if err != nil {
		fmt.Println("Unable to upgrade to Web Sockets :", err.Error())
		return
	}
	fmt.Println("client connected")
	fileTicker := time.NewTicker(filePeriod)
	defer func() {
		fileTicker.Stop()
		conn.Close()
	}()
	for {
		select {
		case <-fileTicker.C:
			fi, err := os.Stat(filepath)
			if err != nil {
				fmt.Println("Unable to get file", err.Error())
				return
			}
			//fmt.Println(fi.ModTime(), lastMod)
			if fi.ModTime().After(lastMod) {
				lastMod = fi.ModTime()
				/*
					err = conn.WriteMessage(websocket.TextMessage, p)
					if err != nil {
						fmt.Println("Unable to send Message :", err.Error())
						return
					}
				*/
				p, er := ioutil.ReadFile(filepath)
				if er != nil {
					fmt.Println("Unable to Read file :", err.Error())
					return
				}
				err = conn.WriteMessage(websocket.BinaryMessage, p)
				if err != nil {
					fmt.Println("Unable to  send Message :", err.Error())
					return
				}
				fmt.Println("Data streamed to client")
			}
		}

	}

}

func main() {
	http.HandleFunc("/stream", websock)
	if err := http.ListenAndServe(":8080", nil); err != nil {
		log.Fatal("Unable to Listen on the port %s\n", err.Error())
	}
}

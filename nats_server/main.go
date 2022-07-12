package main

import (
	"fmt"
	"time"

	"github.com/nats-io/nats-server/v2/server"
)

func main() {
	opts := &server.Options{}

	// Initialize new server with options
	ns, err := server.NewServer(opts)

	if err != nil {
		panic(err)
	}

	// Start the server via goroutine
	go ns.Start()

	// Wait for server to be ready for connections
    if !ns.ReadyForConnections(20 * time.Second) {
        panic("not ready for connection")
    }

	fmt.Println("Nats is live!")

    // Wait for server shutdown
    ns.WaitForShutdown()
}
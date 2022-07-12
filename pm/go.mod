module client

go 1.18

require (
	github.com/golang/protobuf v1.5.2
	github.com/gorilla/websocket v1.5.0
	github.com/nats-io/nats.go v1.16.0
	gorm.io/gorm v1.23.6
	github.com/influxdata/influxdb-client-go/v2 v2.9.0 
	
)

require (
	github.com/deepmap/oapi-codegen v1.8.2 // indirect
	github.com/influxdata/influxdb-client-go/v2 v2.9.0 // indirect
	github.com/influxdata/line-protocol v0.0.0-20200327222509-2487e7298839 // indirect
	github.com/pkg/errors v0.9.1 // indirect
	golang.org/x/net v0.0.0-20211112202133-69e39bad7dc2 // indirect
	gopkg.in/yaml.v2 v2.3.0 // indirect
)

require (
	cm v0.0.0-00010101000000-000000000000
	github.com/jinzhu/inflection v1.0.0 // indirect
	github.com/jinzhu/now v1.1.4 // indirect
	github.com/nats-io/nkeys v0.3.0 // indirect
	github.com/nats-io/nuid v1.0.1 // indirect
	golang.org/x/crypto v0.0.0-20220315160706-3147a52a75dd // indirect
	google.golang.org/protobuf v1.28.0 // indirect
)

replace cm => ../cm

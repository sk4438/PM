FROM golang:1.18-alpine AS builder

RUN set -ex &&\

    apk add --no-progress --no-cache \

      gcc \

      git \

      musl-dev

# Copy the source files

RUN apk update

COPY . /home/karthik/proj/app

WORKDIR /home/karthik/proj/app

ADD ./cm ../cm


 

# Download dependency

RUN go mod download

 

RUN mkdir -p /var/log/xyz.log && touch xyz.log

 

# Build the binary

RUN CGO_ENABLED=1 GOOS=linux GOARCH=amd64 go build -a -tags musl -ldflags '-w -extldflags "-static"'  -o xyz

 

# Stage 2:

FROM alpine:latest as image

# Copy the binary from the build container

COPY --from=builder /home/karthik/proj/app/xyz /app/xyz-exe

COPY --from=builder /var/log/xyz.log /var/log/xyz.log

ENTRYPOINT ["/home/karthik/proj/app/xyz"]
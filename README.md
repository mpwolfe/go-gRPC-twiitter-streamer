# twiitter-streamer

This project is to demonstrate a Grpc streaming server and clients along with
The ability to publish topics and messages to a MQTT broker.

The streaming server will connect to twitter with the proper credentials
and a filter to start streaming tweets.
The credentials are passed to the server via the rpc clients.
The rpc client.cc can either intiate a streaming tweets or
stream tweets plus publish the tweets to a MQTT broker.
The mqtt-sub-client.c can subscribe to the broker to receive 
the tweets.

Grpc server is: twitter-streamer.go
Grpc clients are: client.go and client.cc
MQTT subscriber is: mqtt-sub-client.c

to build the twitter-stream.go:
   - protoc -I. --go_out=plugins=grpc:.  twitter/twitter.proto
   - go build twitter-streamer.go
   - go run twitter-streamer.go
   
to build the client.go:
   - protoc -I. --go_out=plugins=grpc:.  twitter/twitter.proto
   - go build client.go
   - go run client.go
   
to build the client.cc:
   - use the Makefile
   
to build the mqtt-sub-client.c
   -  gcc -o mqtt-sub-client    mqtt-sub-client.c -lpaho-mqtt3a -lpaho-mqtt3c
   
Then first run the twitter-streamer.go then the client programs

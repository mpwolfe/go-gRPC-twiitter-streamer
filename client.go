// grpc client to get tweets from grpc streamng server  - client.go
// client sends request to server with :
// consumerKey, consumerSecret, accessToker, accessSecret, filter
// the client receives a stream of tweets as a reply from the server

package main

import (
	"log"
	"context"
	"os"
	"fmt"
	
	pb "google.golang.org/grpc/examples/twitter/twitter"
	grpc "google.golang.org/grpc"
)

const (port = ":50053")

const  (	
	consumerKey     = "YOUR consumerkey"
        consumerSecret = "YOUR consumersecret"
        accessToken       = "YOUR accesstoken"
        accessSecret      = "YOUR accesssecret"

func main() {
	
	log.Println("start of gRPC client .....  client.go - ")
	fmt.Println("enter filter on command line")
	
	filter := os.Args[1]
	fmt.Println("your twitter filter is -> ", filter)
	

	conn, err := grpc.Dial(port,grpc.WithInsecure())
	if err != nil {
		log.Fatalf("gRPC dial fialed to connect to server .... %v",err)
	}

	client := pb.NewTweetServiceClient(conn)  // register for gRPC server service
	ctx := context.Background()
	
	req := &pb.TwitterRequest{   // request method
		ConsumerKey: consumerKey,
		ConsumerSecret: consumerSecret,
		AccessToken: accessToken,
		AccessSecret: accessSecret,
		Filter: filter }

	stream, err := client.GetTweet(ctx, req)   // RPC service
	if err != nil {
		log.Fatal("error client.GetTweet  -> ",err)
	}

	for {
		res, err := stream.Recv()
		if err != nil {
			// log.Fatal("stream.Recv error -> ", err)
			log.Fatal("stream.Recv error -> ", err)
		}

		log.Printf(" filter -> [%s] - [%v]\n",filter,res)
	}
}

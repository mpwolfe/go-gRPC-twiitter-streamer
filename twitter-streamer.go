//  grpc streaming server for tweets from twitter - twitter-streamer.go

//   send tweets to the client based on client authorization and search filter
//   the server will grpc stream the tweets to the client


//   client sends as a request to server with :
//   consumerKey, consumerSecret, accessToker, accessSecret, filter
//   server replies with a stream of tweets via filter

package main

import (
    "fmt"
	"log"
	"time"
	"net"
	
	// for twitter
	"github.com/ChimeraCoder/anaconda" 
	
	// for grpc
	"google.golang.org/grpc/grpclog"
	"google.golang.org/grpc"
        pb "google.golang.org/grpc/examples/twitter/twitter"
)

type server struct{}
const (port = ":50053")


// RPC service: request, stream reply
// client request with twitter authoriazation and filter. server streams twitter tweets to client Twitter_TwitterServiceServer
func (s *server) GetTweet(req *pb.TwitterRequest, stream pb.TweetService_GetTweetServer) error { // service/method Server
	grpclog.Println(" ****  TwitterService ******")

	// we need to authorize client before we can stream tweets
	grpclog.Println("consumerKey -> ", req.ConsumerKey)
	grpclog.Println("consumerSecret -> ", req.ConsumerSecret)
	grpclog.Println("accessToken -> ", req.AccessToken)
	grpclog.Println("accessSecret -> ", req.AccessSecret)
	grpclog.Println("filter -> ", req.Filter)

	if req.ConsumerKey == "" || req.ConsumerSecret == "" || req.AccessToken == "" || req.AccessSecret == "" {
		log.Fatal("error, Consumer key/secret and Access token/secret required")
	}

	// twitter setup api with authorization
	anaconda.SetConsumerKey(req.ConsumerKey)
	anaconda.SetConsumerSecret(req.ConsumerSecret)
	api := anaconda.NewTwitterApi(req.AccessToken, req.AccessSecret)

	var err error
	if api.Credentials == nil {
		log.Println("Twitter Api client has empty (nil) credentials")
		err = nil
		return err
	}

	fmt.Printf(" ** api.Credentials -> %v\n\n ", *api.Credentials)

	for {
		search_result, err := api.GetSearch(req.Filter, nil)   // search for tweets that match filter
		if err != nil {
			log.Println("error, GetSearch failed! ",err)
			return err
		}
           
		for _, tweet := range search_result.Statuses {               // send to client results of tweet search

			t,_ := tweet.CreatedAtTime()     // time of tweet
			t1 := t.String()                                      
			t2 := tweet.Text                          // the tweet
			// tweets := t1.String()
			log.Printf("filter: [%s] time: [%s] tweet: [%s]\n\n",req.Filter, t1, t2)
			TimeTweet := t1 + " " + t2

			// mpw
			b := []byte(TimeTweet)
			err := stream.Send(&pb.TwitterReply{Tweet: b})     // send the tweet and time of tweet to client
			
			if err != nil {
				log.Println("error, possible client closed connection !!")
				return err
			}
			time.Sleep(time.Millisecond * 12000)    // sleep for 10 seconds between sending results to client
		}
	}
	return err
}

// ********************************************************
/*
        consumerKey := "OvGDjC6Lehj8uRpvZogDtEbpb"
        consumerSecret := "r3A70e6ni5L1m1WVZtyjAQ6FzUwHgvczPjlJ2hWjw3Kiu1NaJs"
        accessToken := "256493456-tQuKAhfSVE16FIv6tEKm2W8zFVR6KCBlwozi1u5I"
        accessSecret := "KdQ4Ant0MML9UKxJRLHsANAgqwqWiliDQAsSeAVluDspH"
*/

func main() {
	
	log.Println("start of twitter-streamer server -- twitter-streamer .....")

	listener, err := net.Listen("tcp","localhost"+port)    // setup listener
	if err != nil {
		log.Fatalf("Server, failed on listen: %v",err)
	}
	grpcServer := grpc.NewServer()
	pb.RegisterTweetServiceServer(grpcServer, new(server))   // register the service

	log.Printf("server listening on port -> %s\n",port)
	
	if err := grpcServer.Serve(listener);  err != nil {      // listen serve client connections
		log.Fatalf("Server, failed to server: %v",err)
	}
}


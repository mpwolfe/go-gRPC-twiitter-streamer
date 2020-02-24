//  client connects to server, server streams data back to client
// client gets tweets from server, then will publish them to mosquitto broker

#include <string>
#include <stdio.h>
#include <memory>
#include <iostream>

// mqtt header file
#include "MQTTClient.h"

#include <grpcpp/grpcpp.h>
#include "twitter.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


using grpc::ClientReader;
using grpc::ClientReaderWriter;
using grpc::ClientWriter;

using Twitter::TweetService;      // the .proto service
using Twitter::TwitterRequest;
using Twitter::TwitterReply;

using namespace std;

#define ADDRESS "tcp://127.0.0.1:1883"    // address of broker
#define GrpcAddress "127.0.0.1:50053"     // Grpc service address
#define CLIENTID "twitter-id"             // mqtt
#define QOS 1                             // mqtt
#define TIMEOUT 10000L                    // mqtt


// *****************************************************************
class TwitterClient {

public:
  TwitterClient(std::shared_ptr<Channel> channel)
    : stub_(TweetService::NewStub(channel)) {}

  // tweeter credentials to connect to tweeter account, server will stream data back to client
  // we will also publish a topic/tweet to mqtt broker
  void getTweetPub(string &ckey, string &csecret, string &atoken, string &asecret, string &filter, string &pubTopic) {

    printf("getTweetPub: filter -> [%s]  topic -> [%s] \n", filter.c_str(), pubTopic.c_str());
    printf("sending request to Grpc service -> \n%s\n %s\n %s\n %s\n %s\n",ckey.c_str(),
	   csecret.c_str(),
	   atoken.c_str(),
	   asecret.c_str(),
	   filter.c_str());
      
    //  *****  mqtt stuff - begin
    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_create(&client, ADDRESS,CLIENTID, MQTTCLIENT_PERSISTENCE_NONE,NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if((rc = MQTTClient_connect(client, &conn_opts)) != 0) {
      printf("failed to connect to broker, code = %d\n", rc);
      exit(-1);
    }
    else {
      printf("client connected to broker, code = %d\n",rc);
    }

    // **** end -- mqtt stuff
    
    printf("getTweet ....\n");
    printf("sending request to service -> \n%s\n %s\n %s\n %s\n %s\n",ckey.c_str(),
	   csecret.c_str(),
	   atoken.c_str(),
	   asecret.c_str(),
	   filter.c_str());

    // data we are sending to the server
    TwitterRequest request;
    TwitterReply reply;                // container for the data we expect from the server
    ClientContext context;          // Context for the client.
    Status status;

    // request data
    request.set_consumerkey(ckey);
    request.set_consumersecret(csecret);
    request.set_accesstoken(atoken);
    request.set_accesssecret(asecret);
    request.set_filter(filter);

    // calling service with request, reply will be streaming data
    std::unique_ptr<ClientReader<TwitterReply>> reader(stub_->GetTweet(&context, request));

    printf("getTweet: records returned from server \n");
    while (reader->Read(&reply)) {
      cout << "tweet -> " << reply.tweet()  << "\n";
      char * msg = (char *) reply.tweet().c_str();
      char * topic = (char *) pubTopic.c_str();
      publishMessage(client, pubmsg,  msg, token, topic);   // pubish the tweet to mqtt broker
    }

  }

  // tweeter credentials to connect to tweeter account, server will stream data back to client
  void getTweet(string &ckey, string &csecret, string &atoken, string &asecret, string &filter) {
 
    printf("getTweet ....\n");
    printf("sending request to Grpc service -> \n%s\n %s\n %s\n %s\n %s\n",ckey.c_str(),
	   csecret.c_str(),
	   atoken.c_str(),
	   asecret.c_str(),
	   filter.c_str());

    // data we are sending to the server
    TwitterRequest request;
    TwitterReply reply;                // container for the data we expect from the server
    ClientContext context;          // Context for the client.
    Status status;

    // request data
    request.set_consumerkey(ckey);
    request.set_consumersecret(csecret);
    request.set_accesstoken(atoken);
    request.set_accesssecret(asecret);
    request.set_filter(filter);

    // calling service with request, reply will be streaming data
    std::unique_ptr<ClientReader<TwitterReply>> reader(stub_->GetTweet(&context, request));

    printf("getTweet: records returned from server \n");
    while (reader->Read(&reply)) {
      cout << "tweet -> " << reply.tweet()  << "\n";
    }

  }
  
  // publish topic/tweet  to mqtt broker
  void publishMessage(MQTTClient client, MQTTClient_message pubmsg,  char * msg,  MQTTClient_deliveryToken token, char * topic) {

    int rc;

    // pubmsg.payload = (char *) 1;
    pubmsg.payload = msg;
    pubmsg.payloadlen = strlen(msg);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
  
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
  
    //  printf("waiting for up to %d secondes for publication of %s\n"
    //	   "on topic %s for client with clientID: %s\n", (int)(TIMEOUT/1000), pubmsg.payload,topic,CLIENTID);

    rc = MQTTClient_waitForCompletion(client,token,TIMEOUT);
    printf("Message with delivery token %d delivered, result code -> %d\n",token,rc);
  }

private:
  std::unique_ptr<TweetService::Stub> stub_;

};

// *****************************************
int main(int argc, char* argv[]) {

  printf("start of Grpc  client -> client.cc \n");
  printf("enter: twitter filter or twitter filter pub/sub topic \n");

  /*
  for(int i=0; i <argc; i++)  {
    printf("args ->argc [%d]  #[%d]   [%s] \n",argc, i, argv[i]); 
  }
  */
  
  string filter;
  string topic;
  
  if (argc <  2) {
    printf("to few arguments, either filter or filter and  topic  \n");
    exit(0);
  }

  if (argc > 3) {
    printf("to many arguments ... either filter or filter and topic \n");
    exit(0);
  }

  if (argc == 2) {
    printf("filter -> [%s] \n",argv[1]);
    filter = argv[1];      // twitter filter
  } else if ( argc == 3) {
    printf("filter -> [%s] topic -> [%s] \n", argv[1], argv[2]);
    filter    = argv[1];       // twitter filter
     topic  = argv[2];     // pub/sub top
  }

  // tiwtter credentials
  string consumerKey("YOUR consumerkey");
  string consumerSecret("YOUR consumersecret");
  string accessToken("YOUR accesstoken");
  string accessSecret("YOUR accesssecret");

  // connect to Grpc server
  TwitterClient client(grpc::CreateChannel(GrpcAddress,grpc::InsecureChannelCredentials()));
  
  // call Grpc server to stream tweets
  if (argc == 1) {
    printf("calling getTweet with filter \n");
    client.getTweet(consumerKey,consumerSecret,accessToken,accessSecret,filter);
  } else {
    printf("calling getTweetPub with filter and pub/sub topic\n");
    client.getTweetPub(consumerKey,consumerSecret,accessToken,accessSecret,filter,topic);
  }

  

  return 0;
}

// asynchronous subscription example 
//   gcc -o mqtt-sub-client    mqtt-sub-client.c -lpaho-mqtt3a -lpaho-mqtt3c


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#define ADDRESS "tcp://127.0.0.1:1883"
#define CLIENTID "clientsub1"
#define QOS 1
#define TIMEOUT 10000L

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *contxt, MQTTClient_deliveryToken dt) {

  printf("message delivered....\n"); 
  printf("delivered: Message with token value %d delivery confired\n", dt);
  deliveredtoken = dt;
}

// callback function
int msgarrived(void *context, char *topic, int topicLen, MQTTClient_message *message) {
  printf("msgarrived.../n");

  int i;
  char *ptr;

  printf("Message arrived: topic: [%s]\n",topic);
  printf("message: ");

  ptr = message->payload;
  for(i=0; i < message->payloadlen; i++) {
    putchar(*ptr++);
  }
  putchar('\n');
  MQTTClient_freeMessage(&message);
  MQTTClient_free(topic);

  return 1;
}

void connlost(void *context, char *cause) {
  printf("\nConnection lost\n");
  printf("          cause: %s\n",cause);
}


int  main(int argc, char *argv[]) {
  printf("start of mqtt-sub-client...\n");

  if (argc < 2) {
	  printf("to few arguments, provide a topic .....\n");
	  exit(0);
  }


  MQTTClient client;
  MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
  int rc;
  int ch;

  MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
  conn_opts.keepAliveInterval = 20;
  conn_opts.cleansession = 1;

  // set the callbacks
  MQTTClient_setCallbacks(client,NULL,connlost, msgarrived , delivered);

  if (rc = MQTTClient_connect(client, &conn_opts) != MQTTCLIENT_SUCCESS) {
    printf("failed to connect to broker, return code %d\n",rc);
    exit(1);
  }

  char * topic = (char *) malloc(strlen(argv[1]) + 1);
  strcpy(topic,argv[1]);

  printf("client connected to broker\n");
  printf("subscribing to topic: %s client: %s using Qos: %d\n", topic, CLIENTID, QOS);

  MQTTClient_subscribe(client, topic, QOS);
  
  do {
    ch = getchar();
  } while(ch!= 'q');

  MQTTClient_disconnect(client, 10000);
  MQTTClient_destroy(&client);

  return rc;
}
  

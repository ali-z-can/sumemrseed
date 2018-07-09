#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTClient.h"

#define ADDRESS     "tcp://192.168.2.192"
#define CLIENTID    "doest2hismatter"
#define TOPIC       "test"
#define PAYLOAD     "gelime la"
#define QOS         1
#define TIMEOUT     10000L

int main(int argc, char* argv[])
{

    MQTTClient client;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;
    int rc;

    MQTTClient_message* m = NULL;
    MQTTClient_create(&client, ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);

    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    int x;
    x = strlen(TOPIC);
   // pubmsg.qos = QOS;
    //pubmsg.retained = 0;	
	char* topicc; 
	
	char TOPC[100];
	fprintf(stderr,"Enter topic\n");
	gets(TOPC);

 //fprintf(stderr,"here\n");
    MQTTClient_subscribe(client,TOPC,QOS);
while(1){
again:
   MQTTClient_receive(client,&topicc,&x,&m,5000);
  //  MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
	if(m == NULL){
		goto again;
	}
	char *cc;
	cc = malloc(m->payloadlen);
	memcpy(cc,m->payload,m->payloadlen);
	cc[m->payloadlen] = '\0';
    printf("%s\n",(cc));
	free(cc);
}
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);

    //printf("Message with delivery token %d delivered\n", token);
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

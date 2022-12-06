#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"

#define BROKER_ADDRESS     "tcp://10.0.0.101:1883"
#define USERNAME "aluno"
#define PASSWORD "@luno*123"

#define CLIENTID    "R-TP04/G03"
#define QOS1         1
#define QOS2         2
#define TIMEOUT     5000L

// Comandos de requisição
#define GET_ANALOG_INPUT_VALUE "0x04"
#define GET_DIGITAL_INPUT_VALUE "0x05"
#define SET_ON_NODEMCU_LED "0x06"
#define SET_OFF_NODEMCU_LED "0x07"
#define GET_NODE_CONNECTION_STATUS "0x08"

// Definições dos tópicos
#define ANALOG_SENSOR "tp04/g03/node/analog-sensor/value"
#define DIGITAL_SENSOR "tp04/g03/node/digital-sensor/value"
#define REQUEST "tp04/g03/mqtt/request/value"
#define RESPONSE "tp04/g03/mqtt/response/value"
#define ADDRESS "tp04/g03/node/digital-sensor/address"
#define NODE_CONNECTION_STATUS "tp04/g03/node/status"

// Definições dos endereços dos sensores digitais
#define ADDR_D0 "0xD0"
#define ADDR_D1 "0xD1"
#define ADDR_D2 "0xD2"
#define ADDR_D3 "0xD3"
#define ADDR_D4 "0xD4"
#define ADDR_D5 "0xD5"
#define ADDR_D6 "0xD6"
#define ADDR_D7 "0xD7"

volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;

void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    int i;
    char* payloadptr;

    printf("Mensagem recebida\n");
    printf("tópico: %s\n", topicName);
    printf("mensagem: ");

    payloadptr = message->payload;
    for(i=0; i<message->payloadlen; i++)
    {
        putchar(*payloadptr++);
    }
    putchar('\n');
	
    if(strcmp(topicName,NODE_CONNECTION_STATUS) == 0){
    	if(strcmp(msg,"0x200") == 0){
		printf("Node online");
	}
    }
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause)
{
    printf("\nConexão perdida\n");
    printf("     cause: %s\n", cause);
}

void send(char* topic, char* payload) {
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    pubmsg.payload = payload;
    pubmsg.payloadlen = strlen(pubmsg.payload);
    pubmsg.qos = QOS2;
    pubmsg.retained = 0;
    MQTTClient_deliveryToken token;
    MQTTClient_publishMessage(client, topic, &pubmsg, &token);
    MQTTClient_waitForCompletion(client, token, TIMEOUT);
}

int main(int argc, char* argv[])
{
    int rc;
    int ch;
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;

    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    
    MQTTClient_create(&client, BROKER_ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Pressione Q<Enter> para sair\n\n");
    MQTTClient_subscribe(client, RESPONSE, QOS2);
    MQTTClient_subscribe(client, ANALOG_SENSOR, QOS2);
    MQTTClient_subscribe(client, DIGITAL_SENSOR, QOS2);
    do
    {
        ch = getchar();
        switch(ch){
		case '1':
			send(REQUEST,SET_ON_NODEMCU_LED);
			break;
		case '2':
			send(REQUEST,SET_OFF_NODEMCU_LED);
			break;
		case '3':
			send(REQUEST,GET_ANALOG_INPUT_VALUE);
			break;
		case '4':
			send(ADDRESS,ADDR_D0);
			send(REQUEST,GET_DIGITAL_INPUT_VALUE);
			break;

        }
        
    } while(ch!='Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

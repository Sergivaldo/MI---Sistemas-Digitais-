#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <wiringPi.h>
#include <lcd.h>

// configurações do cliente mqtt
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

#define ADDR_D0 "D0"
#define ADDR_D1 "D1"
#define ADDR_D2 "D2"
#define ADDR_D3 "D3"
#define ADDR_D4 "D4"
#define ADDR_D5 "D5"
#define ADDR_D6 "D6"
#define ADDR_D7 "D7"

//Pinos do lcd
#define LCD_RS  13               //Register select pin
#define LCD_E   18               //Enable Pin
#define LCD_D4  21               //Data pin D4
#define LCD_D5  24               //Data pin D5
#define LCD_D6  26               //Data pin D6
#define LCD_D7  27               //Data pin D7

//Pinos dos Botões 
#define BUTTON_1 19
#define BUTTON_2 23
#define BUTTON_3 25

int lcd;

// Controles de navegação dos menus
int currentMenuOption = 0;
int currentMenuSensorOption = 1;
int currentMenuIntervalOption = 1;

// Flags de parada
int stopLoopMainMenu = 0;
int stopLoopConfigMenu = 0;
int stopLoopSensorsMenu = 0;
int stopLoopSetTimeInterval = 0;
int stopLoopSetTimeUnit = 0;
// Intervalo de Tempo
int timeInterval = 1;
char timeUnit = 's';
int timeUnitAux = 0;

// Led
int ledState = 0;

// Funções do Cliente MQTT

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

// Funções do menu lcd

// Debounce
void isPressed(int btt, int (*function)(int, int), int* controller, int minMax){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			*controller = function(*controller, minMax);
			while(digitalRead(btt) == 0);
		}
	} 	
}

void enter(int btt,void (*function)(void)){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			function();
			while(digitalRead(btt) == 0);
		}
	} 
}

void close(int btt,int* stopFlag){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			*stopFlag = 1;
			while(digitalRead(btt) == 0);
		}
	} 
}

// Incrementa uma variável se não tiver atingido seu valor máximo
int increment(int valueController, int max){
	if(valueController < max){
		valueController++;
	}
	return valueController;
}

// Decrementa uma variável se não tiver atingido seu valor mínimo
int decrement(int valueController, int min){
	if(valueController > min){
		valueController--;
	}
	return valueController;
}

//Encerrar o sistema
void finishMessage(){
	lcdClear(lcd);
	lcdPuts(lcd,"     SAINDO     ");
	delay(1500);
	lcdClear(lcd);
}

// Ajustar o intervalo de tempo em que os sensores serão atualizados
void setTimeInterval(){
	lcdClear(lcd);
	lcdPuts(lcd,"   INTERVALO    ");
	while(!stopLoopSetTimeInterval){
		lcdPosition(lcd,0,1);
		lcdPrintf(lcd,"1%c",timeUnit);
		for(int i=1;i <=timeInterval; i++){
			lcdPutchar(lcd,0xFF);
		}
		
		for(int aux = timeInterval; aux < 10; aux++){
			lcdPuts(lcd," ");
		}
		
		lcdPrintf(lcd,"10%c ",timeUnit);
		isPressed(BUTTON_2,increment,&timeInterval,10);
		isPressed(BUTTON_1,decrement,&timeInterval,1);
		close(BUTTON_3,&stopLoopSetTimeInterval);
	}
	stopLoopSetTimeInterval = 0;
	lcdClear(lcd);
}

void setLedState(){
	if(ledState == 1){
		ledState = 0;
		send(REQUEST,SET_OFF_NODEMCU_LED);
	}else{
		ledState = 1;
		send(REQUEST,SET_ON_NODEMCU_LED);
	}
}
	
void setTimeUnit(){
	lcdClear(lcd);
	while(!stopLoopSetTimeUnit){
		lcdHome(lcd);
		
		if(timeUnitAux == 0){
			timeUnit = 's';
		}else if(timeUnitAux == 1){
			timeUnit = 'm';
		}else{
			timeUnit = 'h';
		}
		
		if(timeUnit == 's'){
			lcdPuts(lcd,"UNID. TEMPO: SEG");
		}else if(timeUnit == 'm'){
			lcdPuts(lcd,"UNID. TEMPO: MIN");
		}else{
			lcdPuts(lcd,"UNID. TEMPO: HOR");
		}
		lcdPosition(lcd,0,1);
		lcdPuts(lcd,"                ");	
		isPressed(BUTTON_2,increment,&timeUnitAux,2);
		isPressed(BUTTON_1,decrement,&timeUnitAux,0);
		close(BUTTON_3,&stopLoopSetTimeUnit);	
	}
	lcdClear(lcd);
	stopLoopSetTimeUnit = 0;
}

void configMenu(){
	while(!stopLoopConfigMenu){
		switch(currentMenuIntervalOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"    INTERVALO   ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,3);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,1);
				enter(BUTTON_3,setTimeInterval);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"   UNID. TEMPO  ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,3);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,1);
				enter(BUTTON_3,setTimeUnit);
				break;
			case 3:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,3);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,1);
				close(BUTTON_3,&stopLoopConfigMenu);
				break;
		}
	}
	stopLoopConfigMenu = 0;
	lcdClear(lcd);
}

void sensorsMenu(){
	while(!stopLoopSensorsMenu){
		switch(currentMenuSensorOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D0   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D1   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 3:
			    lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D2   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D3   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D4   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D5   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D6   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 8:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D7   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 9:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				close(BUTTON_3,&stopLoopSensorsMenu);
				break;
		}	
	}
	stopLoopSensorsMenu = 0;
	lcdClear(lcd);
}

void mainMenu(){
	while(!stopLoopMainMenu){
		switch(currentMenuOption){
			case 0:
				lcdHome(lcd);
				lcdPuts(lcd,"     MI - SD    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"   Problema 3   ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				break;
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"LEITURA:        ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR DIGITAL  ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				enter(BUTTON_3,sensorsMenu);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"LEITURA:        ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR ANALOGICO");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				break;
			case 3:
				lcdHome(lcd);
				if(ledState == 0){
					lcdPrintf(lcd,"LED:    ON  %cOFF",0xFF);
				}else{
					lcdPrintf(lcd,"LED:    ON%c  OFF",0xFF);
				}
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				enter(BUTTON_3,setLedState);
				break;

			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"  CONFIGURACOES ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				enter(BUTTON_3,configMenu);
				break;

			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				close(BUTTON_3,&stopLoopMainMenu);
				break;

		}
	}
}

int main(int argc, char* argv[])
{
    int rc;
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

    printf("Conexão estabelecida\n\n");
    MQTTClient_subscribe(client, RESPONSE, QOS2);
    MQTTClient_subscribe(client, ANALOG_SENSOR, QOS2);
    MQTTClient_subscribe(client, DIGITAL_SENSOR, QOS2);
    
    wiringPiSetup();
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
	
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    pinMode(BUTTON_3,INPUT);
	
    mainMenu();
    finishMessage();
    
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include <wiringPi.h>
#include <lcd.h>
#include <pthread.h>
#include <time.h>

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
#define GET_LED_VALUE "0x09"

//Comandos de resposta
#define NODE_SITUATION "0x200"

// Definições dos tópicos de comunicação com a ORANGE PI
#define RESPONSE "tp04/g03/mqtt/response/value"              //Respostas da NODE

// Definições dos tópicos de comunicação com a NODE
#define REQUEST "tp04/g03/mqtt/request/value"                //Enviar algum comando de requisição
#define ANALOG_SENSOR "tp04/g03/node/analog-sensor/value"    //Receber a medição analogico
#define DIGITAL_SENSOR "tp04/g03/node/digital-sensor/value"  //Receber as medições digitais       
#define NODE_CONNECTION_STATUS "tp04/g03/node/status"        //Receber o status da conexão com a NODE
#define ACTIVE_SENSORS "tp04/g03/node/active-sensors"        //Enviar a configuração dos sensores digitais
#define TIME_INTERVAL "tp04/g03/node/time-interval"          //Enviar o intervalo de atualização dos sensores

// Definções dos topicos de comunicação com o APP
#define APP_CONNECTION_STATUS "tp04/g03/app/status"          //Receber o status de conexão com o APP

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
int currentMenuAnalogSensorOption = 1;
int currentMenuIntervalOption = 1;
int currentUsedSensorsOption = 1;
int currentConnectionStatusOption = 1;
int currentHistoryMenuOption = 1;
int currentHistoryDigitalSensorOption = 1;

// Flags de parada
int stopLoopMainMenu = 0;
int stopLoopConfigMenu = 0;
int stopLoopDigitalSensorsMenu = 0;
int stopLoopAnalogSensorsMenu = 0;
int stopLoopSetTimeInterval = 0;
int stopLoopSetTimeUnit = 0;
int stopLoopSetUsedSensors = 0;
int stopLoopConnectionStatusMenu = 0;
int stopLoopHistoryMenu = 0;
int stopLoopHistoryDigitalSensors = 0;

// Intervalo de Tempo
int timeInterval = 1;
char timeUnit = 's';
int timeUnitAux = 0;
long int timeSeconds = 0;

// Flags de conexão
int connectionNode = -1;
int connectionApp = -1;
int testConnection = 0;

typedef struct{
	char values[16];
	char time[10];
}History;

// Flag de estado do led
int ledState = 0;

// Flag dos sensores digitais em uso
char activeSensors[] = {'1','1','0','0','0','0','0','0'};

char lastValueDigitalSensors[] = {'n','n','n','n','n','n','n','n'};
char timeLastValueDigitalSensors[10];
char currentTime[10];
char* bufDigitalValues;
char analogValue[5];

History historyList[10];
int nextHistory = 0;

// Funções do Cliente MQTT

volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;

// Threads
pthread_t stats_connection;
pthread_t time_now;

void updateHistory(int nextHistory){
	if(nextHistory < 9){
		sprintf(historyList[nextHistory].values,"%c,%c,%c,%c,%c,%c,%c,%c",lastValueDigitalSensors[0],lastValueDigitalSensors[1],lastValueDigitalSensors[2],lastValueDigitalSensors[3],lastValueDigitalSensors[4],lastValueDigitalSensors[5],lastValueDigitalSensors[6],lastValueDigitalSensors[7]);
		strcpy(historyList[nextHistory].time,timeLastValueDigitalSensors);
		nextHistory++;
	}
}

void setDigitalValueSensors(){
  char * substr =  malloc(50);
  substr = strtok(bufDigitalValues, ",");
   // loop through the string to extract all other tokens
  while( substr != NULL ) {
      char *pinName = malloc(2);
      strncpy(pinName, substr,2);
      int index = ((int)pinName[1]) - ((int)'0');

      char *pinValue = malloc(2);
      strncpy(pinValue, substr+3,1);
      lastValueDigitalSensors[index] = *pinValue;

      substr = strtok(NULL, ",");
   }
   strcpy(timeLastValueDigitalSensors, currentTime);
   updateHistory(nextHistory);
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

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    char* msg = message -> payload;
	if(strcmp(topicName,RESPONSE) == 0){
    		if(strcmp(msg,"0x03") == 0){
			ledState = 1;
		}
		else if(strcmp(msg,"0x04") == 0){
			ledState = 0;
		}
		else if(strcmp(msg,"0x200") == 0){
			testConnection = 0;
		}
		else if(strcmp(msg,"0xFA") == 0){
			printf("Intervalo mudado");
		}
    }else if(strcmp(topicName,DIGITAL_SENSOR) == 0){
    	bufDigitalValues = msg;
    	setDigitalValueSensors();
    }else if(strcmp(topicName,ANALOG_SENSOR) == 0){
    	strcpy(analogValue,msg);
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


// Funções do menu lcd

// Debounce
void isPressed(int btt, int (*function)(int, int, int), int* controller, int max,int min){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			*controller = function(*controller, max, min);
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

void toggleState(int btt,int index){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			if(activeSensors[index] == '1'){
				activeSensors[index] = '0';
			}else{
				activeSensors[index] = '1';
			}
			
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

void closeSetSensors(int btt,int* stopFlag,void (*function)(void)){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			*stopFlag = 1;
			while(digitalRead(btt) == 0);
		}
		function();
	} 
}

// Incrementa uma variável se não tiver atingido seu valor máximo
int increment(int valueController, int max, int min){
	valueController++;
	if(valueController > max){
		valueController = min;
	}
	
	return valueController;
}

// Decrementa uma variável se não tiver atingido seu valor mínimo
int decrement(int valueController, int max, int min){
	valueController--;
	if(valueController < min){
		valueController = max;
	}
	
	return valueController;
}

//Encerrar o sistema
void finish(){
	lcdClear(lcd);
	lcdPuts(lcd,"     SAINDO     ");
	delay(1500);
	lcdClear(lcd);
	pthread_join(stats_connection,NULL);
	pthread_join(time_now,NULL);
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
		isPressed(BUTTON_2,increment,&timeInterval,10,1);
		isPressed(BUTTON_1,decrement,&timeInterval,10,1);
		close(BUTTON_3,&stopLoopSetTimeInterval);
	}
	if(timeUnit == 'h'){
		timeSeconds = timeInterval * 3600;
	} else if(timeUnit == 'm'){
		timeSeconds = timeInterval * 60;
	}else{
		timeSeconds = timeInterval; 
	}
	stopLoopSetTimeInterval = 0;
	lcdClear(lcd);
}

void setLedState(){
	if(ledState == 1){
		send(REQUEST,SET_OFF_NODEMCU_LED);
	}else{
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
		isPressed(BUTTON_2,increment,&timeUnitAux,2,0);
		isPressed(BUTTON_1,decrement,&timeUnitAux,2,0);
		close(BUTTON_3,&stopLoopSetTimeUnit);	
	}
	lcdClear(lcd);
	stopLoopSetTimeUnit = 0;
}

void sendActiveSensors(){
	char str[50];
	sprintf(str,"D0-%c,D1-%c,D2-%c,D3-%c,D4-%c,D5-%c,D6-%c,D7-%c",activeSensors[0],activeSensors[1],activeSensors[2],activeSensors[3],activeSensors[4],activeSensors[5],activeSensors[6],activeSensors[7]);
	send(ACTIVE_SENSORS,str);
}

void statusSensorMessage(int index){
	if(activeSensors[index] == '1'){
		lcdPuts(lcd,"ATIVADO         ");
	}else{
		lcdPuts(lcd,"DESATIVADO      ");
	}
}

void valueDigitalSensor(int index){

	if(activeSensors[index] == '1'){
		lcdPrintf(lcd,"VALOR:%c         ",lastValueDigitalSensors[index]);
	}else{
		lcdPuts(lcd,"DESATIVADO      ");
	}
}

void setUsedSensors(){
	while(!stopLoopSetUsedSensors){
		switch(currentUsedSensorsOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D0:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(0);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 0);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D1:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(1);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 1);
				break;
			case 3:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D2:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(2);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 2);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D3:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(3);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 3);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D4:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(4);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 4);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D5:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(5);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 5);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D6:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(6);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 6);
				break;
			case 8:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D7:      ");
				lcdPosition(lcd,0,1);
				statusSensorMessage(7);
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				toggleState(BUTTON_3, 7);
				break;
			case 9:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentUsedSensorsOption,9,1);
				isPressed(BUTTON_1,decrement,&currentUsedSensorsOption,9,1);
				close(BUTTON_3,&stopLoopSetUsedSensors);
				break;
		}
	}
	stopLoopSetUsedSensors = 0;
	currentUsedSensorsOption = 1;
	lcdClear(lcd);
}

void configMenu(){
	while(!stopLoopConfigMenu){
		switch(currentMenuIntervalOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"    INTERVALO   ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,4,1);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,4,1);
				enter(BUTTON_3,setTimeInterval);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"   UNID. TEMPO  ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,4,1);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,4,1);
				enter(BUTTON_3,setTimeUnit);
				break;
			case 3:
				lcdHome(lcd);
				lcdPuts(lcd,"  ESPECIFICAR   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSORES ATIVOS ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,4,1);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,4,1);
				enter(BUTTON_3,setUsedSensors);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuIntervalOption,4,1);
				isPressed(BUTTON_1,decrement,&currentMenuIntervalOption,4,1);
				close(BUTTON_3,&stopLoopConfigMenu);
				break;
		}
	}
	sendActiveSensors();
	char buf[10];
	sprintf(buf,"%ld",timeSeconds);
	send(TIME_INTERVAL,buf);
	
	stopLoopConfigMenu = 0;
	currentMenuIntervalOption = 1;
	lcdClear(lcd);
}

void analogSensorsMenu(){
	lcdClear(lcd);
	while(!stopLoopAnalogSensorsMenu){
		switch(currentMenuAnalogSensorOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR A0   ");
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"    VALOR:%s  ",analogValue);
				isPressed(BUTTON_2,increment,&currentMenuAnalogSensorOption,2,1);
				isPressed(BUTTON_1,decrement,&currentMenuAnalogSensorOption,2,1);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuAnalogSensorOption,2,1);
				isPressed(BUTTON_1,decrement,&currentMenuAnalogSensorOption,2,1);
				close(BUTTON_3,&stopLoopAnalogSensorsMenu);
				break;
		}
	}
	
	stopLoopAnalogSensorsMenu = 0;
	currentMenuAnalogSensorOption = 1;
	lcdClear(lcd);
}

void digitalSensorsMenu(){
	while(!stopLoopDigitalSensorsMenu){
		switch(currentMenuSensorOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D0       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(0);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D1       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(1);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 3:
			    lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D2       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(2);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D3       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(3);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D4       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(4);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D5       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(5);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D6       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(6);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 8:
				lcdHome(lcd);
				lcdPuts(lcd,"SENSOR D7       ");
				lcdPosition(lcd,0,1);
				valueDigitalSensor(7);
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				break;
			case 9:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,9,1);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,9,1);
				close(BUTTON_3,&stopLoopDigitalSensorsMenu);
				break;
		}	
	}
	stopLoopDigitalSensorsMenu = 0;
	currentMenuSensorOption = 1;
	lcdClear(lcd);
}

void connectionStatusMenu(){
    while(!stopLoopConnectionStatusMenu){
        switch(currentConnectionStatusOption){
            case 1:
                		lcdHome(lcd);
				lcdPuts(lcd,"NODEMCU         ");
				lcdPosition(lcd,0,1);
				if(connectionNode == -1){
				    lcdPuts(lcd,"STATUS: OFFLINE");
				}else if(connectionNode == 1){
				    lcdPuts(lcd,"STATUS: ONLINE ");
				}
				isPressed(BUTTON_2,increment,&currentConnectionStatusOption,3,1);
				isPressed(BUTTON_1,decrement,&currentConnectionStatusOption,3,1);
				break;
            case 2:
                		lcdHome(lcd);
				lcdPuts(lcd,"APP         ");
				lcdPosition(lcd,0,1);
				if(connectionApp == -1){
				    lcdPuts(lcd,"STATUS: OFFLINE");
				}else if(connectionApp == 1){
				    lcdPuts(lcd,"STATUS: ONLINE ");
				}
				isPressed(BUTTON_2,increment,&currentConnectionStatusOption,3,1);
				isPressed(BUTTON_1,decrement,&currentConnectionStatusOption,3,1);
				break;
            case 3:
                		lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentConnectionStatusOption,3,1);
				isPressed(BUTTON_1,decrement,&currentConnectionStatusOption,3,1);
				close(BUTTON_3,&stopLoopConnectionStatusMenu);
				break;
        }
    }
    stopLoopConnectionStatusMenu = 0;
    currentConnectionStatusOption = 1;
    lcdClear(lcd);
}

void historyDigitalSensors(){
	lcdClear(lcd);
	while(!stopLoopHistoryDigitalSensors){
		switch(currentHistoryDigitalSensorOption){
			case 1:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[0].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[0].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 2:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[1].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[1].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 3:
			    lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[2].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[2].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 4:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[3].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[3].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 5:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[4].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[4].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 6:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[5].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[5].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 7:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[6].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[6].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 8:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[7].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[7].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 9:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[8].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[8].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
			case 10:
				lcdHome(lcd);
				lcdPrintf(lcd,"%s ",historyList[9].values);
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"%s        ",historyList[9].time);
				isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,0);
				isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,0);
				close(BUTTON_3, &stopLoopHistoryDigitalSensors);
				break;
		}
	}
	stopLoopHistoryDigitalSensors = 0;
}

void historyAnalogSensors(){
	
}

void historyMenu(){
	while(!stopLoopHistoryMenu){
		switch(currentHistoryMenuOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"HISTORICO:      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR DIGITAL  ");
				isPressed(BUTTON_2,increment,&currentHistoryMenuOption,3,1);
				isPressed(BUTTON_1,decrement,&currentHistoryMenuOption,3,1);
				enter(BUTTON_3,historyDigitalSensors);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"HISTORICO:      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR ANALOGICO");
				isPressed(BUTTON_2,increment,&currentHistoryMenuOption,3,1);
				isPressed(BUTTON_1,decrement,&currentHistoryMenuOption,3,1);
				break;
			case 3:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentHistoryMenuOption,3,1);
				isPressed(BUTTON_1,decrement,&currentHistoryMenuOption,3,1);
				close(BUTTON_3,&stopLoopHistoryMenu);
				break;
		}
	}
	stopLoopHistoryMenu = 0;
	currentHistoryMenuOption = 1;
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
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				break;
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"LEITURA:        ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR DIGITAL  ");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,digitalSensorsMenu);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"LEITURA:        ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR ANALOGICO");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,&analogSensorsMenu);
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
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,setLedState);
				break;

			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"  CONFIGURACOES ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,configMenu);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"     STATUS     ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"  DAS CONEXOES  ");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,connectionStatusMenu);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"    HISTORICO   ");
				lcdPosition(lcd,0,1);
				lcdPrintf(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				enter(BUTTON_3,historyMenu);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuOption,7,1);
				isPressed(BUTTON_1,decrement,&currentMenuOption,7,1);
				close(BUTTON_3,&stopLoopMainMenu);
				break;

		}
	}
}

void *checkConnections(void *arg){

	while (1){
		testConnection = 1;
		send(REQUEST, GET_NODE_CONNECTION_STATUS);
		delay(500);
		if(testConnection == 1){
			connectionNode = -1;
		}else{
			connectionNode = 1;
		}
		
	}
}

void *getTime(void *arg){
	 //ponteiro para struct que armazena data e hora  
	  struct tm *timeNow;     
	  
	  //variável do tipo time_t para armazenar o tempo em segundos  
	  time_t seconds;
	  while(1){
	      //obtendo o tempo em segundos  
	      time(&seconds);   
	  
	      //para converter de segundos para o tempo local  
	      //utilizamos a função localtime  
	      timeNow = localtime(&seconds);  
	      
	      char buf_hour[3];
	      char buf_min[3];
	      char buf_sec[3];
	      
	      timeNow -> tm_hour <=9 ? sprintf(buf_hour,"0%d",(timeNow -> tm_hour)):sprintf(buf_hour,"%d",(timeNow -> tm_hour));
	    
	      timeNow -> tm_min <=9 ? sprintf(buf_min,"0%d",(timeNow -> tm_min)):sprintf(buf_min,"%d",(timeNow -> tm_min));
	    
	      timeNow -> tm_sec <=9 ? sprintf(buf_sec,"0%d",(timeNow -> tm_sec)):sprintf(buf_sec,"%d",(timeNow -> tm_sec));
	      
	      sprintf(currentTime,"%s:%s:%s",buf_hour,buf_min,buf_sec);
      
  }
}

int main(int argc, char* argv[])
{
    int rc;
    wiringPiSetup();
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    pinMode(BUTTON_3,INPUT);
 
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;
    
    
    MQTTClient_create(&client, BROKER_ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 1500;
    conn_opts.cleansession = 1;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Conexão estabelecida\n\n");
    MQTTClient_subscribe(client, RESPONSE, QOS2);
    MQTTClient_subscribe(client, ANALOG_SENSOR, QOS2);
    MQTTClient_subscribe(client, DIGITAL_SENSOR, QOS2);
    send(REQUEST,GET_NODE_CONNECTION_STATUS);
    pthread_create(&stats_connection, NULL, checkConnections, NULL);
    pthread_create(&time_now, NULL, getTime, NULL);
    
    mainMenu();
    finish();

    return rc;
 }

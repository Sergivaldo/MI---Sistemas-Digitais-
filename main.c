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

// Comandos de requisição da
#define GET_ANALOG_INPUT_VALUE "0x04"
#define GET_DIGITAL_INPUT_VALUE "0x05"
#define SET_ON_NODEMCU_LED "0x06"
#define SET_OFF_NODEMCU_LED "0x07"
#define GET_NODE_CONNECTION_STATUS "0x08"
#define GET_LED_VALUE "0x09"
#define GET_APP_CONNECTION_STATUS "0x0A"

//Comandos de resposta
#define NODE_SITUATION "0x200"
#define APP_SITUATION "0x201"

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
#define RASP_RESPONSE "tp04/g03/mqtt/response/rasp/value"
#define RASP_REQUEST "tp04/g03/mqtt/request/rasp/value"

#define APP_CONNECTION_STATUS "tp04/g03/app/status"
#define APP_REQUEST "tp04/g03/mqtt/request/app/value"
#define APP_RESPONSE "tp04/g03/mqtt/response/app/value"
#define APP_TIME_INTERVAL "tp04/g03/app/time-interval"
#define APP_ACTIVE_SENSORS "tp04/g03/app/active-sensors"
#define APP_ANALOG_SENSOR "tp04/g03/app/analog-sensor/value"
#define APP_DIGITAL_SENSOR "tp04/g03/app/digital-sensor/value"
#define APP_HISTORY "tp04/g03/app/history"

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

// Descritor de arquivo do lcd
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
int currentHistoryAnalogSensorOption = 1;

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
int stopLoopHistoryAnalogSensors = 0;

// Intervalo de Tempo
int timeInterval = 5;
char timeUnit = 's';
int timeUnitAux = 0;
long int timeSeconds = 0;

// Flags de conexão
int connectionNode = -1;
int connectionApp = -1;

// Flags de teste
int testConnectionNode = 0;
int testConnectionApp = 0;

// Flags de solicitação
int appSolicitationCounter = 0;
char appSolicitation;

// Estrutura do histórico
typedef struct{
        char values[16];
        char time[10];
}History;

// Flag de estado do led
int ledState = 0;

// Flag dos sensores digitais em uso
char activeSensors[] = {'1','1','0','0','0','0','0','0'};

// Valores das últimas medições dos sensores digitais
char lastValueDigitalSensors[] = {'n','n','n','n','n','n','n','n'};
// Horário das últimas medições dos sensores digitais
char timeLastValueDigitalSensors[10];

// Buffers de recebimento dos valores dos sensores
char* bufDigitalValues;
char* bufAnalogValue;

// Valores das últimas medições dos sensores analógicos
char lastAnalogValue[5];
// Horários das últimas medições dos sensores analógicos
char timeLastValueAnalogSensors[10];

// Históricos das dez ultimas medições dos sensores digitais
History historyListDigital[10];

// Históricos das dez ultimas medições dos sensores analógicos
History historyListAnalog[10];

// Índice do próximo histórico dos sensores digitais
int nextHistoryDigital = 0;

// Índice do próximo histórico dos sensores analógicos
int nextHistoryAnalog = 0;

// Flag que indica se chegou uma nova informação
int newInfo = 0;

// Estruturas do Cliente MQTT
volatile MQTTClient_deliveryToken deliveredtoken;
MQTTClient client;

// Threads
pthread_t stats_connection;
pthread_t checkSolicitation;

// Função que pega o tempo atual
void getTime(char buf[]){
        //ponteiro para struct que armazena data e hora
        struct tm *timeNow;

        //variável do tipo time_t para armazenar o tempo em segundos
        time_t seconds;

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

        sprintf(buf,"%s:%s:%s",buf_hour,buf_min,buf_sec);

}

// Função para atualizar o histórico dos sensores digitais
void updateHistoryDigital(int next){
	
	// Copia o valor da ultima medição dos sensores digitais para o próximo histórico
        if(next < 10){
                sprintf(historyListDigital[next].values,"%c,%c,%c,%c,%c,%c,%c,%c",lastValueDigitalSensors[0],lastValueDigitalSensors[1],lastValueDigitalSensors[2],lastValueDigitalSensors[3],lastValueDigitalSensors[4],lastValueDigitalSensors[5],lastValueDigitalSensors[6],lastValueDigitalSensors[7]);
                strcpy(historyListDigital[next].time,timeLastValueDigitalSensors);
                nextHistoryDigital++;
	// Se o array de historicos possuir dez históricos armazenados, move os últimos nove históricos com a função memcpy
        }else{
                memcpy(historyListDigital, &historyListDigital[1], 9*sizeof(*historyListDigital));
                sprintf(historyListDigital[9].values,"%c,%c,%c,%c,%c,%c,%c,%c",lastValueDigitalSensors[0],lastValueDigitalSensors[1],lastValueDigitalSensors[2],lastValueDigitalSensors[3],lastValueDigitalSensors[4],lastValueDigitalSensors[5],lastValueDigitalSensors[6],lastValueDigitalSensors[7]);
                strcpy(historyListDigital[9].time,timeLastValueDigitalSensors);
        }
}

// Função para atualizar o histórico dos sensores analógicos
void updateHistoryAnalog(int next){
	// Copia o valor da ultima medição dos sensores analógicos para o próximo histórico
        if(next<10){
                sprintf(historyListAnalog[next].values,"%s",lastAnalogValue);
                strcpy(historyListAnalog[next].time,timeLastValueAnalogSensors);
                nextHistoryAnalog++;
	// Se o array de historicos possuir dez históricos armazenados, move os últimos nove históricos com a função memcpy
        }else{
                memcpy(historyListAnalog, &historyListAnalog[1], 9*sizeof(*historyListAnalog));
                sprintf(historyListAnalog[next].values,"%s",lastAnalogValue);
                strcpy(historyListAnalog[9].time,timeLastValueAnalogSensors);
        }

}
// Fatia a string recebida com o valor de todos os sensores digitais e adiciona em uma posição no array
void setDigitalValueSensors(){
  char * substr =  malloc(50);
  substr = strtok(bufDigitalValues, ",");
   
  while( substr != NULL ) {
      char *pinName = malloc(2);
      strncpy(pinName, substr,2);
      int index = ((int)pinName[1]) - ((int)'0');

      char *pinValue = malloc(2);
      strncpy(pinValue, substr+3,1);
      lastValueDigitalSensors[index] = *pinValue;

      substr = strtok(NULL, ",");
   }
   // pega o horário atual
   getTime(timeLastValueDigitalSensors);
  // atualiza o histórico
   updateHistoryDigital(nextHistoryDigital);
}

// Salva o valor da última medição do sensor analógico e atualiza o histórico
void setAnalogValueSensors(){
        strcpy(lastAnalogValue,bufAnalogValue);
        getTime(timeLastValueAnalogSensors);
        updateHistoryAnalog(nextHistoryAnalog);
}

// Função utilizada para publicar mensagens em um tópico
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

// Função callback utilizada para receber mensagens de tópicos inscritos
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
                        testConnectionNode = 0;
                }
                else if(strcmp(msg,"0xFA") == 0){
                        printf("Intervalo mudado");
                }
    }else if(strcmp(topicName,DIGITAL_SENSOR) == 0){
        bufDigitalValues = msg;
        setDigitalValueSensors();
                newInfo = 1;
    }else if(strcmp(topicName,ANALOG_SENSOR) == 0){
                bufAnalogValue = msg;
                setAnalogValueSensors();
                newInfo = 1;
    }else if(strcmp(topicName,APP_RESPONSE) == 0){
                if(strcmp(msg,"0x201") == 0){
                        testConnectionApp = 0;
                }
    }else if(strcmp(topicName,APP_REQUEST) == 0){
                if(strcmp(msg,"0x06") == 0){
                        appSolicitationCounter++;
                        appSolicitation = 0x06;
                }else if(strcmp(msg,"0x07") == 0){
                        appSolicitationCounter++;
                        appSolicitation = 0x07;
                }else if(strcmp(msg,"0x08") == 0){
                        appSolicitationCounter++;
                        appSolicitation = 0x08;
                }

        }


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

// Função callback que será executada caso a conexão seja perdida
void connlost(void *context, char *cause)
{
    printf("\nConexão perdida\n");
    printf("     cause: %s\n", cause);
}


// Funções do menu lcd

// função utilizada para incrementar ou decrementar uma variável ao pressionar um botão
void isPressed(int btt, int (*function)(int, int, int), int* controller, int max,int min){
        if(digitalRead(btt) == 0){
                delay(90);
                if(digitalRead(btt) == 0){
                        *controller = function(*controller, max, min);
                        while(digitalRead(btt) == 0);
                }
        }
}

// Função utilzada para entrar em outra função ao pressionar um botão
void enter(int btt,void (*function)(void)){
        if(digitalRead(btt) == 0){
                delay(90);
                if(digitalRead(btt) == 0){
                        function();
                        while(digitalRead(btt) == 0);
                }
        }
}

// Função utilizada para trocar o estado de um sensor para ativado ou desativado
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

// converte o intervalo em segundos com base na unidade de tempo definida
void convertTimeInterval(){
        if(timeUnit == 'h'){
                timeSeconds = timeInterval * 3600;
        } else if(timeUnit == 'm'){
                timeSeconds = timeInterval * 60;
        }else{
                timeSeconds = timeInterval;
        }
}

// Menu utilizado para ajustar o intervalo de tempo em que os sensores serão atualizados
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
        convertTimeInterval();
        stopLoopSetTimeInterval = 0;
        lcdClear(lcd);
}

// Função utilizada para modificar o estado do led da estação de medição
void setLedState(){
        if(ledState == 1){
                send(REQUEST,SET_OFF_NODEMCU_LED);
        }else{
                send(REQUEST,SET_ON_NODEMCU_LED);
        }
}

// Menu utilzado para mudar a unidade de tempo do intervalo de medição
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


// Função que envia o estado(ativado ou desativado) de todos os sensores para a estação de medição
void sendActiveSensors(){
        char str[50];
        sprintf(str,"D0-%c,D1-%c,D2-%c,D3-%c,D4-%c,D5-%c,D6-%c,D7-%c",activeSensors[0],activeSensors[1],activeSensors[2],activeSensors[3],activeSensors[4],activeSensors[5],activeSensors[6],activeSensors[7]);
        send(ACTIVE_SENSORS,str);
}

// Função que printa a mensagem ativado ou desativado de acordo com o estado do sensor 
void statusSensorMessage(int index){
        if(activeSensors[index] == '1'){
                lcdPuts(lcd,"ATIVADO         ");
        }else{
                lcdPuts(lcd,"DESATIVADO      ");
        }
}

// Função que imprime um valor do sensor caso este esteja ativado ou a mensagem desativado caso contrário
void valueDigitalSensor(int index){

        if(activeSensors[index] == '1'){
                lcdPrintf(lcd,"VALOR:%c         ",lastValueDigitalSensors[index]);
        }else{
                lcdPuts(lcd,"DESATIVADO      ");
        }
}

// Menu utilizado para definir o estado(ativado ou desativado) dos sensores
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

// Menu que dá acesso as configurações do sistema
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

// Menu que da acesso ao valor em tempo real dos sensores analógicos
void analogSensorsMenu(){
        lcdClear(lcd);
        while(!stopLoopAnalogSensorsMenu){
                switch(currentMenuAnalogSensorOption){
                        case 1:
                                lcdHome(lcd);
                                lcdPuts(lcd,"    SENSOR A0   ");
                                lcdPosition(lcd,0,1);
                                lcdPrintf(lcd,"    VALOR:%s  ",lastAnalogValue);
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
// Menu que da acesso ao valor em tempo real dos sensores digitais
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

// Menu que imprime o estado de conexão do aplicativo e da NodeMCU
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

// Menu que da acesso ao histórico das medições dos sensores digitais
void historyDigitalSensors(){
        lcdClear(lcd);

        while(!stopLoopHistoryDigitalSensors){
                if(nextHistoryDigital == 0){
                        lcdPrintf(lcd,"  SEM HISTORICO ",historyListDigital[0].values);
                        lcdPosition(lcd,0,1);
                        lcdPrintf(lcd,"                ");
                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                }else{
                        switch(currentHistoryDigitalSensorOption){
                                case 1:
                                        lcdHome(lcd);
										if(nextHistoryDigital == 0){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[0].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H01->  %s",historyListDigital[0].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 2:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 1){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[1].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H02->  %s",historyListDigital[1].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 3:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 2){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[2].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H03->  %s",historyListDigital[2].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 4:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 3){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[3].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H04->  %s",historyListDigital[3].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 5:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 4){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[4].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H05->  %s",historyListDigital[4].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 6:
                                        lcdHome(lcd);
                                       if(nextHistoryDigital == 5){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[5].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H06->  %s",historyListDigital[5].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 7:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 6){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[6].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H07->  %s",historyListDigital[6].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 8:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 7){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[7].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H08->  %s",historyListDigital[7].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 9:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 8){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[8].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H09->  %s",historyListDigital[8].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                                case 10:
                                        lcdHome(lcd);
                                        if(nextHistoryDigital == 9){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                						}else{
										lcdPrintf(lcd,"%s ",historyListDigital[9].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H10->  %s",historyListDigital[9].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryDigitalSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryDigitalSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryDigitalSensors);
                                        break;
                        }
                }

        }
        stopLoopHistoryDigitalSensors = 0;
        currentHistoryDigitalSensorOption = 1;
}

// Menu que dá acesso ao histórico dos sensores analógicos
void historyAnalogSensors(){
        lcdClear(lcd);

        while(!stopLoopHistoryAnalogSensors){
                if(nextHistoryAnalog == 0){
                        lcdPrintf(lcd,"  SEM HISTORICO ",historyListAnalog[0].values);
                        lcdPosition(lcd,0,1);
                        lcdPrintf(lcd,"                ");
                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                }else{
                        switch(currentHistoryAnalogSensorOption){
                                case 1:
                                        lcdHome(lcd);
                                        lcdPrintf(lcd,"%s ",historyListAnalog[0].values);
                                        lcdPosition(lcd,0,1);
                                        lcdPrintf(lcd,"H01-> %s",historyListAnalog[0].time);
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 2:
                                        lcdHome(lcd);
										if(nextHistoryAnalog == 1){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[1].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H02-> %s",historyListAnalog[1].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 3:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 2){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[2].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H03-> %s",historyListAnalog[2].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 4:
										
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 3){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[3].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H04-> %s",historyListAnalog[3].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 5:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 4){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[4].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H05-> %s",historyListAnalog[4].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 6:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 5){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[5].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H06-> %s",historyListAnalog[5].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 7:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 6){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[6].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H07-> %s",historyListAnalog[6].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 8:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 7){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[7].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H08-> %s",historyListAnalog[7].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 9:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 8){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[8].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H09-> %s",historyListAnalog[8].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                                case 10:
                                        lcdHome(lcd);
                                        if(nextHistoryAnalog == 9){
										lcdPrintf(lcd,"  SEM HISTORICO ");
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"                ");
										close(BUTTON_3,&stopLoopHistoryAnalogSensors);
										}else{
										lcdPrintf(lcd,"%s ",historyListAnalog[9].values);
										lcdPosition(lcd,0,1);
										lcdPrintf(lcd,"H10-> %s",historyListAnalog[9].time);
										}
                                        isPressed(BUTTON_2,increment,&currentHistoryAnalogSensorOption,10,1);
                                        isPressed(BUTTON_1,decrement,&currentHistoryAnalogSensorOption,10,1);
                                        close(BUTTON_3,&stopLoopHistoryAnalogSensors);
                                        break;
                        }
                }

        }
        stopLoopHistoryAnalogSensors = 0;
        currentHistoryAnalogSensorOption = 1;
}

// Menu com opções de histórico dos sensores
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
                                enter(BUTTON_3,historyAnalogSensors);
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

//  Menu principal
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

// Função callback que enviará as informações dos sensores para o aplicativo
void *sendInfo(void *arg){
        while(1){
                if(newInfo == 1){
                        char bufDigital[16];
                        sprintf(bufDigital,"%c,%c,%c,%c,%c,%c,%c,%c",lastValueDigitalSensors[0],lastValueDigitalSensors[1],lastValueDigitalSensors[2],lastValueDigitalSensors[3],lastValueDigitalSensors[4],lastValueDigitalSensors[5],lastValueDigitalSensors[6],lastValueDigitalSensors[7]);

                        char bufAnalog[5];
                        sprintf(bufAnalogValue,"%s",lastAnalogValue);

                        char bufHistory[260];
                        sprintf(bufHistory,"%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s-%c,%c,%c,%c,%c,%c,%c,%c,%s",
                        historyListDigital[0].values[0],historyListDigital[0].values[1],
                        historyListDigital[0].values[2],historyListDigital[0].values[3],
                        historyListDigital[0].values[4],historyListDigital[0].values[5],
historyListDigital[0].values[6],historyListDigital[0].values[7],historyListDigital[0].time,
                        historyListDigital[1].values[0],historyListDigital[1].values[1],
                        historyListDigital[1].values[2],historyListDigital[1].values[3],
                        historyListDigital[1].values[4],historyListDigital[1].values[5],
historyListDigital[10].values[6],historyListDigital[1].values[7],historyListDigital[1].time,
                        historyListDigital[2].values[0],historyListDigital[2].values[1],
                        historyListDigital[2].values[2],historyListDigital[2].values[3],
                        historyListDigital[2].values[4],historyListDigital[2].values[5],
historyListDigital[2].values[6],historyListDigital[2].values[7],historyListDigital[2].time,
                        historyListDigital[3].values[0],historyListDigital[3].values[1],
                        historyListDigital[3].values[2],historyListDigital[3].values[3],
                        historyListDigital[3].values[4],historyListDigital[3].values[5],
historyListDigital[3].values[6],historyListDigital[3].values[7],historyListDigital[3].time,
                        historyListDigital[4].values[0],historyListDigital[4].values[1],
                        historyListDigital[4].values[2],historyListDigital[4].values[3],
                        historyListDigital[4].values[4],historyListDigital[4].values[5],
historyListDigital[4].values[6],historyListDigital[4].values[7],historyListDigital[4].time,
                        historyListDigital[5].values[0],historyListDigital[5].values[1],
                        historyListDigital[5].values[2],historyListDigital[5].values[3],
                        historyListDigital[5].values[4],historyListDigital[5].values[5],
historyListDigital[5].values[6],historyListDigital[5].values[7],historyListDigital[5].time,
                        historyListDigital[6].values[0],historyListDigital[6].values[1],
                        historyListDigital[6].values[2],historyListDigital[6].values[3],
                        historyListDigital[6].values[4],historyListDigital[6].values[5],
historyListDigital[6].values[6],historyListDigital[6].values[7],historyListDigital[6].time,
                        historyListDigital[7].values[0],historyListDigital[7].values[1],
                        historyListDigital[7].values[2],historyListDigital[7].values[3],
                        historyListDigital[7].values[4],historyListDigital[7].values[5],
historyListDigital[7].values[6],historyListDigital[7].values[7],historyListDigital[7].time,
                        historyListDigital[8].values[0],historyListDigital[8].values[1],
                        historyListDigital[8].values[2],historyListDigital[8].values[3],
                        historyListDigital[8].values[4],historyListDigital[8].values[5],
historyListDigital[8].values[6],historyListDigital[8].values[7],historyListDigital[8].time,
                        historyListDigital[9].values[0],historyListDigital[9].values[1],
                        historyListDigital[9].values[2],historyListDigital[9].values[3],
                        historyListDigital[9].values[4],historyListDigital[9].values[5],
historyListDigital[9].values[6],historyListDigital[9].values[7],historyListDigital[9].time);
                        newInfo = 0;

                        send(APP_DIGITAL_SENSOR,bufDigital);
                        send(APP_ANALOG_SENSOR,bufAnalog);
                        send(APP_HISTORY,bufHistory);
                }
        }

}

// função callback que irá verificar se o aplicativo solicitou algo a SBC
void *checkAppSolicitations(void *arg){
        while(1){
                if(appSolicitationCounter > 0){
                        if(appSolicitation == 0x06){
                                send(REQUEST,SET_ON_NODEMCU_LED);
                        }else if(appSolicitation == 0x07){
                                send(REQUEST,SET_OFF_NODEMCU_LED);
                        }else if(appSolicitation == 0x08){
                                convertTimeInterval();
                                char buf[10];
                                sprintf(buf,"%ld",timeSeconds);
                                send(TIME_INTERVAL,buf);
                        }
                        appSolicitation = 0;
                        appSolicitationCounter--;
                }
        }
}

// Função callback que checa as conexões da NodeMCU e da aplicação web
void *checkConnections(void *arg){
        while (1){
                testConnectionNode = 1;
                testConnectionApp = 1;

                send(REQUEST, GET_NODE_CONNECTION_STATUS);
                send(RASP_REQUEST,GET_APP_CONNECTION_STATUS);
                delay(500);

                if(testConnectionNode == 1){
                        connectionNode = -1;
                }else{
                        connectionNode = 1;
                        send(REQUEST,GET_LED_VALUE);
                }

                if(testConnectionApp == 1){
                        connectionApp = -1;
                }else{
                        connectionApp = 1;
                        char bufLed[3];
                        char bufNodeStatus[3];
                        sprintf(bufLed,"l%d",ledState);
                        sprintf(bufNodeStatus,"n%d",connectionNode);
                        send(RASP_RESPONSE,bufLed);
                        send(RASP_RESPONSE,bufNodeStatus);
                }
        }
}

int main(int argc, char* argv[])
{
   
    int rc;
    // Configura os pinos da SBC
    wiringPiSetup();
    // Inicializa o lcd
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
    
    // Define os pinos em modo de entrada
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    pinMode(BUTTON_3,INPUT);
	
    // Define as configurações do cliente MQTT
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = USERNAME;
    conn_opts.password = PASSWORD;

    // Instancia um cliente MQTT
    MQTTClient_create(&client, BROKER_ADDRESS, CLIENTID,
        MQTTCLIENT_PERSISTENCE_NONE, NULL);
    conn_opts.keepAliveInterval = 2000;
    conn_opts.cleansession = 1;

    // Define as funções callback do cliente MQTT
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, NULL);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }

    printf("Conexão estabelecida\n\n");
	
    // Inscreve o cliente em tópicos no broker
    MQTTClient_subscribe(client, RESPONSE, QOS2);
    MQTTClient_subscribe(client, ANALOG_SENSOR, QOS2);
    MQTTClient_subscribe(client, DIGITAL_SENSOR, QOS2);
    MQTTClient_subscribe(client,APP_REQUEST, QOS2);
    MQTTClient_subscribe(client,APP_RESPONSE, QOS2);
    MQTTClient_subscribe(client,APP_TIME_INTERVAL, QOS2);
    MQTTClient_subscribe(client,APP_ACTIVE_SENSORS, QOS2);
	
    // Cria threads para checar solicitações e o estado das conexões
    pthread_create(&stats_connection, NULL, checkConnections, NULL);
    pthread_create(&checkSolicitation,NULL, checkAppSolicitations, NULL);
    
    // Chama a função para executar o menu no display lcd
    mainMenu();
    // Chama a função para finalizar o programa
    finish();

    return rc;
 }

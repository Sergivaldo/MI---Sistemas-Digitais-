#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <string.h> 
#include <PubSubClient.h>

#ifndef STASSID
#define STASSID "INTELBRAS"             // nome da rede WiFi
#define STAPSK  "Pbl-Sistemas-Digitais" // Senha da rede WiFi
#endif

// Nome do ESP na rede
const char* host = "ESP-10.0.0.108";  

// Definições do servidor MQTT
const char* BROKER_MQTT = "10.0.0.101";  //URL do broker MQTT 
int BROKER_PORT = 1883;  

// Definições do ID
#define ID_MQTT "NODE01"        // ID desta nodeMCU
WiFiClient wifiClient;
PubSubClient MQTT(wifiClient);   // Instancia o Cliente MQTT passando o objeto espClient

// Protocolos de comunicação com a OrangePi
# define  GET_ANALOG_INPUT_VALUE  "0x04"
# define  GET_DIGITAL_INPUT_VALUE  "0x05"
# define  SET_ON_NODEMCU_LED  "0x06"
# define  SET_OFF_NODEMCU_LED  "0x07"

// Definições dos tópicos
#define ANALOG_SENSOR "ANALOG_SENSOR"
#define DIGITAL_SENSOR "DIGITAL_SENSOR"
#define LED "LED"
#define PROTOCOLS "PROTOCOLS"
#define ADDRESS "ADDRESS"

// Endereços dos sensores digitais
#define SENSOR_D0 "D0"
#define SENSOR_D1 "D1"
#define SENSOR_D2 "D2"
#define SENSOR_D3 "D3"
#define SENSOR_D4 "D4"
#define SENSOR_D5 "D5"
#define SENSOR_D6 "D6"
#define SENSOR_D7 "D7"

// Definições de rede
IPAddress local_IP(10, 0, 0, 108);
IPAddress gateway(10, 0, 0, 1);
IPAddress subnet(255, 255, 0, 0);

const char* ssid = STASSID;
const char* password = STAPSK;

byte current_response;
byte sensor_addr;
char* address; // Ultimo endereço recebido

// Função para sinalizar quando for concluida o upload do código na nodeMCU
void code_uploaded(){
  for(int i=0;i<2;i++){
    digitalWrite(LED_BUILTIN,LOW);
    delay(150);
    digitalWrite(LED_BUILTIN,HIGH);
    delay(150);
  }
}

//Função para acender o LED
void set_led_on(){
   digitalWrite(LED_BUILTIN,LOW);
   MQTT.publish(LED, "Ligado"); 
}

//Função para apagar o LED
void set_led_off(){
   digitalWrite(LED_BUILTIN,HIGH);
   MQTT.publish(LED, "Desligado"); 
}

//Pegar o valor digital da entrada informada por "port"
void get_digital_input(byte port){
  
    if(digitalRead(port) == 1){
      MQTT.publish(DIGITAL_SENSOR, "1"); // Publicar o valor no tópico
    }
    else if(digitalRead(port) == 0){
      MQTT.publish(DIGITAL_SENSOR, "0"); // Publicar o valor no tópico
    }
}

// Pegar o valor da entrada analógica
void get_analog_input(){
    char buf[3];
    sprintf(buf,"%f",analogRead(A0));
    MQTT.publish(ANALOG_SENSOR, buf);   // Publicar o valor no tópico
}

// Configurações para realizar o upload do código via wifi
void OTA_setup(){
  
  Serial.begin(115200);
  Serial.println("Booting");

  // Configuração do IP fixo no roteador, se não conectado, imprime mensagem de falha
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(host);

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  
}

//Faz conexão com WiFi
/*void connectWifi(){
    if (WiFi.status() == WL_CONNECTED) {
        return;
    }
        
    Serial.print("Conectando-se na rede: ");
    Serial.print(STASSID);
    Serial.println("  Aguarde!");

    WiFi.begin(STASSID, STAPSK); // Conecta na rede WI-FI  
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    
    Serial.println();
    Serial.print("Conectado com sucesso, na rede: ");
    Serial.print(STASSID);  
    Serial.print("  IP obtido: ");
    Serial.println(WiFi.localIP()); 
}
*/

//Garante que as conexoes com WiFi e MQTT Broker se mantenham ativas
/*
void keepsConnections(){
    if (!MQTT.connected()) {
       connectMQTT(); 
    }
    
    connectWiFi(); //se não há conexão com o WiFI, a conexão é refeita
}
*/

//Faz conexão com Broker MQTT
void connectMQTT(){
    while (!MQTT.connected()) {
        Serial.print("Conectando ao Broker MQTT: ");
        Serial.println(BROKER_MQTT);
        if (MQTT.connect(ID_MQTT)) {
            Serial.println("Conectado ao Broker com sucesso!");
        } 
        else {
            Serial.println("Noo foi possivel se conectar ao broker.");
            Serial.println("Nova tentativa de conexão em 10s");
            delay(10000);
        }
    }
}

void receivePackage(char* topic, byte* payload, unsigned int length){
    char* msg;
    //obtem a string do payload recebido
    for(int i = 0; i < length; i++) 
    {
      char c = (char)payload[i];
      msg += c;
    }

    if (strcmp(topic,PROTOCOLS) == 0){ 

      // Valor da entrada analógica
      if(strcmp(msg,GET_ANALOG_INPUT_VALUE) == 0){
          get_analog_input();
      }

      // Valor da entrada digital
      else if(strcmp(msg,GET_DIGITAL_INPUT_VALUE) == 0){
          // Pegar o valor da entrada digital solicitada pelo endereço enviado
          if(strcmp(address,SENSOR_D0) == 0){
            get_digital_input(D0);
          }
          else if(strcmp(address,SENSOR_D1) == 0){
            get_digital_input(D1);
          }
          else if(strcmp(address,SENSOR_D2) == 0){
            get_digital_input(D2);
          }
          else if(strcmp(address,SENSOR_D3) == 0){
            get_digital_input(D3);
          }
          else if(strcmp(address,SENSOR_D4) == 0){
            get_digital_input(D4);
          }
          else if(strcmp(address,SENSOR_D5) == 0){
            get_digital_input(D5);
          }
          else if(strcmp(address,SENSOR_D6) == 0){
            get_digital_input(D6);
          }
          else if(strcmp(address,SENSOR_D7) == 0){
            get_digital_input(D7); 
          }
      }

      // Acender led
      else if(strcmp(msg,SET_ON_NODEMCU_LED) == 0){
        set_led_on();
      }

      // Apagar led
      else if(strcmp(msg,SET_OFF_NODEMCU_LED) == 0){
        set_led_off();
      }
    }
    
    else if (strcmp(topic,ADDRESS) == 0){   
        address = msg; 
    }
}

void setup() {
  code_uploaded();
  OTA_setup(); 
  Serial.begin(9600); // 115200
  pinMode(LED_BUILTIN,OUTPUT);
  digitalWrite(LED_BUILTIN,HIGH);

  MQTT.setServer(BROKER_MQTT, BROKER_PORT); 
  MQTT.setCallback(receivePackage);

  MQTT.subscribe(PROTOCOLS);
  MQTT.subscribe(ADDRESS);
}

void loop() {
  ArduinoOTA.handle();
  
  //keepsConnections();
  MQTT.loop();

  if(digitalRead(D0) == 1);
    MQTT.publish(PROTOCOLS, SET_ON_NODEMCU_LED);
}

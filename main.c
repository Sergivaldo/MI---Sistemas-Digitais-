#include <wiringPi.h>
#include <lcd.h>
#include <stdio.h>

//USE WIRINGPI PIN NUMBERS
#define LCD_RS  13               //Register select pin
#define LCD_E   18               //Enable Pin
#define LCD_D4  21               //Data pin D4
#define LCD_D5  24               //Data pin D5
#define LCD_D6  26               //Data pin D6
#define LCD_D7  27               //Data pin D7


// Botões 
#define BUTTON_1 19
#define BUTTON_2 23
#define BUTTON_3 25

int lcd;
int cont = 0x30;

int menuOption = 0;
int AvailableSensors = 0;

// Flags de parada
int stopMain = 0;
int stopLoopInterval = 0;
int stopLoopSensors = 0;


int timeInterval = 1;

void isPressed(int btt, void (*function)(void)){
	if(digitalRead(btt) == 0){
		delay(65);
		if(digitalRead(btt) == 0){
			function();
			while(digitalRead(btt) == 0);
		}
	} 	
	
}

void nextOption(){
	if(menuOption < 6){
		menuOption++;
	}
}

void prevOption(){
	if(menuOption > 1){
		menuOption--;
	}
}

void exitLoop(){
	lcdClear(lcd);
	lcdPuts(lcd,"     SAINDO     ");
	delay(1000);
	lcdClear(lcd);
	stopMain = 1;
}

void incrementInterval(){
	if(timeInterval < 10){
		timeInterval++;
	}
}

void decrementInterval(){
	if(timeInterval > 1){
		timeInterval--;
	}
}

void exitIntervalAdjustment(){
	stopLoopInterval = 1;
}

void setTimeInterval(){
	lcdClear(lcd);
	lcdPuts(lcd,"   INTERVALO    ");
	while(!stopLoopInterval){
		lcdPosition(lcd,0,1);
		lcdPuts(lcd,"1s");
		for(int i=1;i <=timeInterval; i++){
			lcdPutchar(lcd,0xFF);
		}
		
		
		for(int aux = timeInterval; aux < 10; aux++){
			lcdPuts(lcd," ");
		}
		
		lcdPuts(lcd,"10s ");
		isPressed(BUTTON_2,incrementInterval);
		isPressed(BUTTON_1,decrementInterval);
		isPressed(BUTTON_3,exitIntervalAdjustment);
	}
	stopLoopInterval = 0;
	lcdClear(lcd);
}

void sensors(){
	while(!stopLoopSensors){
		switch(availableSensors){
			case 0:
				lcdPuts("SENSOR D0");
				break;
			case 1:
				lcdPuts("SENSOR D1");
				break;
			case 2:
				lcdPuts("SENSOR D2");
				break;
			case 3:
				lcdPuts("SENSOR D3");
				break;
			case 4:
				lcdPuts("SENSOR D4");
				break;
			case 5:
				lcdPuts("SENSOR D5");
				break;
			case 6:
				lcdPuts("SENSOR D6");
				break;
			case 7:
				lcdPuts("SENSOR D7");
				break;
		}
	}
}

int main()
{
    wiringPiSetup();
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
	
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    pinMode(BUTTON_3,INPUT);
    while(!stopMain){
	switch(menuOption){
		case 0:
			int btt = digitalRead(BUTTON_3);
			lcdHome(lcd);
			lcdPuts(lcd,"     MI - SD    ");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"   Problema 3   ");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			break;
		case 1:
			lcdHome(lcd);
			lcdPuts(lcd,"LER:  S. DIGITAL");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"                ");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			isPressed(BUTTON_3,
			break;
		case 2:
			lcdHome(lcd);
			lcdPuts(lcd,"LER: S.  ANALOG.");
			
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			break;
		case 3:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:     ACENDER");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			break;
		case 4:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:      APAGAR");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			break;
			
		case 5:
			lcdHome(lcd);
			lcdPuts(lcd,"AJUSTE INTERVALO");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			isPressed(BUTTON_3,setTimeInterval);
			break;
			
		case 6:
			lcdHome(lcd);
			lcdPuts(lcd,"      SAIR      ");
			isPressed(BUTTON_2,nextOption);
			isPressed(BUTTON_1,prevOption);
			isPressed(BUTTON_3,exitLoop);
			break;
			
	}
   }
}

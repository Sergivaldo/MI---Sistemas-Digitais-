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
int stopMain = 0;
int timeInterval = 0;

void isPressed(int btt, void (*function)(void)){
	if(digitalRead(btt) == 0){
		delay(50);
		if(digitalRead(btt) == 0){
			function();
			while(digitalRead(btt) == 0);
		}
	} 	
	
}

void nextOption(){
	if(menuOption < 5){
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

void setTimeInterval(){
	int stop = 0;
	int cont = 1;
	while(!stop){
		lcdHome(lcd);
		lcdPuts(lcd," 1 ");
		if(cont != timeInterval){
			cont = timeInterval;
			for(int i=0;i <cont; i++){
				lcdPutchar(lcd,0xFF);
			}
		}
		lcdPuts(lcd," 10");
		isPressed(BUTTON_1,incrementInterval);
		isPressed(BUTTON_2,decrementInterval);
	}
}

void incrementInterval(){
	if(timeInterval < 10){
		timeInterval++;
	}
}

void decrementInterval(){
	if(timeInterval > 1){
		timeIntervall --;
	}
}

int main()
{
    wiringPiSetup();
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
	
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    while(!stopMain){
	switch(menuOption){
		case 0:
			lcdHome(lcd);
			lcdPuts(lcd,"     MI - SD    ");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"   Problema 3   ");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			break;
		case 1:
			lcdHome(lcd);
			lcdPuts(lcd,"LER:  S. DIGITAL");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"                ");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			break;
		case 2:
			lcdHome(lcd);
			lcdPuts(lcd,"LER: S. ANALOG.");
			
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			break;
		case 3:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:     ACENDER");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			break;
		case 4:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:      APAGAR");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			break;
			
		case 5:
			lcdHome(lcd);
			lcdPuts(lcd,"AJUSTE INTERVALO");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			isPressed(BUTTON_3,setTimeInterval);
			break;
			
		case 6:
			lcdHome(lcd);
			lcdPuts(lcd,"      SAIR      ");
			isPressed(BUTTON_1,nextOption);
			isPressed(BUTTON_2,prevOption);
			isPressed(BUTTON_3,exitLoop);
			break;
			
	}
   }
}

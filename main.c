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

// Controles de navegação dos menus
int currentMenuOption = 0;
int currentMenuSensorOption = 0;
int currentMenuIntervalOption = 0;

// Flags de parada
int stopLoopMainMenu = 0;
int stopLoopIntervalMenu = 0;
int stopLoopSensorsMenu = 0;

// Intervalo de Tempo
int timeInterval = 1;
char timeUnit = 's';

// Debounce
void isPressed(int btt, void (*function)(int*, int), int* controller, int minMax){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			function(controller, minMax);
			while(digitalRead(btt) == 0);
		}
	} 	
}

void enterMenu(int btt,void (*menu)(void)){
	if(digitalRead(btt) == 0){
		delay(90);
		if(digitalRead(btt) == 0){
			menu();
			while(digitalRead(btt) == 0);
		}
	} 
}

// Incrementa uma variável se não tiver atingido seu valor máximo
void increment(int* controller, int max){
	if(*controller < max){
		*controller++;
	}
}

// Decrementa uma variável se não tiver atingido seu valor mínimo
void decrement(int* controller, int min){
	if(*controller > min){
		*controller++;
	}
}

//Encerrar o sistema
void exitLoop(){
	lcdClear(lcd);
	lcdPuts(lcd,"     SAINDO     ");
	delay(1000);
	lcdClear(lcd);
	stopMain = 1;
}

// Finalizar o ajuste do intervalo
void exitIntervalAdjustment(){
	stopLoopIntervalMenu = 1;
}

// Finalizar loop dos sensores
void exitLoopSensors(){
	stopLoopSensorsMenu = 1;
}

// Ajustar o intervalo de tempo em que os sensores serão atualizados
void setTimeInterval(){
	lcdClear(lcd);
	lcdPuts(lcd,"   INTERVALO    ");
	while(!stopLoopIntervalMenu){
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
	}
	stopLoopIntervalMenu = 0;
	lcdClear(lcd);
}


void sensorsMenu(){
	while(!stopLoopSensorsMenu){
		switch(currentMenuSensorOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D0   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D1   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 3:
			    lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D2   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D3   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D4   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D5   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D6   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
			case 8:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D7   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"      sinal     ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				isPressed(BUTTON_3,exitLoopSensors);
				break;
		}
	}
	stopLoopSensorsMenu = 0;
	lcdClear(lcd);
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
			lcdHome(lcd);
			lcdPuts(lcd,"     MI - SD    ");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"   Problema 3   ");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			break;
		case 1:
			lcdHome(lcd);
			lcdPuts(lcd,"LER:  S. DIGITAL");
			lcdPosition(lcd,0,1);
			lcdPuts(lcd,"                ");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			isPressed(BUTTON_3,sensors);
			break;
		case 2:
			lcdHome(lcd);
			lcdPuts(lcd,"LER: S.  ANALOG.");
			
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			break;
		case 3:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:     ACENDER");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			break;
		case 4:
			lcdHome(lcd);
			lcdPuts(lcd,"LED:      APAGAR");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			break;
			
		case 5:
			lcdHome(lcd);
			lcdPuts(lcd,"AJUSTE INTERVALO");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			isPressed(BUTTON_3,setTimeInterval);
			break;
			
		case 6:
			lcdHome(lcd);
			lcdPuts(lcd,"      SAIR      ");
			isPressed(BUTTON_2,increment,&currentMenuOption,6);
			isPressed(BUTTON_1,decrement,&currentMenuOption,1);
			isPressed(BUTTON_3,exitLoop);
			break;
			
	}
   }
}

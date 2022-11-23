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

// Controles de navegação dos menus
int currentMenuOption = 0;
int currentMenuSensorOption = 1;
int currentMenuIntervalOption = 1;

// Flags de parada
int stopLoopMainMenu = 0;
int stopLoopIntervalMenu = 0;
int stopLoopSensorsMenu = 0;
int stopLoopSetTimeInterval = 0;
int stopLoopSetTimeUnit = 0;
// Intervalo de Tempo
int timeInterval = 1;
char timeUnit = 's';
int timeUnitAux = 0;

// Led
int ledState = 0;

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

void exit(int btt,int* stopFlag){
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
		exit(BUTTON_3,&stopLoopSetTimeInterval);
	}
	stopLoopSetTimeInterval = 0;
	lcdClear(lcd);
}

void setLedState(){
	if(ledState == 1){
		ledState = 0;
	}else{
		ledState = 1;
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
		else{
			lcdPuts(lcd,"UNID. TEMPO: HOR");
		}
			
		isPressed(BUTTON_2,increment,&timeUnitAux,2);
		isPressed(BUTTON_1,decrement,&timeUnitAux,0);
		exit(BUTTON_3,&stopLoopSetTimeUnit);	
	}
	lcdClear(lcd);
	stopLoopSetTimeUnit = 0;
}

void intervalMenu(){
	while(!stopLoopIntervalMenu){
		switch(currentMenuIntervalOption){
			case 1:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"    INTERVALO   ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				enter(BUTTON_3,setTimeInterval);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"     AJUSTAR    ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"   UNID. TEMPO  ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				enter(BUTTON_3,setTimeUnit);
				break;
		}
		
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
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D1   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 3:
			    lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D2   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D3   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D4   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 6:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D5   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 7:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D6   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 8:
				lcdHome(lcd);
				lcdPuts(lcd,"    SENSOR D7   ");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"                ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				break;
			case 9:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				isPressed(BUTTON_2,increment,&currentMenuSensorOption,8);
				isPressed(BUTTON_1,decrement,&currentMenuSensorOption,1);
				exit(BUTTON_3,&stopLoopSensorsMenu);
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
				lcdPuts(lcd,"LEITURA:");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR DIGITAL");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				enter(BUTTON_3,sensorsMenu);
				break;
			case 2:
				lcdHome(lcd);
				lcdPuts(lcd,"LEITURA:");
				lcdPosition(lcd,0,1);
				lcdPuts(lcd,"SENSOR ANALOGICO");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				break;
			case 3:
				lcdHome(lcd);
				if(ledState == 0){
					lcdPrintf(lcd,"LED:   ON  %cOFF",0xFF);	
				}else{
					lcdPrintf(lcd,"LED:   ON%c  OFF",0xFF);
				}
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				break;

			case 4:
				lcdHome(lcd);
				lcdPuts(lcd,"  CONFIGURACOES ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				break;

			case 5:
				lcdHome(lcd);
				lcdPuts(lcd,"      SAIR      ");
				isPressed(BUTTON_2,increment,&currentMenuOption,5);
				isPressed(BUTTON_1,decrement,&currentMenuOption,1);
				exit(BUTTON_3,&stopLoopMainMenu);
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
	
    mainMenu();
    finishMessage();
}

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

int lcd;
int cont = 0x30;

void isPressed(int btt, void (*function)(void)){
	if(digitalRead(btt) == 0){
		delay(50);
		if(digitalRead(btt) == 0){
			function();
			while(digitalRead(btt) == 0);
		}
	} 	
	
}

void increment(){
	cont++;
	lcdHome(lcd);
	lcdPutchar(lcd,cont);
}
int main()
{
    wiringPiSetup();
    lcd = lcdInit (2, 16, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7,0,0,0,0);
	
    pinMode(BUTTON_1,INPUT);
    pinMode(BUTTON_2,INPUT);
    for(;;){
	isPressed(BUTTON_1,increment);
   }
}

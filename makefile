all:main

main:main.c
	gcc -o main main.c -lwiringPi -lwiringPiDev -lcrypt -lm -lrt

#include <LPC17xx.h>
#include <RTL.h>
#include <stdio.h>
#include <stdlib.h>
#include "GLCD.h"
#include "glcd_scroll.h"
#include "glcd.h"

#define WT 0x0699
#define BL Blue
#define RD Red
#define BK Black
#define DG DarkGreen
#define OG 0xFB00
#define YL Yellow


// 8 tasks are controlled 8 LEDs
// - each cell represents the successor of a task to be turned on next
OS_TID nextTaskArray[] = {0,0,0,0,0,0,0,0};

int currentLED = 1;

U32 playerX = 147;
U32 playerY = 220;

int lives = 8;
int maxBullets =10;
int numBullets = 0;
int score = 0;

const unsigned char ledPosArray[8] = { 28, 29, 31, 2, 3, 4, 5, 6 };

typedef struct Enemy{
	U32 x;
	U32 y;
} enemy_t;

typedef struct Bullet{
	U32 x;
	U32 y;
	U32 active;
} bullet_t;

typedef struct Explosion{
	U32 x;
	U32 y;
	U32 lives;
} expl;

U32 numEnemies = 0;
enemy_t enemies[3];
bullet_t bullets[5];

/***********************************Shapes*******************************/
unsigned short player[] = {  WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,DG,DG,DG,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,BK,BK,BK,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,BK,BK,BK,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,BK,BK,BK,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,DG,DG,BL,BL,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,BL,BL,DG,DG,WT,
														 DG,DG,BL,RD,RD,BL,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,BL,RD,RD,BL,DG,DG,
														 DG,DG,BL,RD,RD,BL,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,BL,RD,RD,BL,DG,DG,
														 WT,DG,DG,BL,BL,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,DG,BL,BL,DG,DG,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,DG,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,WT,WT,WT,WT,WT,WT,WT,WT};
int playerWidth = 27;
int playerHeight = 20;

unsigned short enemy[] = {   WT,WT,WT,WT,WT,WT,WT,WT,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,BK,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,WT,
														 RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,
														 RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,
														 WT,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,BK,BK,BK,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,BK,BK,BK,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,BK,BK,BK,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
														 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT};

	unsigned short explosion[] = { WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,OG,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,WT,RD,RD,OG,RD,RD,WT,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,RD,OG,RD,RD,OG,OG,OG,RD,RD,OG,RD,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,OG,OG,OG,OG,OG,OG,OG,OG,OG,OG,OG,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,RD,OG,OG,OG,OG,YL,YL,YL,YL,YL,OG,OG,OG,OG,RD,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,RD,RD,RD,OG,OG,YL,YL,YL,YL,YL,YL,YL,OG,OG,RD,RD,RD,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,RD,RD,RD,OG,OG,YL,YL,YL,YL,YL,YL,YL,YL,YL,OG,OG,RD,RD,RD,WT,WT,WT,WT,
																 WT,WT,WT,RD,RD,OG,OG,OG,YL,YL,YL,YL,YL,YL,YL,YL,YL,YL,YL,OG,OG,OG,RD,RD,WT,WT,WT,
																 WT,WT,WT,WT,RD,RD,RD,OG,OG,YL,YL,YL,YL,YL,YL,YL,YL,YL,OG,OG,RD,RD,RD,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,RD,RD,RD,OG,OG,YL,YL,YL,YL,YL,YL,YL,OG,OG,RD,RD,RD,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,RD,OG,OG,OG,OG,YL,YL,YL,YL,YL,OG,OG,OG,OG,RD,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,OG,OG,OG,OG,OG,OG,OG,OG,OG,OG,OG,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,RD,OG,OG,RD,OG,OG,OG,RD,OG,OG,RD,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,OG,RD,RD,OG,RD,RD,RD,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,OG,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,OG,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,RD,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,RD,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT};

	unsigned short clear_square[] = {	 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,
																		 WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT};


unsigned short clear_column[] = {  WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT,WT };

unsigned short clear_cell[] = {WT};

unsigned short bullet[] = {BK,BK,BK,BK,BK };

unsigned short clear_bullet[] = {WT,WT,WT,WT,WT};


void draw_player(int posX, int posY){
	GLCD_Bitmap(posX, posY, 27, 20, (unsigned char*)player);
}

void init_screen(){
	GLCD_Clear(WT);
	draw_player(playerX,playerY);
}

/*******************************LED Functions***************************/

void LEDInit(){
	LPC_SC->PCONP     |= (1 << 15); /*enables power*/
	LPC_PINCON->PINSEL3 &= ~(0xCF00); /*sets pin to be in GPIO mode*/
	LPC_PINCON->PINSEL4 &= (0xC00F); /*sets pin to be in GPIO mode*/
	LPC_GPIO1->FIODIR |= 0xB0000000; /*sets pin to be an output pin*/
	LPC_GPIO2->FIODIR |= 0x0000007C; /*sets pin to be an output pin*/
}

// Turn on the LED in a position from 0 to 7
void turnOnLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);
	//The first 3 LEDs are connected to a different port
	if ( led < 3 ) {
		LPC_GPIO1->FIOSET |= mask;
	} else {
		LPC_GPIO2->FIOSET |= mask;
	}

}

// Turn off the LED in the position from 0 to 7
void turnOffLED( unsigned char led ) {
	unsigned int mask = (1 << ledPosArray[led]);
	//The first 3 LEDs are connected to a different port
	if ( led < 3 ) {
		LPC_GPIO1->FIOCLR |= mask;
	} else {
		LPC_GPIO2->FIOCLR |= mask;
	}
}

void updateLEDS(int lives){
	int i;
	for(i=0; i<8; i++) turnOffLED(i);
	for(i=0; i<lives; i++) turnOnLED(i);
}

/*******************************INT0 Button Functions***************************/
void INT0Init() {
	LPC_PINCON->PINSEL4 &= ~(3<<20); /*sets to GPIO mode*/
	LPC_GPIO2->FIODIR   &= ~(1<<10); /*sets to input pin*/
	LPC_GPIOINT->IO2IntEnF |= (1 << 10); /*reads the rising edges to generate the Interupt Request*/
	NVIC_EnableIRQ( EINT3_IRQn ); /*Interupt Request is enabled in NVIC*/
}

// INT0 interrupt handler
void EINT3_IRQHandler( void ) {
	int i; //for looping
	int num=0;
	// Check whether the interrupt is called on the rising edge
	if ( LPC_GPIOINT->IO2IntStatF && (0x01 << 10) ) {
		//Create Bullet
		if(numBullets<maxBullets){
			for(i=0; i<maxBullets; i++){
				if(bullets[i].active==0){
					num = i;
					i=maxBullets+1;
				}
			}
			bullets[num].active = 1;
			bullets[num].x = playerX+13;
			bullets[num].y = playerY+5;
			numBullets++;
			GLCD_Bitmap(bullets[i].x, bullets[i].y, 1, 5, (unsigned char*)bullet);
		}
		LPC_GPIOINT->IO2IntClr |= (1 << 10); // clear interrupt condition
	}
}

/*******************************Joystick Functions***************************/
void JoystickInit(){
	LPC_SC->PCONP |= (1<<15);
	LPC_PINCON->PINSEL3 &= ~((3<< 8)|(3<<14)|(3<<16)|(3<<18)|(3<<20)); //selects pins P1.20 to p1.26
	LPC_GPIO1->FIODIR &= ~((1<<20)|(1<<23)|(1<<24)|(1<<25)|(1<<26)); //turns all the pins to input mode
}

U32 joy_get(){
 U32 val;
 val = (LPC_GPIO1->FIOPIN >> 20) & 0x79;
 return (val);
}

/****************************Collision************************************/
int collision(enemy_t enemies){
	if((enemies.x+27 >= playerX) && (enemies.x < playerX+27) && (enemies.y+20 > playerY)){
			if(lives>0){
				lives--;
				updateLEDS(lives);
			}
		return 1;
	}
	return 0;
}


int bullet_collision(bullet_t bullet){
	int i=0;

	for(i=0; i<3; i++){
		if((bullet.x >= enemies[i].x) && (bullet.x <= enemies[i].x+27) && (bullet.y <= enemies[i].y+20)&& (bullet.y+5 >= enemies[i].y)  ){
			GLCD_Bitmap(enemies[i].x, enemies[i].y, 27, 20, (unsigned char*)clear_square);
			enemies[i].x = (rand() % 293);
			enemies[i].y = 0;
			score+=1;
			return 1;
		}
	}
	return 0;
}

/***********************************Tasks*******************************/

__task void move_player(){
  U32 val;
	val = joy_get();
	val = (~val & 0x79);
	switch(val){
		case 0x10 : //right
			if(playerX<293){
				GLCD_Bitmap(playerX, playerY, 1, 20, (unsigned char*)clear_column);
				playerX++;
				GLCD_Bitmap(playerX, playerY, 27, 20, (unsigned char*)player);
				os_dly_wait(1);
				break;
			}
		case 0x40 : //left
			if(playerX>0){
				GLCD_Bitmap(playerX+27, playerY, 1, 20, (unsigned char*)clear_column);
				playerX--;
				GLCD_Bitmap(playerX, playerY, 27, 20, (unsigned char*)player);
				os_dly_wait(1);
				break;
			}
		default :
			GLCD_Bitmap(playerX, playerY, 27, 20, (unsigned char*)player);
			break;
	}
	os_tsk_delete_self();
}

__task void move_enemies(){
  int i;
	//draw enemies
	for(i=0; i<3; i++){
		GLCD_Bitmap(enemies[i].x, enemies[i].y, 20, 1, (unsigned char*)clear_column);
		enemies[i].y++;
		GLCD_Bitmap(enemies[i].x, enemies[i].y, 27, 20, (unsigned char*)enemy);
		if(enemies[i].y > 210 || collision(enemies[i])){
			GLCD_Bitmap(enemies[i].x, enemies[i].y, 27, 20, (unsigned char*)clear_square);
			enemies[i].x = (rand() % 293);
			enemies[i].y = 0;
		}
	}
	os_tsk_delete_self();
}

__task void move_bullets(){
  int i=0;
	for(i=0; i<=maxBullets; i++){
		if(bullets[i].active==1){
			GLCD_Bitmap(bullets[i].x, bullets[i].y, 1, 5, (unsigned char*)clear_bullet);
			bullets[i].y--;
			GLCD_Bitmap(bullets[i].x, bullets[i].y, 1, 5, (unsigned char*)bullet);
			if((bullets[i].y==0) || (bullet_collision(bullets[i]))){
				bullets[i].active = 0;
				GLCD_Bitmap(bullets[i].x, bullets[i].y, 1, 5, (unsigned char*)clear_bullet);
				bullets[i].x = 0;
				bullets[i].y = 0;
				numBullets--;
			}
		}
	}
	os_tsk_delete_self();
}

//Initial Task
__task void init_task(void){
	int i;
	unsigned char str[10];
	srand(os_time_get());
	for(i=0; i<8; i++){
		turnOnLED(i);
	}

	for(i=0; i<3; i++){
		enemies[i].x = (rand() % 293);
		enemies[i].y = 0;
		GLCD_Bitmap(enemies[i].x, enemies[i].y, 27, 20, (unsigned char*)enemy);
	}

	for(i=0; i<5; i++){
	bullets[i].x = (playerX+13);
	bullets[i].y = playerY;
	}
	while(lives>0){
		os_tsk_create(move_player, 10);
		os_tsk_create(move_bullets, 10);
		os_tsk_create(move_enemies, 10);
	}
		printf("Game Over\n");
		printf("Score %d", score);
	while(1){} //wait


}

int main( void ) {

	SystemInit();
	SystemCoreClockUpdate();

	GLCD_Init();
	//init_scroll();
	init_screen();

	//power
	LPC_SC->PCONP |= (1 << 10);
	LPC_PINCON->PINSEL3 &= ~(3 << 24);

	//talking to
	LPC_GPIO1->FIODIR |= (1 << 29);
	//sets
	LPC_GPIO1->FIOSET |= (1 << 29);
	//clear
	LPC_GPIO1->FIOCLR |= (1 << 29);

	LEDInit();
	INT0Init();
	JoystickInit();

	os_sys_init(init_task);
}

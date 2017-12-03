#include <pic32mx.h>
#include <stdint.h>
#include "buttons.h"

#define DISPLAY_VDD PORTFbits.RF6
#define DISPLAY_VBATT PORTFbits.RF5
#define DISPLAY_COMMAND_DATA PORTFbits.RF4
#define DISPLAY_RESET PORTGbits.RG9


#define DISPLAY_VDD_PORT PORTF
#define DISPLAY_VDD_MASK 0x40
#define DISPLAY_VBATT_PORT PORTF
#define DISPLAY_VBATT_MASK 0x20
#define DISPLAY_COMMAND_DATA_PORT PORTF
#define DISPLAY_COMMAND_DATA_MASK 0x10
#define DISPLAY_RESET_PORT PORTG
#define DISPLAY_RESET_MASK 0x200

#define MAX_X               127
#define MAX_Y               31
#define PLAYER_HEIGHT       8
#define PLAYER_WIDTH		2
#define BALL_HEIGHT			2
#define BALL_WIDTH			2


// The display is divided into 4 pages with 128 columns each
// where each column consists of 8 vertical pixels.
uint8_t screen[128 * 4] = {0};

char textbuffer[4][16];

// The player(s) have a x and y coordinate
// as well as speed along the Y-axis.
typedef struct Player {
	int x, y, speedY;
}Player;

// The ball has a x and y coordinate
// it has speed along both the X- and Y-axis.
typedef struct Ball {
	int x, y, speedX, speedY;
}Ball;

Ball ball;
Player player1,player2;

// Player logic
void movePlayer(){
	player1.y += player1.speedY;
	player2.y += player2.speedY;
	player1.speedY = 0;
	player2.speedY = 0;

	if (buttonOne()){
		player2.speedY = -1;
	}
	if (buttonTwo()){
		player2.speedY = 1;
	}
	if (buttonThree()) {
		player1.speedY = 1;
	}
	if (buttonFour()) {
		player1.speedY = -1;
	}
	if(player1.y < 0){
		player1.y = 0;
	}
	if(player1.y > MAX_Y - 8){
		player1.y = MAX_Y - 8;
	}
	if(player2.y < 0){
		player2.y = 0;
	}
	if(player2.y > MAX_Y - 8){
		player2.y = MAX_Y - 8;
	}
}

// Ball logic
void moveBall(){
	ball.x += ball.speedX;
	ball.y += ball.speedY;

	// check if ball is on the bottom of the screen
	if (ball.y <= 0) {
		ball.speedY *= (-1);
	}
	//top of screen
	else if (ball.y >= MAX_Y - BALL_HEIGHT) {
		ball.speedY *= (-1);
	}

	//if ball is on the far left side of the screen
	if (ball.x <= 0) {
		ball.speedX *= (-1);
	}
	//if ball is on the right side of the screen
	else if (ball.x >= MAX_X - BALL_WIDTH) {
		ball.speedX *= (-1);
	}
}

void tick() {
	moveBall();
	movePlayer();
}


// Starting values
void startGame(){
	ball.x = 64;
	ball.y = 16;
	ball.speedX = 2;
	ball.speedY = 1;

	player1.x = 0;
	player1.y = 14;
	player1.speedY = 0;

	player2.x = 126;
	player2.y = 14;
	player2.speedY = 0;
}

//we update a pixel into SPI format, so it can light up.
//We have 4 rows (0,1,2,3), that consist of columns of 8 bits (8*4 = 32 = screen height)
//we have 128 of these columns
void updatePixel(int x, int y){
	int row = 0;
	if(y>0) {
		row = y / 8;
	}
	// the OR-masking gives us access to the bit in our column of size 8, that we want to turn on.
	screen[row * 128 + x] |= 1 << (y - row * 8);
}

//draw out the player
void drawPlayer(Player p) {
	int i, j;
	//For the width
	for (i = 0; i < PLAYER_WIDTH; i++){
		//for the height
		for (j = 0; j < PLAYER_HEIGHT; j++){
			updatePixel(p.x + i, p.y + j);
		}
	}
}

void drawBall(Ball b) {
	int i, j;
	for (i = 0; i < BALL_WIDTH; i++){
		for (j = 0; j < BALL_HEIGHT; j++){
			updatePixel(b.x + i, b.y + j);
		}
	}
}


void resetScreen(){
	int i;
	for(i = 0; i< (128*4); i++){
		screen[i] = 0;
	}
}

void delay(int cyc) {
	int i;
	for(i = cyc; i > 0; i--);
};

uint8_t spi_send_recv(uint8_t data) {
	while(!(SPI2STAT & 0x08));
	SPI2BUF = data;
	while(!(SPI2STAT & 0x01));
	return SPI2BUF;
};

void display_init() {
	DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
	delay(10);
	DISPLAY_VDD_PORT &= ~DISPLAY_VDD_MASK;
	delay(1000000);

	spi_send_recv(0xAE);
	DISPLAY_RESET_PORT &= ~DISPLAY_RESET_MASK;
	delay(10);
	DISPLAY_RESET_PORT |= DISPLAY_RESET_MASK;
	delay(10);

	spi_send_recv(0x8D);
	spi_send_recv(0x14);

	spi_send_recv(0xD9);
	spi_send_recv(0xF1);

	DISPLAY_VBATT_PORT &= ~DISPLAY_VBATT_MASK;
	delay(10000000);

	spi_send_recv(0xA1);
	spi_send_recv(0xC8);

	spi_send_recv(0xDA);
	spi_send_recv(0x20);

	spi_send_recv(0xAF);
}

void updateScreen(uint8_t screen[]) {
	int i, j;
	for (i = 0; i < 4; i++){

		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
        spi_send_recv(0x22);
        spi_send_recv(i);

        spi_send_recv(0 & 0xF);
        spi_send_recv(0x10 | ((0 >> 4) & 0xF));

        DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;

		for(j = 0; j < 128; j++){
			spi_send_recv(screen[i*128 + j]);
		}
	}
}

void drawToScreen(){
	resetScreen();
	drawPlayer(player1);
	drawPlayer(player2);
	drawBall(ball);
	updateScreen(screen);
}

void spi_init(){
	/* Set up peripheral bus clock */
	OSCCON &= ~0x180000;
	OSCCON |= 0x080000;

	/* Set up output pins */
	AD1PCFG = 0xFFFF;
	ODCE = 0x0;
	TRISECLR = 0xFF;
	PORTE = 0x0;

	/* Output pins for display signals */
	PORTF = 0xFFFF;
	PORTG = (1 << 9);
	ODCF = 0x0;
	ODCG = 0x0;
	TRISFCLR = 0x70;
	TRISGCLR = 0x200;

	/* Set up input pins */
	TRISDSET = (1 << 8);
	TRISFSET = (1 << 1);

	/* Set up SPI as master */
	SPI2CON = 0;
	SPI2BRG = 4;

	/* Clear SPIROV*/
	SPI2STATCLR &= ~0x40;
	/* Set CKP = 1, MSTEN = 1; */
        SPI2CON |= 0x60;

	/* Turn on SPI */
	SPI2CONSET = 0x8000;
}

int main(void) {
	spi_init();
	display_init();
	startGame();

	while(1){
		int i;
		for(i = 0; i<100000; i++){

		}
		tick();

		drawToScreen();
	}




	for(;;) ;
	return 0;
}

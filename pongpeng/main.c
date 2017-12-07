// make install TTYDEV=/dev/tty.usbserial-AJV9K343

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
#define PLAYER_WIDTH				2
#define BALL_HEIGHT					1
#define BALL_WIDTH					1

int score = 0;
int highScore = 0;

// The display is divided into 4 pages with 128 columns each
// where each column consists of 8 vertical pixels.
uint8_t screen[128 * 4] = {0};

char scoreArr[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

static const uint8_t const font[] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 94, 0, 0, 0, 0,
	0, 0, 4, 3, 4, 3, 0, 0,
	0, 36, 126, 36, 36, 126, 36, 0,
	0, 36, 74, 255, 82, 36, 0, 0,
	0, 70, 38, 16, 8, 100, 98, 0,
	0, 52, 74, 74, 52, 32, 80, 0,
	0, 0, 0, 4, 3, 0, 0, 0,
	0, 0, 0, 126, 129, 0, 0, 0,
	0, 0, 0, 129, 126, 0, 0, 0,
	0, 42, 28, 62, 28, 42, 0, 0,
	0, 8, 8, 62, 8, 8, 0, 0,
	0, 0, 0, 128, 96, 0, 0, 0,
	0, 8, 8, 8, 8, 8, 0, 0,
	0, 0, 0, 0, 96, 0, 0, 0,
	0, 64, 32, 16, 8, 4, 2, 0,
	0, 62, 65, 73, 65, 62, 0, 0,
	0, 0, 66, 127, 64, 0, 0, 0,
	0, 0, 98, 81, 73, 70, 0, 0,
	0, 0, 34, 73, 73, 54, 0, 0,
	0, 0, 14, 8, 127, 8, 0, 0,
	0, 0, 35, 69, 69, 57, 0, 0,
	0, 0, 62, 73, 73, 50, 0, 0,
	0, 0, 1, 97, 25, 7, 0, 0,
	0, 0, 54, 73, 73, 54, 0, 0,
	0, 0, 6, 9, 9, 126, 0, 0,
	0, 0, 0, 102, 0, 0, 0, 0,
	0, 0, 128, 102, 0, 0, 0, 0,
	0, 0, 8, 20, 34, 65, 0, 0,
	0, 0, 20, 20, 20, 20, 0, 0,
	0, 0, 65, 34, 20, 8, 0, 0,
	0, 2, 1, 81, 9, 6, 0, 0,
	0, 28, 34, 89, 89, 82, 12, 0,
	0, 0, 126, 9, 9, 126, 0, 0,
	0, 0, 127, 73, 73, 54, 0, 0,
	0, 0, 62, 65, 65, 34, 0, 0,
	0, 0, 127, 65, 65, 62, 0, 0,
	0, 0, 127, 73, 73, 65, 0, 0,
	0, 0, 127, 9, 9, 1, 0, 0,
	0, 0, 62, 65, 81, 50, 0, 0,
	0, 0, 127, 8, 8, 127, 0, 0,
	0, 0, 65, 127, 65, 0, 0, 0,
	0, 0, 32, 64, 64, 63, 0, 0,
	0, 0, 127, 8, 20, 99, 0, 0,
	0, 0, 127, 64, 64, 64, 0, 0,
	0, 127, 2, 4, 2, 127, 0, 0,
	0, 127, 6, 8, 48, 127, 0, 0,
	0, 0, 62, 65, 65, 62, 0, 0,
	0, 0, 127, 9, 9, 6, 0, 0,
	0, 0, 62, 65, 97, 126, 64, 0,
	0, 0, 127, 9, 9, 118, 0, 0,
	0, 0, 38, 73, 73, 50, 0, 0,
	0, 1, 1, 127, 1, 1, 0, 0,
	0, 0, 63, 64, 64, 63, 0, 0,
	0, 31, 32, 64, 32, 31, 0, 0,
	0, 63, 64, 48, 64, 63, 0, 0,
	0, 0, 119, 8, 8, 119, 0, 0,
	0, 3, 4, 120, 4, 3, 0, 0,
	0, 0, 113, 73, 73, 71, 0, 0,
	0, 0, 127, 65, 65, 0, 0, 0,
	0, 2, 4, 8, 16, 32, 64, 0,
	0, 0, 0, 65, 65, 127, 0, 0,
	0, 4, 2, 1, 2, 4, 0, 0,
	0, 64, 64, 64, 64, 64, 64, 0,
	0, 0, 1, 2, 4, 0, 0, 0,
	0, 0, 48, 72, 40, 120, 0, 0,
	0, 0, 127, 72, 72, 48, 0, 0,
	0, 0, 48, 72, 72, 0, 0, 0,
	0, 0, 48, 72, 72, 127, 0, 0,
	0, 0, 48, 88, 88, 16, 0, 0,
	0, 0, 126, 9, 1, 2, 0, 0,
	0, 0, 80, 152, 152, 112, 0, 0,
	0, 0, 127, 8, 8, 112, 0, 0,
	0, 0, 0, 122, 0, 0, 0, 0,
	0, 0, 64, 128, 128, 122, 0, 0,
	0, 0, 127, 16, 40, 72, 0, 0,
	0, 0, 0, 127, 0, 0, 0, 0,
	0, 120, 8, 16, 8, 112, 0, 0,
	0, 0, 120, 8, 8, 112, 0, 0,
	0, 0, 48, 72, 72, 48, 0, 0,
	0, 0, 248, 40, 40, 16, 0, 0,
	0, 0, 16, 40, 40, 248, 0, 0,
	0, 0, 112, 8, 8, 16, 0, 0,
	0, 0, 72, 84, 84, 36, 0, 0,
	0, 0, 8, 60, 72, 32, 0, 0,
	0, 0, 56, 64, 32, 120, 0, 0,
	0, 0, 56, 64, 56, 0, 0, 0,
	0, 56, 64, 32, 64, 56, 0, 0,
	0, 0, 72, 48, 48, 72, 0, 0,
	0, 0, 24, 160, 160, 120, 0, 0,
	0, 0, 100, 84, 84, 76, 0, 0,
	0, 0, 8, 28, 34, 65, 0, 0,
	0, 0, 0, 126, 0, 0, 0, 0,
	0, 0, 65, 34, 28, 8, 0, 0,
	0, 0, 4, 2, 4, 2, 0, 0,
	0, 120, 68, 66, 68, 120, 0, 0,
};


typedef struct Player {
	int x, y, speedY;
}Player;

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

	// Player2 up
	if (buttonOne()){
		player2.speedY = -1;
	}
	// Player2 down
	if (buttonTwo()){
		player2.speedY = 1;
	}
	//Player1 down
	if (buttonThree()) {
		player1.speedY = 1;
	}
	//Player1 up
	if (buttonFour()) {
		player1.speedY = -1;
	}
	if(player1.y < 0){
		player1.y = 0;
	}
	if(player1.y > MAX_Y - PLAYER_HEIGHT){
		player1.y = MAX_Y - PLAYER_HEIGHT;
	}
	if(player2.y < 0){
		player2.y = 0;
	}
	if(player2.y > MAX_Y - PLAYER_HEIGHT){
		player2.y = MAX_Y - PLAYER_HEIGHT;
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

// Draw player
void drawPlayer(Player p) {
	int i, j;
	// Draw player width
	for (i = 0; i < PLAYER_WIDTH; i++){
		// Draw player height
		for (j = 0; j < PLAYER_HEIGHT; j++){
			updatePixel(p.x + i, p.y + j);
		}
	}
}

//Draw Ball
void drawBall(Ball b) {
	int i, j;
	//Draw Ball width
	for (i = 0; i < BALL_WIDTH; i++){
		//Draw Ball height
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
	int i, j, k, l;
	int c;
	for (i = 0; i < 4; i++){

		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
    spi_send_recv(0x22);
    spi_send_recv(i);

    spi_send_recv(0 & 0xF);
    spi_send_recv(0x10 | ((0 >> 4) & 0xF));

    DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;

		for(j = 0; j < 128; j++){
			spi_send_recv(screen[i * 128 + j]);
		}
	}
}

void display_score() {
	int i, j, x, x1;
	int ten = score / 10 ;
	int decimal = score % 10;
	x = 68;
	x1 = 60;
  char c = scoreArr[decimal];
	char k = scoreArr[ten];
  for (i = 0; i < 8; i++) {
  	screen[x + i] = font[c * 8 + i];
 	}
	for(j = 0; j < 8; j++) {
		screen[x1 + j] = font[k * 8 + j];
	}
}

void drawToScreen(){
	resetScreen();
	drawPlayer(player1);
	drawPlayer(player2);
	drawBall(ball);
	display_score();
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
		delay(100000);
		tick();
		drawToScreen();
	}

	for(;;) ;
	return 0;
}

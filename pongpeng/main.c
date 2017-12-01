#include <pic32mx.h>
#include <stdint.h>

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
#define PADDLE_HEIGHT       8
#define PADDE_WIDTH					2




char textbuffer[4][16];

typedef struct Player {
	int x, y, speedY;
}Player;

typedef struct Ball {
	int x, y, speedX, speedY;
}Ball;


Ball ball;
Player player1,player2;

//initialize the screen as all white;
uint8_t screen[128][4] = {0};



void moveBall() {
	ball.x += ball.speedX;
	ball.y += ball.speedY;

	// check if ball is on the bottom of the screen
    if (ball.y <= 0) {
        ball.y = 0;
        ball.speedY *= (-1);
    }
		//top of screen
		else if (ball.y >= MAX_Y) {
        ball.y = MAX_Y;
        ball.speedY *= (-1);
    }

		//if ball is on the far left side of the screen
		if (ball.x <= 0) {
        ball.x = 0;
        ball.speedX *= (-1);
    }
		//if ball is on the right side of the screen
		else if (ball.x >= MAX_X) {
        ball.x = MAX_X;
        ball.speedX *= (-1);
    }
}


void startGame(){
	ball.x = 64;
	ball.y = 16;
	ball.speedX = 1;
	ball.speedY = 1;

	player1.x = 0;
	player1.y = 14;

	player2.x = 126;
	player2.y = 14;
}

//we update a pixel into SPI format, so it can light up
void updateDisplay(int x, int y){

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

void display_string(int line, char *s) {
	int i;
	if(line < 0 || line >= 4)
		return;
	if(!s)
		return;

	for(i = 0; i < 16; i++)
		if(*s) {
			textbuffer[line][i] = *s;
			s++;
		} else
			textbuffer[line][i] = ' ';
}

void display_image(int x, const uint8_t *data) {
	int i, j;

	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));

		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;

		for(j = 0; j < 32; j++)
			spi_send_recv(~data[i*32 + j]);
	}
}


void draw(int x, int y) {
    int offset = 0;
    if (y > 0) { offset = y / 8; }
    game[offset * 128 + x] |= 1 << (y - offset * 8);
}


void display_ball(Ball ball) {
	for (i = 0; i < count; i++) {

	}
}

void display_update() {
	int i, j, k;
	int c;
	for(i = 0; i < 4; i++) {
		DISPLAY_COMMAND_DATA_PORT &= ~DISPLAY_COMMAND_DATA_MASK;
		spi_send_recv(0x22);
		spi_send_recv(i);

		spi_send_recv(0x0);
		spi_send_recv(0x10);

		DISPLAY_COMMAND_DATA_PORT |= DISPLAY_COMMAND_DATA_MASK;

		for(j = 0; j < 16; j++) {
			c = textbuffer[i][j];
			if(c & 0x80)
				continue;

			for(k = 0; k < 8; k++)
				spi_send_recv(font[c*8 + k]);
		}
	}
}

int main(void) {
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

	display_init();
	display_string(0, "such world");
	display_string(1, "much hello");
	display_string(2, "many text");
	display_string(3, "wow");
	display_update();

	display_image(96, icon);

	for(;;) ;
	return 0;
}

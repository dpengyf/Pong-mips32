#include <stdint.h>
#include <pic32mx.h>
#include "buttons.h"

int getbtns( void ){
  int btn = PORTD >> 5;
  btn &= 0x7;
  return btn;
}


int buttonOne(){
  int btn = PORTF;

  if(btn & 0x2){
    return 1;
  }
  else return 0;
}

int buttonTwo(){
  int btns = getbtns();

  if((btns & 0x1)){
    return 1;
  }
  else return 0;
}

int buttonThree(){
  int btns = getbtns();

  if((btns & 0x2) == 2){
    return 1;
  }
  else return 0;
}

int buttonFour(){
  int btns = getbtns();

  if((btns & 0x4) == 4){
    return 1;
  }
  else return 0;
}




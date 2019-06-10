// https://github.com/vandalton/DaltonAJKI
#include <PS4USB.h>
#include <BTHID.h>
#include "KeyboardParser.h"

// Atari joystick pinout - seen from the front
// \1 2 3 4 5/
//  \6 7 8 9/
//
// 1 - up, 2 - down, 3 - left, 4 - right, 6 - fire
// 7 - 5V, 8 - gnd

#define OUT_UP 2 // connect digital pin 2 of Arduino to pin 1 of joystick port
#define OUT_DOWN 3
#define OUT_LEFT 4
#define OUT_RIGHT 5
#define OUT_FIRE 6

#define OUT_KR1 A5
#define OUT_KR2 7

#define OUT_START A0
#define OUT_SELECT A1
#define OUT_OPTION A2

int outputs[10] = {OUT_UP, OUT_DOWN, OUT_LEFT, OUT_RIGHT, OUT_FIRE, OUT_START, OUT_SELECT, OUT_OPTION, OUT_KR1, OUT_KR2};

volatile bool up = false, down = false, left = false, right = false, fire = false, start = false, select = false, option = false;

volatile int KRCounter = 0;
volatile int pc;
volatile int KR5Last = 0;

USB Usb;
//USBHub Hub1(&Usb);
BTD Btd(&Usb);

PS4USB PS4(&Usb);
//BTHID bthid(&Btd, PAIR, "0000");
BTHID bthid(&Btd);

KbdRptParser keyboardPrs;

void setup() {  
  for(int i=0;i<10;++i)
  {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], HIGH);
  }

  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  
  Serial.begin(115200);
  
  if (Usb.Init() == -1) {
    while (1); // Halt
  }

  bthid.SetReportParser(KEYBOARD_PARSER_ID, &keyboardPrs);
  bthid.setProtocolMode(USB_HID_BOOT_PROTOCOL);
  
  cli();
  
  PCICR |= bit (PCIE1);     // interrupt port c
  PCMSK1 |= bit (PCINT11);  // pin A3
  
  sei();
}

void loop() {
  Usb.Task();

  up = down = left = right = fire = start = select = option = false;
  
  if (PS4.connected()) {    
    up |= PS4.getButtonPress(UP) || PS4.getAnalogHat(LeftHatY) < 64;
    down |= PS4.getButtonPress(DOWN) || PS4.getAnalogHat(LeftHatY) > 192;
    left |= PS4.getButtonPress(LEFT) || PS4.getAnalogHat(LeftHatX) < 64;
    right |= PS4.getButtonPress(RIGHT) || PS4.getAnalogHat(LeftHatX) > 192;
    fire |= PS4.getButtonPress(CROSS);

    start |= PS4.getButtonPress(OPTIONS);
    select |= PS4.getButtonPress(SHARE);
    option |= PS4.getButtonPress(TOUCHPAD);
  }

  if(bthid.connected) {
    start |= keyboardPrs.IsSpecialPressed(0);
    select |= keyboardPrs.IsSpecialPressed(1);
    option |= keyboardPrs.IsSpecialPressed(2);
  
    up |= keyboardPrs.IsSpecialPressed(4);
    down |= keyboardPrs.IsSpecialPressed(5);
    left |= keyboardPrs.IsSpecialPressed(6);
    right |= keyboardPrs.IsSpecialPressed(7);
    fire |= keyboardPrs.IsSpecialPressed(8);
  }

  digitalWrite(OUT_UP, up ? LOW : HIGH);
  digitalWrite(OUT_DOWN, down ? LOW : HIGH);
  digitalWrite(OUT_LEFT, left ? LOW : HIGH);
  digitalWrite(OUT_RIGHT, right ? LOW : HIGH);
  digitalWrite(OUT_FIRE, fire ? LOW : HIGH);
  
  digitalWrite(OUT_START, start ? LOW : HIGH);
  digitalWrite(OUT_SELECT, select ? LOW : HIGH);
  digitalWrite(OUT_OPTION, option ? LOW : HIGH);
}

ISR(PCINT1_vect) {
  pc = PINC;
  
  if(KR5Last && !(pc & (1 << 4))) {
    KRCounter = 0;
  }

  KR5Last = pc & (1 << 4);

  digitalWrite(OUT_KR1, keyboardPrs.IsKeyPressed(KRCounter)? LOW : HIGH);
  digitalWrite(OUT_KR2, keyboardPrs.IsModifierPressed(KRCounter) ? LOW : HIGH);
  
  ++KRCounter;
}


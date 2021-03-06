// https://github.com/vandalton/DaltonAJKI
#include <PS4USB.h>

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

#define OUT_START A0
#define OUT_SELECT A1
#define OUT_OPTION A2

int outputs[9] = {OUT_UP, OUT_DOWN, OUT_LEFT, OUT_RIGHT, OUT_FIRE, OUT_START, OUT_SELECT, OUT_OPTION, OUT_KR1};
bool keyboard[64];
volatile int KRCounter = 0;
volatile int pc;
volatile int KR5Last = 0;

USB Usb;
PS4USB PS4(&Usb);

void setup() {
  for(int i=0;i<9;++i)
  {
    pinMode(outputs[i], OUTPUT);
    digitalWrite(outputs[i], HIGH);
  }

  for(int i=0;i<64;++i)
  {
    keyboard[i] = false;
  }

  pinMode(A3, INPUT);
  pinMode(A4, INPUT);

  Serial.begin(115200);
  
  if (Usb.Init() == -1) {
    while (1); // Halt
  }

  cli();
  
  PCICR |= bit (PCIE1);     // interrupt port c
  PCMSK1 |= bit (PCINT11);  // pin A3
  
  sei();
}

void loop() {
  Usb.Task();

  if (PS4.connected()) {
    digitalWrite(OUT_UP, PS4.getButtonPress(UP) || PS4.getAnalogHat(LeftHatY) < 64 ? LOW : HIGH);
    digitalWrite(OUT_DOWN, PS4.getButtonPress(DOWN) || PS4.getAnalogHat(LeftHatY) > 192 ? LOW : HIGH);
    digitalWrite(OUT_LEFT, PS4.getButtonPress(LEFT) || PS4.getAnalogHat(LeftHatX) < 64 ? LOW : HIGH);
    digitalWrite(OUT_RIGHT, PS4.getButtonPress(RIGHT) || PS4.getAnalogHat(LeftHatX) > 192 ? LOW : HIGH);
    digitalWrite(OUT_FIRE, PS4.getButtonPress(CROSS) ? LOW : HIGH);

    digitalWrite(OUT_START, PS4.getButtonPress(OPTIONS) ? LOW : HIGH);
    digitalWrite(OUT_SELECT, PS4.getButtonPress(SHARE) ? LOW : HIGH);
    digitalWrite(OUT_OPTION, PS4.getButtonPress(TOUCHPAD) ? LOW : HIGH);
    //digitalWrite(OUT_RESET, PS4.getButtonPress(OPTIONS) && PS4.getButtonPress(SHARE) ? LOW : HIGH); 

    keyboard[0] = PS4.getButtonPress(TRIANGLE);
    keyboard[63] = PS4.getButtonPress(SQUARE);
  }
}

ISR(PCINT1_vect) {
  pc = PINC;
  
  if(KR5Last && !(pc & (1 << 4))) {
    KRCounter = 0;
  }

  KR5Last = pc & (1 << 4);

  digitalWrite(OUT_KR1, keyboard[KRCounter]? LOW : HIGH);
  
  ++KRCounter;
}


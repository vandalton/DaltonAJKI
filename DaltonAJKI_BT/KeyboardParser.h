#ifndef __kbdrptparser_h_
#define __kbdrptparser_h_

class KbdRptParser : public KeyboardReportParser {

  public:
    KbdRptParser();
    bool IsKeyPressed(int number);
    bool IsSpecialPressed(int number);
    bool IsModifierPressed(int number);
  
  protected:
    virtual uint8_t HandleLockingKeys(USBHID *hid, uint8_t key);
    virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
    virtual void OnKeyDown(uint8_t mod, uint8_t key);
    virtual void OnKeyUp(uint8_t mod, uint8_t key);
    virtual void OnKeyPressed(uint8_t key);

  private:
    bool keyboardState[64];
    bool specialState[9]; // start, select, option, reset, up, down, left, right, fire
    bool modifierState[3]; // break, shift, control
    
    void PrintKey(uint8_t mod, uint8_t key);
    void HandleKey(uint8_t c, uint8_t key, bool down);
    int KeyToNumber(uint8_t c, uint8_t key);
    int keyNumber[4*2] = {
      '>', 8,
      '<', 9, 
      '*', 56, 
      '+', 57, 
    };
      
    int nonAsciiKeyNumber[50*2] = {
      4, 0, //a 
      22, 1, //s 
      10, 2, //g 
      7, 5, //d 
      11, 6, //h 
      9, 7, //f
      37, 10, //8 
      36, 12, //7 
      39, 13, //0 
      38, 15, //9
      20, 16, //q
      26, 17, //w 
      23, 18, //t 
      28, 20, //y 
      8, 21, //e 
      21, 23, //r
      56, 25, // /
      16, 26, //m
      17, 28, //n 
      55, 29, //.
      44, 30, // space
      54, 31, //,
      30, 32, //1 
      31, 33, //2
      34, 34, //5 
      35, 36, //6
      32, 37, //3 
      33, 39, //4 
      29, 40, //z 
      27, 41, //x 
      5, 42, //b
      6, 45, //c 
      25, 47, //v 
      46, 48, //= 
      45, 49, //- 
      12, 50, //i
      24, 52, //u 
      19, 53, //p
      18, 55, //o 
      14, 58, //k
      51, 61, //; 
      13, 62, //j 
      15, 63, //l
      57, 3,  //caps
      40, 51, //return
      41, 35, //esc
      42, 11, //delete
      43, 19, //tab
      67, 46, //help (f10)
      68, 24 // inv (f11)
    };

    int specialKeys[8] = {62, 63, 64, 65, 82, 81, 80, 79};
};

KbdRptParser::KbdRptParser() {
  for(int i=0;i<64;++i)
    this->keyboardState[i] = false;
  
  for(int i=0;i<9;++i)
    this->specialState[i] = false;

  for(int i=0;i<3;++i)
    this->modifierState[i] = false;
}

bool KbdRptParser::IsKeyPressed(int number) {
  return this->keyboardState[number];
}

bool KbdRptParser::IsSpecialPressed(int number) {
  return this->specialState[number];
}

bool KbdRptParser::IsModifierPressed(int number) {
  if(number == 63)
    return this->modifierState[2];
  if(number == 47)
    return this->modifierState[1];
  if(number == 15)
    return this->modifierState[0];
    
  return false;
}

uint8_t KbdRptParser::HandleLockingKeys(USBHID *hid, uint8_t key) {
  uint8_t old_keys = kbdLockingKeys.bLeds;

  switch (key) {
    case UHS_HID_BOOT_KEY_NUM_LOCK:
      Serial.println(F("Num lock"));
      kbdLockingKeys.kbdLeds.bmNumLock = ~kbdLockingKeys.kbdLeds.bmNumLock;
      break;
    case UHS_HID_BOOT_KEY_SCROLL_LOCK:
      Serial.println(F("Scroll lock"));
      kbdLockingKeys.kbdLeds.bmScrollLock = ~kbdLockingKeys.kbdLeds.bmScrollLock;
      break;
  }

  if (old_keys != kbdLockingKeys.bLeds && hid) {
    BTHID *pBTHID = reinterpret_cast<BTHID *> (hid); // A cast the other way around is done in BTHID.cpp
    pBTHID->setLeds(kbdLockingKeys.bLeds); // Update the LEDs on the keyboard
  }

  return 0;
}

void KbdRptParser::PrintKey(uint8_t m, uint8_t key) {
  MODIFIERKEYS mod;
  *((uint8_t*)&mod) = m;
  Serial.print((mod.bmLeftCtrl == 1) ? F("C") : F(" "));
  Serial.print((mod.bmLeftShift == 1) ? F("S") : F(" "));
  Serial.print((mod.bmLeftAlt == 1) ? F("A") : F(" "));
  Serial.print((mod.bmLeftGUI == 1) ? F("G") : F(" "));

  Serial.print(F(" >"));
  Serial.print(key);
  //PrintHex<uint8_t>(key, 0x80);
  Serial.print(F("< "));

  Serial.print((mod.bmRightCtrl == 1) ? F("C") : F(" "));
  Serial.print((mod.bmRightShift == 1) ? F("S") : F(" "));
  Serial.print((mod.bmRightAlt == 1) ? F("A") : F(" "));
  Serial.println((mod.bmRightGUI == 1) ? F("G") : F(" "));
}

void KbdRptParser::HandleKey(uint8_t c, uint8_t key, bool down) {
  int tmp = KeyToNumber(c, key);
  if(tmp != -1)
    this->keyboardState[tmp] = down;

  for(int i=0;i<8;++i)
    if(key == this->specialKeys[i])
      this->specialState[i] = down;

  if(key == 69)
    this->modifierState[0] = down;
}

void KbdRptParser::OnKeyDown(uint8_t mod, uint8_t key) {
  Serial.print(F("DN "));
  PrintKey(mod, key);
  uint8_t c = OemToAscii(mod, key);

  if (c) {
    OnKeyPressed(c);
  }

  HandleKey(c, key, true);
}

void KbdRptParser::OnKeyUp(uint8_t mod, uint8_t key) {
  Serial.print(F("UP "));
  PrintKey(mod, key);

  int8_t c = OemToAscii(mod, key);

  if (c) {
    OnKeyPressed(c);
  }

  HandleKey(c, key, false);
}

void KbdRptParser::OnControlKeysChanged(uint8_t before, uint8_t after) {
  MODIFIERKEYS beforeMod;
  *((uint8_t*)&beforeMod) = before;

  MODIFIERKEYS afterMod;
  *((uint8_t*)&afterMod) = after;

  if (beforeMod.bmLeftCtrl != afterMod.bmLeftCtrl)
    Serial.println(F("LeftCtrl changed"));
  if (beforeMod.bmLeftShift != afterMod.bmLeftShift)
    Serial.println(F("LeftShift changed"));
  if (beforeMod.bmLeftAlt != afterMod.bmLeftAlt)
    Serial.println(F("LeftAlt changed"));
  if (beforeMod.bmLeftGUI != afterMod.bmLeftGUI)
    Serial.println(F("LeftGUI changed"));

  if (beforeMod.bmRightCtrl != afterMod.bmRightCtrl)
    Serial.println(F("RightCtrl changed"));
  if (beforeMod.bmRightShift != afterMod.bmRightShift)
    Serial.println(F("RightShift changed"));
  if (beforeMod.bmRightAlt != afterMod.bmRightAlt)
    Serial.println(F("RightAlt changed"));
  if (beforeMod.bmRightGUI != afterMod.bmRightGUI)
    Serial.println(F("RightGUI changed"));

  this->specialState[8] = afterMod.bmLeftCtrl;

  this->modifierState[1] = afterMod.bmLeftShift || afterMod.bmRightShift;
  this->modifierState[2] = afterMod.bmLeftCtrl || afterMod.bmRightCtrl;
}

void KbdRptParser::OnKeyPressed(uint8_t key) {
  Serial.print(F("ASCII: "));
  Serial.println((char)key);
}

int KbdRptParser::KeyToNumber(uint8_t c, uint8_t key) {
  for(int tmp = 0;tmp < 4*2;tmp += 2)
    if(this->keyNumber[tmp] == c)
      return this->keyNumber[tmp + 1];

  for(int tmp = 0;tmp < 50*2;tmp += 2)
    if(this->nonAsciiKeyNumber[tmp] == key)
      return this->nonAsciiKeyNumber[tmp + 1];
      
  return -1;
}

#endif

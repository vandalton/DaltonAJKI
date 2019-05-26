#ifndef __kbdrptparser_h_
#define __kbdrptparser_h_

class KbdRptParser : public KeyboardReportParser {

  public:
    KbdRptParser();
    bool IsKeyPressed(int number);
    bool IsSpecialPressed(int number);
  
  protected:
    virtual uint8_t HandleLockingKeys(USBHID *hid, uint8_t key);
    virtual void OnControlKeysChanged(uint8_t before, uint8_t after);
    virtual void OnKeyDown(uint8_t mod, uint8_t key);
    virtual void OnKeyUp(uint8_t mod, uint8_t key);
    virtual void OnKeyPressed(uint8_t key);

  private:
    bool keyboardState[64];
    bool specialState[9]; // start, select, option, reset, up, down, left, right, fire
    
    void PrintKey(uint8_t mod, uint8_t key);
    void HandleKey(uint8_t c, uint8_t key, bool down);
    int KeyToNumber(uint8_t c, uint8_t key);
    int keyNumber[47*2] = {
      'a', 0, 
      's', 1, 
      'g', 2, 
      'd', 5, 
      'h', 6, 
      'f', 7,
      '>', 8,
      '<', 9, 
      '8', 10, 
      '7', 12, 
      '0', 13, 
      '9', 15,
      'q', 16,
      'w', 17, 
      't', 18, 
      'y', 20, 
      'e', 21, 
      'r', 23,
      '/', 25,
      'm', 26,
      'n', 28, 
      '.', 29,
      ' ', 30, 
      ',', 31, 
      '1', 32, 
      '2', 33,
      '5', 34, 
      '6', 36, 
      '3', 37, 
      '4', 39, 
      'z', 40, 
      'x', 41, 
      'b', 42,
      'c', 45, 
      'v', 47, 
      '=', 48, 
      '-', 49, 
      'i', 50,
      'u', 52, 
      'p', 53,
      'o', 55, 
      '*', 56, 
      '+', 57, 
      'k', 58,
      ',', 61, 
      'j', 62, 
      'l', 63
    };
      
    int nonAsciiKeyNumber[4*2] = {
      40, 51, //return
      41, 35, //esc
      42, 11, //delete
      43, 19, //tab
    };

    int specialKeys[8] = {62, 63, 64, 65, 82, 81, 80, 79};
/*
INV   24
HELP  46*/
};

KbdRptParser::KbdRptParser() {
  for(int i=0;i<64;++i)
    this->keyboardState[i] = false;
  
  for(int i=0;i<9;++i)
    this->specialState[i] = false;
}

bool KbdRptParser::IsKeyPressed(int number) {
  return this->keyboardState[number];
}

bool KbdRptParser::IsSpecialPressed(int number) {
  return this->specialState[number];
}

uint8_t KbdRptParser::HandleLockingKeys(USBHID *hid, uint8_t key) {
  uint8_t old_keys = kbdLockingKeys.bLeds;

  switch (key) {
    case UHS_HID_BOOT_KEY_NUM_LOCK:
      Serial.println(F("Num lock"));
      kbdLockingKeys.kbdLeds.bmNumLock = ~kbdLockingKeys.kbdLeds.bmNumLock;
      break;
    case UHS_HID_BOOT_KEY_CAPS_LOCK:
      Serial.println(F("Caps lock"));
      kbdLockingKeys.kbdLeds.bmCapsLock = ~kbdLockingKeys.kbdLeds.bmCapsLock;
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
}

void KbdRptParser::OnKeyPressed(uint8_t key) {
  Serial.print(F("ASCII: "));
  Serial.println((char)key);
}

int KbdRptParser::KeyToNumber(uint8_t c, uint8_t key) {
  for(int tmp = 0;tmp < 47*2;tmp += 2)
    if(this->keyNumber[tmp] == c)
      return this->keyNumber[tmp + 1];

  for(int tmp = 0;tmp < 4*2;tmp += 2)
    if(this->nonAsciiKeyNumber[tmp] == key)
      return this->nonAsciiKeyNumber[tmp + 1];
      
  return -1;
}

#endif

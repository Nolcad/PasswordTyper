#include <Arduino.h>

// Set Keyboard layout
#define LAYOUT_FRENCH
#include <DigiKeyboard.h>

// Set user/password here
const char *userName = ".\\administrateur";
const char *userPass = "@dminPassword";
// Set user -> password separator (usually TAB)
const char *keySeparator = "\t";
// Set validation (usually ENTER)
const char *keyValidation = "\n";

// Set delay between keystrokes (ms)
const uint16_t keyTypeDelay = 20;
// Set delay between fields (ms)
const uint16_t keyFieldDelay = 100;

// Set number of required CAPS_LOCK transitions (ON -> OFF)
const uint8_t toogleCount = 2;
// Set timeout for toggle count sequence (ms)
const unsigned long toggleCountTimeout = 1000;

DigiKeyboardDevice DigiKeyboard = DigiKeyboardDevice::GetInstance();
uint8_t lastLedStates = 0;
unsigned long sequenceStartTime = 0;
uint8_t toggleState = 0;
uint8_t toggleCounter = 0;

void typeStuff(const char *stuff, uint16_t length, uint16_t typeDelay)
{
  for (uint16_t i = 0; i < length; i++)
  {
    DigiKeyboard.delay(typeDelay);
    DigiKeyboard.print(stuff[i]);
  }
}

void setup()
{
  pinMode(1, OUTPUT);
}

void loop()
{
  uint8_t led_states = DigiKeyboard.getLeds();
  unsigned long now = millis();

  // Reset state if timeout
  if (now - sequenceStartTime >= toggleCountTimeout)
  {
    toggleState = 0;
    toggleCounter = 0;
    digitalWrite(1, LOW);
  }

  switch (toggleState)
  {
  case 0: // Wait for first OFF->ON
    if (!(lastLedStates & CAPS_LOCK) && (led_states & CAPS_LOCK))
    {
      sequenceStartTime = now;
      toggleCounter = 0;
      toggleState = 1;
      digitalWrite(1, HIGH);
    }
    break;
  case 1: // Wait for next OFF->ON
    if ((lastLedStates & CAPS_LOCK) && !(led_states & CAPS_LOCK))
    {
      toggleCounter++;
      if (toggleCounter >= toogleCount)
      {
        digitalWrite(1, LOW);
        // We have the count, do something
        DigiKeyboard.sendKeyStroke(0);
        typeStuff(userName, strlen(userName), keyTypeDelay);
        DigiKeyboard.delay(keyFieldDelay);
        typeStuff(keySeparator, strlen(keySeparator), keyTypeDelay);
        DigiKeyboard.delay(keyFieldDelay);
        typeStuff(userPass, strlen(userPass), keyTypeDelay);
        DigiKeyboard.delay(keyFieldDelay);
        typeStuff(keyValidation, strlen(keyValidation), keyTypeDelay);

        toggleState = 0;
      }
    }
    break;
  }

  lastLedStates = led_states;
  DigiKeyboard.delay(10);
}

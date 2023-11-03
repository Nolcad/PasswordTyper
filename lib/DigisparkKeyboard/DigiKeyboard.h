/*
 * Based on Obdev's AVRUSB code and under the same license.
 * Modified for Digispark by Digistump
 * Further modified by Axlan to capture indicator LEDs based on
 * https://github.com/7enderhead/kbdwtchdg
 */

#ifndef __DigiKeyboard_h__
#define __DigiKeyboard_h__

#include <Arduino.h>
#include "keylayouts.h"

#define NUM_LOCK (1<<0)
#define CAPS_LOCK (1<<1)
#define SCROLL_LOCK (1<<2)

class DigiKeyboardDevice : public Print {
 public:
  DigiKeyboardDevice ();
    
  void update();
	
	// Delay while updating until we are finished delaying
	void delay(long milli);
  
  //sendKeyStroke: sends a key press AND release
  void sendKeyStroke(uint8_t keyStroke);

  //sendKeyStroke: sends a key press AND release with modifiers
  void sendKeyStroke(uint8_t keyStroke, uint8_t modifiers);

  //sendKeyPress: sends a key press only - no release
  //To release the key, send again with keyPress=0
  void sendKeyPress(uint8_t keyPress);

  //sendKeyPress: sends a key press only, with modifiers - no release
  //To release the key, send again with keyPress=0
  void sendKeyPress(uint8_t keyPress, uint8_t modifiers);
  
  void RepeatKeyStroke(uint8_t stroke, size_t num);

  void RepeatKeyStroke(uint8_t stroke, uint8_t modifiers, size_t num);

  size_t write(uint8_t chr);

  uint8_t getLeds(void);

  using Print::write;

  static DigiKeyboardDevice& GetInstance();

  struct KeyboardReport{
    uint8_t modifier;
    uint8_t reserved;
    uint8_t keycode[6];
  };

  KeyboardReport reportBuffer;
  static DigiKeyboardDevice instance;
};

#endif // __DigiKeyboard_h__

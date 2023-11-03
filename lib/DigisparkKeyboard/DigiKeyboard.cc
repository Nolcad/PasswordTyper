#include "DigiKeyboard.h"

#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>

#include "usbdrv.h"

static uchar idleRate;
static uint8_t protocolVersion = 0;
static volatile uint8_t led_states = 0xff;

const PROGMEM unsigned char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
    /* USB report descriptor */
    0x05, 0x01, // USAGE_PAGE (Generic Desktop)
    0x09, 0x06, // USAGE (Keyboard)
    0xa1, 0x01, // COLLECTION (Application)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x95, 0x08, //   REPORT_COUNT (8)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0xe0, //   USAGE_MINIMUM (Keyboard LeftControl)(224)
    0x29, 0xe7, //   USAGE_MAXIMUM (Keyboard Right GUI)(231)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x01, //   LOGICAL_MAXIMUM (1)
    0x81, 0x02, //   INPUT (Data,Var,Abs) ; Modifier byte
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x81, 0x03, //   INPUT (Cnst,Var,Abs) ; Reserved byte
    0x95, 0x05, //   REPORT_COUNT (5)
    0x75, 0x01, //   REPORT_SIZE (1)
    0x05, 0x08, //   USAGE_PAGE (LEDs)
    0x19, 0x01, //   USAGE_MINIMUM (Num Lock)
    0x29, 0x05, //   USAGE_MAXIMUM (Kana)
    0x91, 0x02, //   OUTPUT (Data,Var,Abs) ; LED report
    0x95, 0x01, //   REPORT_COUNT (1)
    0x75, 0x03, //   REPORT_SIZE (3)
    0x91, 0x03, //   OUTPUT (Cnst,Var,Abs) ; LED report padding
    0x95, 0x06, //   REPORT_COUNT (6)
    0x75, 0x08, //   REPORT_SIZE (8)
    0x15, 0x00, //   LOGICAL_MINIMUM (0)
    0x25, 0x65, //   LOGICAL_MAXIMUM (101)
    0x05, 0x07, //   USAGE_PAGE (Keyboard)(Key Codes)
    0x19, 0x00, //   USAGE_MINIMUM (Reserved (no event indicated))(0)
    0x29, 0x65, //   USAGE_MAXIMUM (Keyboard Application)(101)
    0x81, 0x00, //   INPUT (Data,Ary,Abs)
    0xc0        // END_COLLECTION
};

DigiKeyboardDevice::DigiKeyboardDevice()
{
  cli();

  usbDeviceDisconnect();
  _delay_ms(250);
  usbDeviceConnect();
  usbInit();

  sei();

  memset(&reportBuffer, 0, sizeof(reportBuffer));
  usbSetInterrupt(reinterpret_cast<unsigned char *>(&reportBuffer), sizeof(reportBuffer));
}

void DigiKeyboardDevice::update()
{
  usbPoll();
}

void DigiKeyboardDevice::delay(long milli)
{
  unsigned long last = millis();
  while (milli > 0)
  {
    unsigned long now = millis();
    milli -= now - last;
    last = now;
    update();
  }
}

// sendKeyStroke: sends a key press AND release
void DigiKeyboardDevice::sendKeyStroke(uint8_t keyStroke)
{
  sendKeyStroke(keyStroke, 0);
}

// sendKeyStroke: sends a key press AND release with modifiers
void DigiKeyboardDevice::sendKeyStroke(uint8_t keyStroke, uint8_t modifiers)
{
  sendKeyPress(keyStroke, modifiers);
  // This stops endlessly repeating keystrokes:
  sendKeyPress(0, 0);
}

// sendKeyPress: sends a key press only - no release
// To release the key, send again with keyPress=0
void DigiKeyboardDevice::sendKeyPress(uint8_t keyPress)
{
  sendKeyPress(keyPress, 0);
}

// sendKeyPress: sends a key press only, with modifiers - no release
// To release the key, send again with keyPress=0
void DigiKeyboardDevice::sendKeyPress(uint8_t keyPress, uint8_t modifiers)
{
  while (!usbInterruptIsReady())
  {
    usbPoll();
    _delay_ms(5);
  }

  memset(&reportBuffer, 0, sizeof(reportBuffer));

  reportBuffer.modifier = modifiers;
  reportBuffer.keycode[0] = keyPress;

  usbSetInterrupt(reinterpret_cast<unsigned char *>(&reportBuffer), sizeof(reportBuffer));
}

uint8_t keycode_to_modifier(KEYCODE_TYPE keycode)
{
  uint8_t modifier = 0;

#ifdef SHIFT_MASK
  if (keycode & SHIFT_MASK)
    modifier |= MODIFIERKEY_SHIFT;
#endif
#ifdef ALTGR_MASK
  if (keycode & ALTGR_MASK)
    modifier |= MODIFIERKEY_RIGHT_ALT;
#endif
#ifdef RCTRL_MASK
  if (keycode & RCTRL_MASK)
    modifier |= MODIFIERKEY_RIGHT_CTRL;
#endif
  return modifier;
}

uint8_t keycode_to_key(KEYCODE_TYPE keycode)
{
  uint8_t key = keycode;
  // the only valid ASCII code > 63
  if (keycode != KEY_NON_US_BS)
  {
    key = key & 0x3F;
  }
  return key;
}

size_t DigiKeyboardDevice::write(uint8_t chr)
{
  uint8_t data = 0;
  if (chr == 0x09)
  {
    data = (uint8_t)KEY_TAB;
  }
  else if (chr == 0x0a)
  {
    data = (uint8_t)KEY_ENTER;
  }
  else if (chr >= 0x20)
  {
    // read from mapping table
    data = pgm_read_byte_near(keycodes_ascii + (chr - 0x20));
  }
  if (data)
  {
    sendKeyStroke(keycode_to_key(data), keycode_to_modifier(data));
  }
  return 1;
}

void DigiKeyboardDevice::RepeatKeyStroke(uint8_t stroke, size_t num)
{
  for (size_t i = 0; i < num; i++)
  {
    sendKeyStroke(stroke);
  }
}

void DigiKeyboardDevice::RepeatKeyStroke(uint8_t stroke, uint8_t modifiers, size_t num)
{
  for (size_t i = 0; i < num; i++)
  {
    sendKeyStroke(stroke, modifiers);
  }
}

uint8_t DigiKeyboardDevice::getLeds(void)
{
  return led_states;
}

DigiKeyboardDevice &DigiKeyboardDevice::GetInstance()
{
  return instance;
}

DigiKeyboardDevice DigiKeyboardDevice::instance = DigiKeyboardDevice();
DigiKeyboardDevice *keypoard_ptr = &DigiKeyboardDevice::GetInstance();

usbMsgLen_t usbFunctionSetup(uint8_t data[8])
{
  usbRequest_t *rq = (usbRequest_t *)((void *)data);

  if ((rq->bmRequestType & USBRQ_TYPE_MASK) != USBRQ_TYPE_CLASS)
    return 0;

  switch (rq->bRequest)
  {
  case USBRQ_HID_GET_IDLE:
    usbMsgPtr = &idleRate;
    return 1;
  case USBRQ_HID_SET_IDLE:
    idleRate = rq->wValue.bytes[1];
    return 0;
  case USBRQ_HID_GET_PROTOCOL:
    usbMsgPtr = &protocolVersion;
    return 1;
  case USBRQ_HID_SET_PROTOCOL:
    protocolVersion = rq->wValue.bytes[1];
    return 0;
  case USBRQ_HID_GET_REPORT:
    usbMsgPtr = reinterpret_cast<unsigned char *>(&(keypoard_ptr->reportBuffer));
    return sizeof(DigiKeyboardDevice::KeyboardReport);
  case USBRQ_HID_SET_REPORT:
    if (rq->wLength.word == 1)
    {
      return USB_NO_MSG;
    }
    else
    {
      return 0;
    }
  default:
    return 0;
  }
}

usbMsgLen_t usbFunctionWrite(uint8_t *data, uchar len)
{
  led_states = data[0];
  return 1;
}

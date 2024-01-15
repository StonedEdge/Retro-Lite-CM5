#include "gamepad.h"

// Serial Comms
uint8_t brightnessUp = 0x00;
uint8_t brightnessDn = 0x01;
uint8_t calibrationStepOne = 0x02;
uint8_t calibrationStepTwo = 0x03;
uint8_t calibrationComplete = 0x04;
uint8_t osKeyboardEnabled = 0x05;
uint8_t osKeyboardLeft = 0x06;
uint8_t osKeyboardRight = 0x07;
uint8_t osKeyboardUp = 0x08;
uint8_t osKeyboardDn = 0x09;
uint8_t osKeyboardSelect = 0x10;
uint8_t osKeyboardDisabled = 0x11;
uint8_t menuOpen = 0x12;
uint8_t menuClose = 0x13;
uint8_t povModeEnable = 0x14;
uint8_t povModeDisable = 0x15;
uint8_t mouseEnable = 0x16; // Currently unused. May be used in future to toggle mouse from OSD menu
uint8_t mouseDisable = 0x17; // Currently unused. May be used in future to toggle mouse from OSD menu
int serialButtonDelay = 150;

static uint8_t lastKey = 0;

void serialEvent()
{
  while (Serial.available()) {
    uint8_t key = 0;
    char inChar = (char)Serial.read();
    if (inChar == 27) { // Escape
      key = HID_KEY_ESCAPE;
    } else if (inChar == 8) { // Backspace
      key = HID_KEY_BACKSPACE;
    } else if (inChar == 13) { // Enter
      key = HID_KEY_ENTER;
    } else if (inChar == 14) { // Right
        key = HID_KEY_ARROW_RIGHT;
    } else if (inChar == 15) { // Left
        key = HID_KEY_ARROW_LEFT;
    } else if (inChar == 17) { // Up
        key = HID_KEY_ARROW_UP;
    } else if (inChar == 18) { // Down
        key = HID_KEY_ARROW_DOWN;
    } else if (inChar == calibrationStepOne) {
      calibrationMode = true;
    } else if (inChar == menuClose) {
      menuEnabled = false;
    } else if (inChar == povModeDisable) {
      povHatMode = false;
    } else if (inChar == povModeEnable) {
      povHatMode = true;
    } else {
        key = inChar;
    }

    if (lastKey != key) {
      tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &key, 2);
      lastKey = key;

      if (key != 0) {
        uint8_t keycode[6] = { 0 };
        keycode[0] = key;
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
      }
      else
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
    }
  }
}

void menuMode() {
  if (dpadPinsState[0] == 1) {
    serial_write(osKeyboardUp);
    delay(serialButtonDelay);
  } else if (dpadPinsState[2] == 1) {
    serial_write(osKeyboardDn);
    delay(serialButtonDelay);
  } else if (dpadPinsState[1] == 1) {
    serial_write(osKeyboardRight);
    delay(serialButtonDelay);
  } else if (dpadPinsState[3] == 1) {
    serial_write(osKeyboardLeft);
    delay(serialButtonDelay);
  } else if (buttonState[1] == 1) {
    serial_write(osKeyboardSelect);
    delay(serialButtonDelay);
  }
}

bool send_keyboard_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    if (menuEnabled) {
        serialEvent();
        menuMode();
    }

  if (buttonState[BTN_SELECT] && buttonState[BTN_R3]) { //If this combination of buttons is pressed, Open Menu. (Select and right joystick button)
    if (menuEnabled) {
      serial_write(menuClose);
      delay(200);
      menuEnabled = false;
    } else {
      serial_write(menuOpen);
      delay(200);
      menuEnabled = true;
    }
  }

    return false;
}

#include "bsp/board.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_descriptors.h"

// Serial Comms
byte brightnessUp = 0x00;
byte brightnessDn = 0x01;
byte calibrationStepOne = 0x02;
byte calibrationStepTwo = 0x03;
byte calibrationComplete = 0x04;
byte osKeyboardEnabled = 0x05;
byte osKeyboardLeft = 0x06;
byte osKeyboardRight = 0x07;
byte osKeyboardUp = 0x08;
byte osKeyboardDn = 0x09;
byte osKeyboardSelect = 0x10;
byte osKeyboardDisabled = 0x11;
byte menuOpen = 0x12;
byte menuClose = 0x13;
byte povModeEnable = 0x14;
byte povModeDisable = 0x15;
byte mouseEnable = 0x16; // Currently unused. May be used in future to toggle mouse from OSD menu
byte mouseDisable = 0x17; // Currently unused. May be used in future to toggle mouse from OSD menu
int serialButtonDelay = 150;

void serialEvent()
{
    /*
  while (Serial.available()) {
    uint8_t keycode[6] = { 0 };
    char inChar = (char)Serial.read();
    if (inChar == 27) { // Escape
      keycode[0] = HID_KEY_ESCAPE;
    } else if (inChar == 8) { // Backspace
      keycode[0] = HID_KEY_BACKSPACE;
    } else if (inChar == 13) { // Enter
      keycode[0] = HID_KEY_ENTER;
    } else if (inChar == 14) { // Right
        keycode[0] = HID_KEY_ARROW_RIGHT;
    } else if (inChar == 15) { // Left
        keycode[0] = HID_KEY_ARROW_LEFT;
    } else if (inChar == 17) { // Up
        keycode[0] = HID_KEY_ARROW_UP;
    } else if (inChar == 18) { // Down
        keycode[0] = HID_KEY_ARROW_DOWN;
    } else if (inChar == calibrationStepOne) {
      calibrationMode = true;
    } else if (inChar == menuClose) {
      menuEnabled = false;
    } else if (inChar == povModeDisable) {
      povHatMode = false;
    } else if (inChar == povModeEnable) {
      povHatMode = true;
    } else {
        keycode[0] = inChar;
    }

    if (keycode[0] != 0)
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
    else
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
  }
  */
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

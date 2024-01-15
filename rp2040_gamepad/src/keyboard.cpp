#include "gamepad.h"

int serialButtonsleep_ms = 150;

//static uint8_t lastKey = 0;

void serialEvent()
{
/*
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
            } else
                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        }
    }
*/
}

void menuMode()
{
/*
    if (dpadPinState[DPAD_UP] == 1) {
        serial_write(osKeyboardUp);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_DOWN] == 1) {
        serial_write(osKeyboardDn);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_RIGHT] == 1) {
        serial_write(osKeyboardRight);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_LEFT] == 1) {
        serial_write(osKeyboardLeft);
        sleep_ms(serialButtonsleep_ms);
    } else if (buttonState[BTN_A] == 1) {
        serial_write(osKeyboardSelect);
        sleep_ms(serialButtonsleep_ms);
    }
*/
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

    if (buttonState[BTN_SELECT]
        && buttonState[BTN_R3]) { // If this combination of buttons is pressed, Open Menu. (Select
                                  // and right joystick button)
        if (menuEnabled) {
            serial_write(menuClose);
            sleep_ms(200);
            menuEnabled = false;
        } else {
            serial_write(menuOpen);
            sleep_ms(200);
            menuEnabled = true;
        }
    }

    return false;
}

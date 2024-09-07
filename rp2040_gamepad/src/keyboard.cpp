#include "gamepad.h"

static const int serialButtonsleep_ms = 150;
static uint8_t lastKey = 0;
bool menuModeToggleStarted = false;

bool serialEvent() {
    while (tud_cdc_available() != 0) {
        uint8_t key = 0;
        char inChar = tud_cdc_read_char();
        switch (inChar) {
            case 27: key = HID_KEY_ESCAPE; break;
            case 8: key = HID_KEY_BACKSPACE; break;
            case 13: key = HID_KEY_ENTER; break;
            case 14: key = HID_KEY_ARROW_RIGHT; break;
            case 15: key = HID_KEY_ARROW_LEFT; break;
            case 17: key = HID_KEY_ARROW_UP; break;
            case 18: key = HID_KEY_ARROW_DOWN; break;
            case calibrationStepOne: calibrationMode = true; break;
            case menuClose: menuEnabled = false; break;
            case povModeDisable: povHatMode = false; break;
            case povModeEnable: povHatMode = true; break;
            default: key = inChar; break;
        }

        if (key != 0) {
            uint8_t keycode[6] = { 0 };
            keycode[0] = key;
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
            lastKey = key;
            return true;
        }
    }

    if (lastKey != 0) {
        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        lastKey = 0;
        return true;
    }

    return false;
}

void menuMode()
{
    if (dpadPinState[DPAD_UP] == 1) {
        tud_cdc_write_char(osKeyboardUp);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_DOWN] == 1) {
        tud_cdc_write_char(osKeyboardDn);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_RIGHT] == 1) {
        tud_cdc_write_char(osKeyboardRight);
        sleep_ms(serialButtonsleep_ms);
    } else if (dpadPinState[DPAD_LEFT] == 1) {
        tud_cdc_write_char(osKeyboardLeft);
        sleep_ms(serialButtonsleep_ms);
    } else if (buttonState[BTN_A] == 1) {
        tud_cdc_write_char(osKeyboardSelect);
        sleep_ms(serialButtonsleep_ms);
    }
}

uint16_t get_keyboard_report(uint8_t* buffer, uint16_t reqlen)
{
    if (reqlen >= 6) {
        memset(buffer, 0, 6);
        return 6;
    }

    return 0;
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
        if (serialEvent())
            return true;

        if (menuEnabled)
            menuMode();
    }

    // If this combination of buttons is pressed, Open Menu.
    // (Select and Hotkey+)
    if (menuEnabled.changed(buttonState[BTN_HOTKEY_PLUS] && buttonState[BTN_SELECT])) {
        if (menuEnabled)
            tud_cdc_write_char(menuOpen);
        else
            tud_cdc_write_char(menuClose);
    }

    return false;
}

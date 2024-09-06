#include "gamepad.h"

static uint16_t lastKey = 0;

uint16_t get_consumer_report(uint8_t* buffer, uint16_t reqlen)
{
    if (reqlen >= 2) {
        *(uint16_t*)buffer = 0;
        return 2;
    }

    return 0;
}

bool send_consumer_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    uint16_t key = 0;
    if (buttonState[BTN_HOTKEY_PLUS]) {
        if (dpadPinState[DPAD_UP])
            key = HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT;
        else if (dpadPinState[DPAD_DOWN])
            key = HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT;
        else if (dpadPinState[DPAD_LEFT])
            key = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        else if (dpadPinState[DPAD_RIGHT])
            key = HID_USAGE_CONSUMER_VOLUME_INCREMENT;
    }

    if (lastKey != key) {
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &key, 2);
        lastKey = key;
        return true;
    }

    return false;
}

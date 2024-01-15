#include "bsp/board.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_descriptors.h"

// use to avoid send multiple consecutive zero report
static bool has_consumer_key = false;

bool send_consumer_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    // volume down
    // uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
    // tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);

    // send empty key report (release key) if previously has key pressed
    // uint16_t empty_key = 0;
    // tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);


/*
  if (buttonState[BTN_SELECT] && dpadPinsState[DPAD_UP]) { 
    serial_write(brightnessUp);
    delay(serialButtonDelay);
  }
  if (buttonState[BTN_SELECT] && dpadPinsState[DPAD_DOWN]) {
    serial_write(brightnessDn);
    delay(serialButtonDelay);
  }
*/

    return false;
}

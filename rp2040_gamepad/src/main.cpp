#include "gamepad.h"
#include "hardware/i2c.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void mouse_init();
void gamepad_init();

bool send_keyboard_report();
bool send_mouse_report();
bool send_consumer_report();
bool send_gamepad_report();
bool send_multiaxis_report();

uint16_t get_keyboard_report(uint8_t* buffer, uint16_t reqlen);
uint16_t get_mouse_report(uint8_t* buffer, uint16_t reqlen);
uint16_t get_consumer_report(uint8_t* buffer, uint16_t reqlen);
uint16_t get_gamepad_report(uint8_t* buffer, uint16_t reqlen);
uint16_t get_multiaxis_report(uint8_t* buffer, uint16_t reqlen);

static uint8_t kbd_leds = 0;

// Invoked when device is mounted
void tud_mount_cb(void) { }

// Invoked when device is unmounted
void tud_umount_cb(void) { }

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }

// Invoked when usb bus is resumed
void tud_resume_cb(void) { }

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) 
{
	(void)itf;
	(void)rts;
    (void)dtr;
}

static void send_hid_report(uint8_t report_id)
{
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    switch (report_id) {
    case REPORT_ID_KEYBOARD:
        if (send_keyboard_report())
            break;

    case REPORT_ID_MOUSE:
        if (send_mouse_report())
            break;

    case REPORT_ID_CONSUMER_CONTROL:
        if (send_consumer_report())
            break;

    case REPORT_ID_GAMEPAD:
        if (send_gamepad_report())
            break;

    case REPORT_ID_MULTI_AXIS:
        if (send_multiaxis_report())
            break;

    default:
        break;
    }
}

// Every 10ms, we will send 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
    // Remote wakeup
    if (tud_suspended()) {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        if (checkButtons())
            tud_remote_wakeup();
        else
            sleep_ms(10);
    } else {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_KEYBOARD);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    (void)instance;
    (void)len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT) {
        send_hid_report(next_report_id);
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
    uint8_t* buffer, uint16_t reqlen)
{
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        if (report_id == REPORT_ID_KEYBOARD && reqlen >= 1) {
            buffer[0] = kbd_leds;
            return 1;
        }
        return 0;
    }

    switch (report_id) {
    case REPORT_ID_KEYBOARD:
        return get_keyboard_report(buffer, reqlen);

    case REPORT_ID_MOUSE:
        return get_mouse_report(buffer, reqlen);

    case REPORT_ID_CONSUMER_CONTROL:
        return get_consumer_report(buffer, reqlen);

    case REPORT_ID_GAMEPAD:
        return get_gamepad_report(buffer, reqlen);

    case REPORT_ID_MULTI_AXIS:
        return get_multiaxis_report(buffer, reqlen);

    default:
        break;
    }

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type,
    uint8_t const* buffer, uint16_t bufsize)
{
    (void)instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT) {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD) {
            // bufsize should be (at least) 1
            if (bufsize < 1)
                return;

            kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK) {
                // Capslock On
            } else {
                // Caplocks Off
            }
        }
    }
}

int main()
{
    stdio_init_all();

    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    board_init();
    tusb_init();
    gamepad_init();
    mouse_init();

    while (1) {
        tud_task(); // tinyusb device task
        hid_task();
    }
}

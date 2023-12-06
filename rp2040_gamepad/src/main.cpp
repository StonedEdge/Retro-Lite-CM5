
#define PICO_DEFAULT_I2C 1
#define PICO_DEFAULT_I2C_SDA_PIN 18
#define PICO_DEFAULT_I2C_SCL_PIN 19

#include "imu/MPU6050.h"
#include "tracking/main_loop.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

MPU6050 mpu;

int xAccelOffset = 149;
int yAccelOffset = -684;
int zAccelOffset = 865;
int xGyroOffset = 50;
int yGyroOffset = -33;
int zGyroOffset = -11;

static const double DEG_TO_RAD = 0.017453292519943295769236907684886;
static const double G = 9.81;

void calibrateIMU(MPU6050 *accelgyro, int offsets[6]);

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
    blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
    (void) remote_wakeup_en;
    blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
    blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
    // skip if hid is not ready yet
    if ( !tud_hid_ready() ) return;

    switch(report_id)
    {
        case REPORT_ID_KEYBOARD:
        {
            // use to avoid send multiple consecutive zero report for keyboard
            static bool has_keyboard_key = false;

            if ( btn )
            {
                uint8_t keycode[6] = { 0 };
                keycode[0] = HID_KEY_A;

                tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
                has_keyboard_key = true;
            }else
            {
                // send empty key report if previously has key pressed
                if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
                has_keyboard_key = false;
            }
        }
        break;

        case REPORT_ID_MOUSE:
        {
            int8_t const delta = 5;

            // no button, right + down, no scroll, no pan
            tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, delta, delta, 0, 0);
        }
        break;

        case REPORT_ID_CONSUMER_CONTROL:
        {
            // use to avoid send multiple consecutive zero report
            static bool has_consumer_key = false;

            if ( btn )
            {
                // volume down
                uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
                tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
                has_consumer_key = true;
            }else
            {
                // send empty key report (release key) if previously has key pressed
                uint16_t empty_key = 0;
                if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
                has_consumer_key = false;
            }
        }
        break;

        case REPORT_ID_GAMEPAD:
        {
            // use to avoid send multiple consecutive zero report for keyboard
            static bool has_gamepad_key = false;

            hid_gamepad_report_t report =
            {
                .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
                .hat = 0, .buttons = 0
            };

            if ( btn )
            {
                report.hat = GAMEPAD_HAT_UP;
                report.buttons = GAMEPAD_BUTTON_A;
                tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

                has_gamepad_key = true;
            }else
            {
                report.hat = GAMEPAD_HAT_CENTERED;
                report.buttons = 0;
                if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
                has_gamepad_key = false;
            }
        }
        break;

        default: break;
    }
}

// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if ( board_millis() - start_ms < interval_ms) return; // not enough time
    start_ms += interval_ms;

    uint32_t const btn = board_button_read();

    // Remote wakeup
    if ( tud_suspended() && btn )
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    }else
    {
        // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
        send_hid_report(REPORT_ID_KEYBOARD, btn);
    }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
    (void) instance;
    (void) len;

    uint8_t next_report_id = report[0] + 1;

    if (next_report_id < REPORT_ID_COUNT)
    {
        send_hid_report(next_report_id, board_button_read());
    }
}

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
    // TODO not Implemented
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    return 0;
}

// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
    (void) instance;

    if (report_type == HID_REPORT_TYPE_OUTPUT)
    {
        // Set keyboard LED e.g Capslock, Numlock etc...
        if (report_id == REPORT_ID_KEYBOARD)
        {
            // bufsize should be (at least) 1
            if ( bufsize < 1 ) return;

            uint8_t const kbd_leds = buffer[0];

            if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
            {
                // Capslock On: disable blink, turn led on
                blink_interval_ms = 0;
                board_led_write(true);
            }else
            {
                // Caplocks Off: back to normal blink
                board_led_write(false);
                blink_interval_ms = BLINK_MOUNTED;
            }
        }
    }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
    static uint32_t start_ms = 0;
    static bool led_state = false;

    // blink is disabled
    if (!blink_interval_ms) return;

    // Blink every interval ms
    if (board_millis() - start_ms < blink_interval_ms) return; // not enough time
    start_ms += blink_interval_ms;

    board_led_write(led_state);
    led_state = 1 - led_state; // toggle
}

bool mouse_cb(int8_t x, int8_t y, void* context) {
    // Remote wakeup
    if (tud_suspended())
    {
        // Wake up host if we are in suspend mode
        // and REMOTE_WAKEUP feature is enabled by host
        tud_remote_wakeup();
    } else if (tud_hid_ready()) {
        tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, x, y, 0, 0);
    }
    return true;
}

void mpu_task(void) {
    static uint32_t start_ms = 0;
    if (board_millis() - start_ms < 10) return; // not enough time
    start_ms += 10;

    tracking_step(mouse_cb, nullptr);
}

int imu_read(double* vec) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    
    vec[0] = ((double)ax * 4 / 32768) * G;
    vec[1] = ((double)ay * 4 / 32768) * G;
    vec[2] = ((double)az * 4 / 32768) * G;
    vec[3] = ((double)gx * 1000 / 32768) * DEG_TO_RAD;
    vec[4] = ((double)gy * 1000 / 32768) * DEG_TO_RAD;
    vec[5] = ((double)gz * 1000 / 32768) * DEG_TO_RAD;

    //printf("%f, %f, %f \t %f, %f, %f\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);

    return ACC_DATA_READY | GYR_DATA_READY;
}

int main() {
    stdio_init_all();

    // This example will use I2C0 on the default SDA and SCL pins (4, 5 on a Pico)
    i2c_init(i2c_default, 400 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    // Make the I2C pins available to picotool
    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));

    board_init();
    tusb_init();

    sleep_ms(100);
    mpu.initialize();
    sleep_ms(100);

    if (mpu.testConnection()) {
        printf("Connected!\n");
        mpu.setXAccelOffset(xAccelOffset);
        mpu.setYAccelOffset(yAccelOffset);
        mpu.setZAccelOffset(zAccelOffset);
        mpu.setXGyroOffset(xGyroOffset);
        mpu.setYGyroOffset(yGyroOffset);
        mpu.setZGyroOffset(zGyroOffset);

        /*
        // Calibration Time: generate offsets and calibrate our MPU6050
        int offset[6];
        calibrateIMU(&mpu, offset); // takes a couple minutes to complete
        printf("%d\t%d\t%d\n%d\t%d\t%d\n", offset[0], offset[1], offset[2], offset[3], offset[4], offset[5]);
        */

        sleep_ms(100);
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        sleep_ms(100);

        printf("Connected! Accel range: %d, Gyro range: %d\n",
            mpu.getFullScaleAccelRange(), mpu.getFullScaleGyroRange());

        tracking_begin();
    } else {
        printf("Not connected\n");
    }

    while (1) {
        tud_task(); // tinyusb device task
        led_blinking_task();
        //hid_task();
        mpu_task();
    }
}

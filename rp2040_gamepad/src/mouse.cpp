#include "gamepad.h"
#include "imu/MPU6050.h"
#include "tracking/main_loop.h"

// Mouse Variables
Toggle mouseEnabled;
Toggle mouseCalibration;

// Adjust this value to control mouse sensitivity. Higher number = slower response.
int mouseDivider = 8;
int mouseInterval = 10; // Interval in MS between mouse updates
bool calibrateMPU = false;

MPU6050 mpu;

int xAccelOffset = 149;
int yAccelOffset = -684;
int zAccelOffset = 865;
int xGyroOffset = 50;
int yGyroOffset = -33;
int zGyroOffset = -11;

static const double DEG_TO_RAD = 0.017453292519943295769236907684886;
static const double G = 9.81;

void calibrateIMU(MPU6050* accelgyro, int offsets[6]);
void quickCalibrateIMU(MPU6050* accelgyro, int offsets[6]);

static int16_t g_imu[6];
uint32_t lastMultiaxisReport = -1;
uint32_t lastMotionTime = -1;

int imu_read(double* vec)
{
    lastMotionTime = board_millis();
    mpu.getMotion6(&g_imu[0], &g_imu[1], &g_imu[2], &g_imu[3], &g_imu[4], &g_imu[5]);

    vec[0] = -((double)g_imu[0] * 4 / 32768) * G;
    vec[1] = ((double)g_imu[2] * 4 / 32768) * G;
    vec[2] = ((double)g_imu[1] * 4 / 32768) * G;
    vec[3] = -((double)g_imu[3] * 1000 / 32768) * DEG_TO_RAD;
    vec[4] = ((double)g_imu[5] * 1000 / 32768) * DEG_TO_RAD;
    vec[5] = ((double)g_imu[4] * 1000 / 32768) * DEG_TO_RAD;

    // printf("%f, %f, %f \t %f, %f, %f\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);

    return ACC_DATA_READY | GYR_DATA_READY;
}

void mouseControl()
{
    // Calculate Y Value
    // Divide signal by the mouseDivider for mouse level output
    int yMove = mapJoystick(JOY_LEFT_Y) / mouseDivider;

    // Calculate X Value
    int xMove = mapJoystick(JOY_LEFT_X) / mouseDivider;

    int mouseButtons = buttonState[BTN_L1] | (buttonState[BTN_R1] << 1);
    tud_hid_mouse_report(REPORT_ID_MOUSE, mouseButtons, xMove, yMove, 0, 0);
}

void calibrate_mpu() {
    // Calibration Time: generate offsets and calibrate our MPU6050
    int offset[6];
    quickCalibrateIMU(&mpu, offset); // takes a couple minutes to complete
    xAccelOffset = offset[0];
    yAccelOffset = offset[1];
    zAccelOffset = offset[2];
    xGyroOffset = offset[3];
    yGyroOffset = offset[4];
    zGyroOffset = offset[5];
    writeJoystickConfig();
}

void mouse_init()
{
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

        if (calibrateMPU)
            calibrate_mpu();

        sleep_ms(100);
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        sleep_ms(100);

        printf("Connected! Accel range: %d, Gyro range: %d\n", mpu.getFullScaleAccelRange(),
            mpu.getFullScaleGyroRange());
        printf("%d\t%d\t%d\n%d\t%d\t%d\n", xAccelOffset, yAccelOffset, zAccelOffset,
            xGyroOffset, yGyroOffset, zGyroOffset);
    } else
        printf("Not connected\n");

    mpu.setSleepEnabled(true);
}

void mouse_cb(int8_t x, int8_t y) {
    int mouseButtons = buttonState[BTN_L1] | (buttonState[BTN_R1] << 1);
    tud_hid_mouse_report(REPORT_ID_MOUSE, mouseButtons, x, y, 0, 0);
}

uint16_t get_mouse_report(uint8_t* buffer, uint16_t reqlen)
{
    hid_mouse_report_t report = {
        .buttons = (uint8_t)(buttonState[BTN_L1] | (buttonState[BTN_R1] << 1)),
        .x       = 0,
        .y       = 0,
        .wheel   = 0,
        .pan     = 0
    };

    if (reqlen >= sizeof(report)) {
        memcpy(buffer, &report, sizeof(report));
        return sizeof(report);
    }

    return 0;
}

bool send_mouse_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    // Hotkey+ and L3 toggles the mouse cursor to an on/off state
    if (mouseEnabled.changed(buttonState[BTN_HOTKEY_PLUS] && buttonState[BTN_L3])) {
        if (mouseEnabled) {
            mpu.setSleepEnabled(false);
            sleep_ms(100);
            tracking_begin();
        }
        else {
            mpu.setSleepEnabled(true);
            sleep_ms(100);
            tracking_end();
        }
    }

    if (!mouseEnabled)
        return false;

    if (mouseCalibration.changed(buttonState[BTN_HOTKEY_PLUS] && buttonState[BTN_HOTKEY_MINUS])) {
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_2);
        sleep_ms(5000); // Allow user some time to put the handheld screen down;
        calibrate_mpu();
        sleep_ms(100);
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        mouseCalibration = false;
        return false;
    }

    tracking_step(mouse_cb);
    // mouseControl();

    return true;
}

uint16_t get_multiaxis_report(uint8_t* buffer, uint16_t reqlen)
{
    if (reqlen >= 12) {
        memcpy(buffer, g_imu, 12);
        return 12;
    }

    return 0;
}

bool send_multiaxis_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    if (lastMultiaxisReport == lastMotionTime)
        return false;

    tud_hid_report(REPORT_ID_MULTI_AXIS, g_imu, 12);
    lastMultiaxisReport = lastMotionTime;

    return true;
}

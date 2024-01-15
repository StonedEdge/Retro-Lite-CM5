#include "imu/MPU6050.h"
#include "tracking/main_loop.h"
#include "gamepad.h"

// Mouse Variables
bool mouseEnabled = false;
int mouseDivider = 8; // Adjust this value to control mouse sensitivity. Higher number = slower response.
unsigned long mouseTimer;
int mouseInterval = 10; // Interval in MS between mouse updates
unsigned long mouseModeTimer;
bool mouseModeTimerStarted = false;

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

static int16_t g_imu[6];

int imu_read(double* vec)
{
    mpu.getMotion6(&g_imu[0], &g_imu[1], &g_imu[2], &g_imu[3], &g_imu[4], &g_imu[5]);

    vec[0] = ((double)g_imu[0] * 4 / 32768) * G;
    vec[1] = ((double)g_imu[1] * 4 / 32768) * G;
    vec[2] = ((double)g_imu[2] * 4 / 32768) * G;
    vec[3] = ((double)g_imu[3] * 1000 / 32768) * DEG_TO_RAD;
    vec[4] = ((double)g_imu[4] * 1000 / 32768) * DEG_TO_RAD;
    vec[5] = ((double)g_imu[5] * 1000 / 32768) * DEG_TO_RAD;

    // printf("%f, %f, %f \t %f, %f, %f\n", vec[0], vec[1], vec[2], vec[3], vec[4], vec[5]);

    return ACC_DATA_READY | GYR_DATA_READY;
}

void mouseControl() {
  int var;

  // Calculate Y Value
  var = readJoystick(leftJoyY, invertLeftY); // read raw input
  var = (var - minLeftY) / 2; // scale to fit axisMap
  var = leftYaxisMap(var); // Read value from axisMap
  var = var - 127; // Shift to 0 centre.
  int yMove = var / mouseDivider; // Divide signal by the mouseDivider for mouse level output
 
  // Calculate X Value
  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  var = leftXaxisMap(var);
  var = var - 127;
  int xMove = var / mouseDivider;

  int mouseButtons = buttonState[BTN_L1] | (buttonState[BTN_R1] << 1);
    tud_hid_mouse_report(REPORT_ID_MOUSE, mouseButtons, xMove, yMove, 0, 0);
}

void mouse_cb(int8_t x, int8_t y)
{
    tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, x, y, 0, 0);
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

        /*
        // Calibration Time: generate offsets and calibrate our MPU6050
        int offset[6];
        calibrateIMU(&mpu, offset); // takes a couple minutes to complete
        printf("%d\t%d\t%d\n%d\t%d\t%d\n", offset[0], offset[1], offset[2], offset[3], offset[4],
        offset[5]);
        */

        sleep_ms(100);
        mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
        mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
        sleep_ms(100);

        printf("Connected! Accel range: %d, Gyro range: %d\n", mpu.getFullScaleAccelRange(),
            mpu.getFullScaleGyroRange());

        tracking_begin();
    } else {
        printf("Not connected\n");
    }
}

bool send_mouse_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    // Mouse Toggle
    if (buttonState[BTN_SELECT] && buttonState[BTN_R3]) { // Left joystick click toggles the mouse cursor to an on/off state
        if (mouseModeTimerStarted) {
            if (board_millis() > mouseModeTimer + 2000) {
                mouseEnabled = !mouseEnabled;
                mouseModeTimerStarted = false;
            }
        } else {
            mouseModeTimerStarted = true;
            mouseModeTimer = board_millis();
        }
    } else {
        mouseModeTimerStarted = false;
    }

    if (!mouseEnabled)
        return false;

    tracking_step(mouse_cb);

    // mouseControl();

    return true;
}

bool send_multiaxis_report()
{
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    tud_hid_report(REPORT_ID_MULTI_AXIS, g_imu, 12);
    return true;
}
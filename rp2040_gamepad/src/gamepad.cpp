#include "gamepad.h"
#include "hardware/adc.h"
#include "pico/stdlib.h"
#include <algorithm>

#define PIN_A 0
#define PIN_B 1
#define PIN_X 2
#define PIN_Y 25
#define PIN_START 17
#define PIN_SELECT 11
#define PIN_D_UP 12
#define PIN_D_DOWN 13
#define PIN_D_LEFT 14
#define PIN_D_RIGHT 15
#define PIN_L2_HALF 20
#define PIN_R2_HALF 8
#define PIN_L2_FULL 21
#define PIN_R2_FULL 7
#define PIN_R1 9
#define PIN_L1 10
#define PIN_L3 16
#define PIN_R3 3
#define PIN_HOTKEY_MINUS 24
#define PIN_HOTKEY_PLUS 16

// Define buttons
uint8_t buttonPins[buttonCount] = {
    PIN_A, PIN_B, PIN_X, PIN_Y, PIN_START, PIN_SELECT, PIN_L2_HALF, PIN_R2_HALF, PIN_L2_FULL,
    PIN_R2_FULL, PIN_R1, PIN_L1, PIN_L3, PIN_R3, PIN_HOTKEY_MINUS, PIN_HOTKEY_PLUS
}; // Array to store digital pins used for buttons. Length must be the same as buttonCount.

uint8_t dpadPins[4] = { PIN_D_UP, PIN_D_RIGHT, PIN_D_DOWN,
    PIN_D_LEFT }; // Up, Right, Down, Left. Do not change order of directions.

// Button state arrays
bool dpadPinState[4];
bool buttonState[buttonCount];
bool newButtonState[buttonCount];

const int joystickPins[4] = { 26, 27, 28, 29 };

// Joystick Settings
// Change these settings for Inverted mounting of joysticks.
bool invertOutput[4] = { true, true, false, false };

// Joystick deadband settings. Deadband is the same for both axis on each joystick.
int axisDeadband[4] = { 10, 10, 10, 10 };

// Distance from end of travel to achieve full axis movement.
// This helps square out each axis response to allow full movement speed with direction input.
int axisEarly[4] = { 30, 30, 30, 30 };

const bool useDeadband = true;

// Enable joystick scaling. Needed for switch joysticks due to uneven axis travels.
// Disabling will save some compute time if your joystick works well without it.
bool scaledJoystickOutput = true;

int axisMin[4] = { 215, 300, 205, 330 };
int axisMid[4] = { 529, 525, 477, 512 };
int axisMax[4] = { 730, 780, 635, 830 };

// All variables below general use, not used for configuration.
bool calibrationMode = false;
int calibrationStep = 1;
// unsigned long keyboardTimer;
bool L3Pressed = false;
bool menuEnabled = false;
bool povHatMode = true; // Enable to use POV Hat for Dpad instead of analog

hid_gamepad_report_t joystick;
hid_gamepad_report_t lastState;

static inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t axisMap(int i, int minIn, int midIn, int maxIn, int earlyStop, int deadBand)
{
    // Shift all joystick values to a base of zero. All values are halved due to ram limitations on
    // the 32u4.
    int shiftedMin = 0;
    int shiftedMid = (midIn - minIn) / 2;
    int shiftedMax = (maxIn - minIn) / 2;

    if (i < shiftedMid + deadBand && i > shiftedMid - deadBand)
        return 127;
    if (i < shiftedMid) {
        if (i > shiftedMin + earlyStop)
            return map(i, shiftedMin, shiftedMid - deadBand, 0, 127);
        return 0;
    }
    if (i < shiftedMax - earlyStop)
        return map(i, shiftedMid + deadBand, shiftedMax, 127, 254);
    return 254;
}

uint8_t mapJoystick(int axis)
{
    int var = readJoystick(axis);
    var = (var - axisMin[axis]) / 2;
    return axisMap(
        var, axisMin[axis], axisMid[axis], axisMax[axis], axisEarly[axis], axisDeadband[axis]);
}

int readJoystick(int joystickAxis)
{ // Reads raw joystick values and inverts if required
    adc_select_input(joystickAxis);
    int var = adc_read();
    if (invertOutput[joystickAxis]) {
        var = 1023 - var;
        return var;
    } else {
        return var;
    }
}

void serial_write(uint8_t b)
{
    (void)b;

    // TODO
}

void buttonRead()
{ // Read button inputs and set state arrays.
    for (int i = 0; i < buttonCount; i++)
        buttonState[i] = !gpio_get(buttonPins[i]);

    for (int i = 0; i < 4; i++)
        dpadPinState[i] = !gpio_get(dpadPins[i]);
}

void joypadButtons()
{ // Set joystick buttons for USB output
    for (int i = 0; i < buttonCount; i++) {
        if (newButtonState[i] != buttonState[i]) {
            joystick.buttons = (joystick.buttons & ~(1 << i)) | ((uint32_t)buttonState[i] << i);
            newButtonState[i] = buttonState[i];
        }
    }
}

void dPadInput()
{ // D-Pad as RY and RZ Axis
    if (!povHatMode) {
        if (dpadPinState[DPAD_UP] && !buttonState[BTN_SELECT])
            joystick.ry = 127;
        else if (dpadPinState[DPAD_DOWN] && !buttonState[BTN_SELECT])
            joystick.ry = -127;
        else
            joystick.ry = 0;

        if (dpadPinState[DPAD_RIGHT])
            joystick.rz = 127;
        else if (dpadPinState[DPAD_LEFT])
            joystick.rz = -127;
        else
            joystick.rz = 0;
    } else { // POV Hat Mode
        joystick.hat = GAMEPAD_HAT_CENTERED;
        if (dpadPinState[DPAD_UP] && !buttonState[BTN_SELECT]) {
            if (dpadPinState[DPAD_RIGHT])
                joystick.hat = GAMEPAD_HAT_UP_RIGHT;
            else if (dpadPinState[DPAD_LEFT])
                joystick.hat = GAMEPAD_HAT_UP_LEFT;
            else
                joystick.hat = GAMEPAD_HAT_UP;
        } else if (dpadPinState[DPAD_DOWN] && !buttonState[BTN_SELECT]) {
            if (dpadPinState[DPAD_RIGHT])
                joystick.hat = GAMEPAD_HAT_DOWN_RIGHT;
            else if (dpadPinState[DPAD_LEFT])
                joystick.hat = GAMEPAD_HAT_DOWN_LEFT;
            else
                joystick.hat = GAMEPAD_HAT_DOWN;
        } else if (dpadPinState[DPAD_RIGHT])
            joystick.hat = GAMEPAD_HAT_RIGHT;
        else if (dpadPinState[DPAD_LEFT])
            joystick.hat = GAMEPAD_HAT_LEFT;
    }
}
/*
void writeIntIntoEEPROM(int address, int number) { // Splits Int into uint8_tS for EEPROM
  uint8_t uint8_t1 = number >> 8;
  uint8_t uint8_t2 = number & 0xFF;
  EEPROM.write(address, uint8_t1);
  EEPROM.write(address + 1, uint8_t2);
}

int readIntFromEEPROM(int address) { // Converts uint8_tS to INT from EEPROM
  uint8_t uint8_t1 = EEPROM.read(address);
  uint8_t uint8_t2 = EEPROM.read(address + 1);
  return (uint8_t1 << 8) + uint8_t2;
}
*/
void readJoystickConfig()
{ // Read joystick calibration from EEPROM
    /*
    // Left X
      minLeftX = readIntFromEEPROM(1);
      maxLeftX = readIntFromEEPROM(3);
      midLeftX = readIntFromEEPROM(5);
    // Left Y
      minLeftY = readIntFromEEPROM(7);
      maxLeftY = readIntFromEEPROM(9);
      midLeftY = readIntFromEEPROM(11);
    // Right Y
      minRightY = readIntFromEEPROM(13);
      maxRightY = readIntFromEEPROM(15);
      midRightY = readIntFromEEPROM(17);
    // Right X
      minRightX = readIntFromEEPROM(19);
      maxRightX = readIntFromEEPROM(21);
      midRightX = readIntFromEEPROM(23);

    // IMU calibration data
      xAccelOffset = readIntFromEEPROM(25);
      yAccelOffset = readIntFromEEPROM(27);
      zAccelOffset = readIntFromEEPROM(29);
      xGyroOffset = readIntFromEEPROM(31);
      yGyroOffset = readIntFromEEPROM(33);
      zGyroOffset = readIntFromEEPROM(35);
    */
}

void writeJoystickConfig()
{ // Store joystick calibration in EEPROM
    /*
    // Left X
      writeIntIntoEEPROM(1, minLeftX);
      writeIntIntoEEPROM(3, maxLeftX);
      writeIntIntoEEPROM(5, midLeftX);
    // Left Y
      writeIntIntoEEPROM(7, minLeftY);
      writeIntIntoEEPROM(9, maxLeftY);
      writeIntIntoEEPROM(11, midLeftY);
    // Right Y
      writeIntIntoEEPROM(13, minRightY);
      writeIntIntoEEPROM(15, maxRightY);
      writeIntIntoEEPROM(17, midRightY);
    // Right X
      writeIntIntoEEPROM(19, minRightX);
      writeIntIntoEEPROM(21, maxRightX);
      writeIntIntoEEPROM(23, midRightX);

    // IMU calibration data
      writeIntIntoEEPROM(25, xAccelOffset);
      writeIntIntoEEPROM(27, yAccelOffset);
      writeIntIntoEEPROM(29, zAccelOffset);
      writeIntIntoEEPROM(31, xGyroOffset);
      writeIntIntoEEPROM(33, yGyroOffset);
      writeIntIntoEEPROM(35, zGyroOffset);
    */
}

void eepromLoad()
{ // Loads stored settings from EEPROM
    /*
      if (readIntFromEEPROM(1) != -1) { // Check Joystick Calibration in EEPROM is not Empty
        readJoystickConfig(); // Load joystick calibration from EEPROM
      }
    */
}

void joystickCalibration()
{ // Very rough at the moment but it works. Read usage instructions at top of page.
    if (calibrationStep == 1) {
        if (buttonState[BTN_A]) {
            // RXLED1;
            sleep_ms(100);
            // RXLED0;
            for (int i = 0; i < 4; i++)
                axisMid[i] = readJoystick(i);
            calibrationStep = 2;
            serial_write(calibrationStepTwo);
            sleep_ms(50);
        }
    } else if (calibrationStep == 2) {
        // RXLED1;
        sleep_ms(100);
        // RXLED0;
        for (int i = 0; i < 4; i++) {
            axisMin[i] = axisMid[i];
            axisMax[i] = 0;
        }
        calibrationStep = 3;
        sleep_ms(500);
    } else if (calibrationStep == 3) {
        for (int i = 0; i < 4; i++) {
            int var = readJoystick(i);
            axisMin[i] = std::min(axisMin[i], var);
            axisMax[i] = std::max(axisMax[i], var);
        }
        if (buttonState[BTN_A]) { // Complete Calibration
            // RXLED1;
            sleep_ms(100);
            // RXLED0;
            sleep_ms(200);
            writeJoystickConfig(); // Update EEPROM
            serial_write(calibrationComplete);
            sleep_ms(50);
            calibrationStep = 1;
            calibrationMode = false;
        }
    }
}

void joystickInput()
{
    joystick.rx = mapJoystick(JOY_RIGHT_Y);
    joystick.z = mapJoystick(JOY_RIGHT_X);
    joystick.y = mapJoystick(JOY_LEFT_Y);
    joystick.x = mapJoystick(JOY_LEFT_X);
}

void pinModeInputPullup(int pin)
{
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void gamepad_init()
{
    for (int i = 0; i < buttonCount;
         i++) // Set all button pins as input pullup. Change to INPUT if using external resistors.
        pinModeInputPullup(buttonPins[i]);

    for (int i = 0; i < 4; i++) // Set all dpad button pins as input pullup.
        pinModeInputPullup(dpadPins[i]);

    for (int i = 0; i < 4; i++)
        adc_gpio_init(joystickPins[i]);

    adc_init();

    eepromLoad(); // Check for stored joystick settings and load if applicable.
}

bool send_gamepad_report()
{
    // Poll every 5ms
    const uint32_t interval_ms = 5;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms)
        return false; // not enough time
    start_ms += interval_ms;

    buttonRead();

    if (calibrationMode) {
        joystickCalibration();
    } else if (!menuEnabled) {
        joypadButtons();
        joystickInput();
        dPadInput();
    }

    if (buttonState[BTN_START] && buttonState[BTN_R3]) { // Start+R3 for jostick calibration
        calibrationStep = 1;
        calibrationMode = true;
    }

    if (memcmp(&lastState, &joystick, sizeof(joystick)) && !calibrationMode && !menuEnabled) {
        tud_hid_report(REPORT_ID_GAMEPAD, &joystick, sizeof(joystick));
        lastState = joystick;
        return true;
    }

    return false;
}

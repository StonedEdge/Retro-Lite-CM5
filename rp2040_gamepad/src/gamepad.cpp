#include "bsp/board.h"
#include "pico/binary_info.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "usb_descriptors.h"

#define PIN_A       0
#define PIN_B       1
#define PIN_X       2
#define PIN_Y       3
#define PIN_START   4
#define PIN_SELECT  5
#define PIN_D_UP    6
#define PIN_D_DOWN  7
#define PIN_D_LEFT  8
#define PIN_D_RIGHT 9
#define PIN_L2_HALF 10
#define PIN_R2_HALF 11
#define PIN_L2_FULL 12
#define PIN_R2_FULL 13
#define PIN_R1      14
#define PIN_L1      15
#define PIN_L3      16
#define PIN_R3      17
#define PIN_HOTKEY  18

// Define buttons
byte buttonCount = 15; // Number of buttons in use. Change length of buttonPins, buttonState and newButtonState to match.
byte buttonPins[15] = {
  PIN_A,
  PIN_B,
  PIN_X,
  PIN_Y,
  PIN_START,
  PIN_SELECT,
  PIN_L2_HALF,
  PIN_R2_HALF,
  PIN_L2_FULL,
  PIN_R2_FULL,
  PIN_R1,
  PIN_L1,
  PIN_L3,
  PIN_R3,
  PIN_HOTKEY
}; // Array to store digital pins used for buttons. Length must be the same as buttonCount.

#define BTN_A       0
#define BTN_B       1
#define BTN_X       2
#define BTN_Y       3
#define BTN_START   4
#define BTN_SELECT  5
#define BTN_L2_HALF 6
#define BTN_R2_HALF 7
#define BTN_L2_FULL 8
#define BTN_R2_FULL 9
#define BTN_R1      10
#define BTN_L1      11
#define BTN_L3      12
#define BTN_R3      13
#define BTN_HOTKEY  14

byte dpadPins[4] = {
  PIN_D_UP,
  PIN_D_RIGHT,
  PIN_D_DOWN,
  PIN_D_LEFT
}; // Up, Right, Down, Left. Do not change order of directions.

#define DPAD_UP    0
#define DPAD_DOWN  1
#define DPAD_LEFT  2
#define DPAD_RIGHT 3

// Button state arrays
bool dpadPinsState[4];       // Empty State array for dPad
bool buttonState[15];    // Empty State array for buttons last sent state. Must be same length as buttonPins
bool newButtonState[15]; // Empty State array for buttons. Must be same length as buttonPins

// Define Analog Pins for joysticks
const int leftJoyX = A3;
const int leftJoyY = A2;
const int rightJoyX = A1;
const int rightJoyY = A0;

// Joystick Settings
const bool invertLeftY = false;  //------------------------------------------
const bool invertLeftX = false;  // Change these settings for Inverted mounting 
const bool invertRightY = true;  // of joysticks.
const bool invertRightX = true;  //------------------------------------------
const int deadBandLeft = 10;     //
const int deadBandRight = 10;    // Joystick deadband settings. Deadband is the same for both axis on each joystick.
const bool useDeadband = true;   //
const int earlyLeftX = 30;       //--------------------------------------------------
const int earlyLeftY = 30;       // Distance from end of travel to achieve full axis movement. 
const int earlyRightY = 30;      // This helps square out each axis response to allow full movement speed with direction input.
const int earlyRightX = 30;      //--------------------------------------------------
bool scaledJoystickOutput = true; // Enable joystick scaling. Needed for switch joysticks due to uneven axis travels. Disabling will save some compute time if your joystick works well without it.

int minLeftX = 330;
int maxLeftX = 830;
int midLeftX = 512;

int minLeftY = 205;
int maxLeftY = 635;
int midLeftY = 477;

int minRightY = 215;
int maxRightY = 730;
int midRightY = 529;

int minRightX = 300;
int maxRightX = 780;
int midRightX = 525;

int xAccelOffset = 1055;
int yAccelOffset = -3053;
int zAccelOffset = 1149;
int xGyroOffset = 16;
int yGyroOffset = -20;
int zGyroOffset = 42;

// All variables below general use, not used for configuration.
bool calibrationMode = false;
int calibrationStep = 1;
// unsigned long keyboardTimer;
bool L3Pressed = false;
bool menuEnabled = false;
bool povHatMode = true; // Enable to use POV Hat for Dpad instead of analog

// Mouse Variables
bool mouseEnabled = false;
int mouseDivider = 8; // Adjust this value to control mouse sensitivity. Higher number = slower response.
unsigned long mouseTimer;
int mouseInterval = 10; // Interval in MS between mouse updates
unsigned long mouseModeTimer;
bool mouseModeTimerStarted = false;

bool calibrateMPU = false;

hid_gamepad_report_t joystick;
hid_gamepad_report_t lastState;

byte axisMap(int i, int minIn, int midIn, int maxIn, int earlyStop, int deadBand) {
  // Shift all joystick values to a base of zero. All values are halved due to ram limitations on the 32u4.
  int shiftedMin = 0;
  int shiftedMid = (midIn - minIn) / 2;
  int shiftedMax = (maxIn - minIn) / 2;

  if (i < shiftedMid + deadBand && i > shiftedMid - deadBand) {
    return 127;
  } else if (i < shiftedMid) {
    if (i > shiftedMin + earlyStop) {
      return map(i, shiftedMin, shiftedMid - deadBand, 0, 127);
    } else {
      return 0;
    }
  } else {
    if (i < shiftedMax - earlyStop) {
      return map(i, shiftedMid + deadBand, shiftedMax, 127, 254);
    } else {
      return 254;
    }
  }
  return 0;
}

byte leftXaxisMap(int i) {
  return axisMap(i, minLeftX, midLeftX, maxLeftX, earlyLeftX, deadBandLeft);
}

byte leftYaxisMap(int i) {
  return axisMap(i, minLeftY, midLeftY, maxLeftY, earlyLeftY, deadBandLeft);
}

byte rightXaxisMap(int i) {
  return axisMap(i, minRightX, midRightX, maxRightX, earlyRightX, deadBandRight);
}

byte rightYaxisMap(int i) {
  return axisMap(i, minRightY, midRightY, maxRightY, earlyRightY, deadBandRight);
}

int readJoystick(int joystickPin, bool invertOutput) { // Reads raw joystick values and inverts if required
  int var = analogRead(joystickPin);
  if (invertOutput) {
    var = 1023 - var;
    return var;
  } else {
    return var;
  }
}

void serial_write(byte b)
{
    (void)b;

    // TODO
}

void buttonRead() { // Read button inputs and set state arrays.
  for (int i = 0; i < buttonCount; i++)
    buttonState[i] = !gpio_get(buttonPins[i]);

  for (int i = 0; i < 4; i++)
    dpadPinsState[i] = !gpio_get(dpadPins[i]);
}

void joypadButtons() { // Set joystick buttons for USB output
  for (int i = 0; i < buttonCount; i++) {
    if (newButtonState[i] != buttonState[i]) {
        joystick.buttons = (joystick.buttons & ~(1 << i)) | ((uint32_t)buttonState[i] << i);
      newButtonState[i] = buttonState[i];
    }
  }
}

void dPadInput() { // D-Pad as RY and RZ Axis
  if (!povHatMode) {
    if (dpadPinsState[DPAD_UP] && !buttonState[BTN_SELECT]) { // Up
      joystick.ry = 127;
    } else if (dpadPinsState[DPAD_DOWN] && !buttonState[BTN_SELECT]) { // Down
      joystick.ry = -127;
    } else {
      joystick.ry = 0;
    }
    if (dpadPinsState[DPAD_RIGHT]) { // Right
      joystick.rz = 127;
    } else if (dpadPinsState[DPAD_LEFT]) { // Left
      joystick.rz = -127;
    } else {
      joystick.rz = 0;
    }
  } else { // POV Hat Mode
    joystick.hat = GAMEPAD_HAT_CENTERED;
    if (dpadPinsState[DPAD_UP] && !buttonState[BTN_SELECT]) { // Up
      if (dpadPinsState[DPAD_RIGHT]) {
        hostick.hat = GAMEPAD_HAT_UP_RIGHT;
      } else if (dpadPinsState[DPAD_LEFT]) {
        hostick.hat = GAMEPAD_HAT_UP_LEFT;
      } else {
        hostick.hat = GAMEPAD_HAT_UP;
      }
    } else if (dpadPinsState[DPAD_DOWN] && !buttonState[BTN_SELECT]) { // Down
      if (dpadPinsState[DPAD_RIGHT]) {
        hostick.hat = GAMEPAD_HAT_DOWN_RIGHT;
      } else if (dpadPinsState[DPAD_LEFT]) {
        hostick.hat = GAMEPAD_HAT_DOWN_LEFT;
      } else {
        hostick.hat = GAMEPAD_HAT_DOWN;
      }
    } else if (dpadPinsState[DPAD_RIGHT]) { // Right
      hostick.hat = GAMEPAD_HAT_RIGHT;
    } else if (dpadPinsState[DPAD_LEFT]) { // Left
      hostick.hat = GAMEPAD_HAT_LEFT;
    }
  }
}
/*
void writeIntIntoEEPROM(int address, int number) { // Splits Int into BYTES for EEPROM
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address) { // Converts BYTES to INT from EEPROM
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}
*/
void readJoystickConfig() { // Read joystick calibration from EEPROM
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

void writeJoystickConfig() { // Store joystick calibration in EEPROM
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

void eepromLoad() { // Loads stored settings from EEPROM
/*
  if (readIntFromEEPROM(1) != -1) { // Check Joystick Calibration in EEPROM is not Empty
    readJoystickConfig(); // Load joystick calibration from EEPROM
  }
*/
}

void joystickCalibration() { // Very rough at the moment but it works. Read usage instructions at top of page.
  if (calibrationStep == 1) {
    if (buttonState[BTN_A]) {
      //RXLED1;
      delay(100);
      //RXLED0;
      midLeftX = readJoystick(leftJoyX, invertLeftX);
      midLeftY = readJoystick(leftJoyY, invertLeftY);
      midRightX = readJoystick(rightJoyX, invertRightX);
      midRightY = readJoystick(rightJoyY, invertRightY);
      calibrationStep = 2;
      serial_write(calibrationStepTwo);
      delay(50);
    }
  } else if (calibrationStep == 2) {
      //RXLED1;
      delay(100);
      //RXLED0;
      minLeftX = midLeftX;
      minLeftY = midLeftY;
      maxLeftX = 0;
      maxLeftY = 0;
      minRightX = midRightX;
      minRightY = midRightY;
      maxRightX = 0;
      maxRightY = 0;
      calibrationStep = 3;
      delay(500);
  } else if (calibrationStep == 3) {
    int var = readJoystick(leftJoyX, invertLeftX);
    if (var > maxLeftX) maxLeftX = var;
    if (var < minLeftX) minLeftX = var;
    var = readJoystick(leftJoyY, invertLeftY);
    if (var > maxLeftY) maxLeftY = var;
    if (var < minLeftY) minLeftY = var;
    var = readJoystick(rightJoyX, invertRightX);
    if (var > maxRightX) maxRightX = var;
    if (var < minRightX) minRightX = var;
    var = readJoystick(rightJoyY, invertRightY);
    if (var > maxRightY) maxRightY = var;
    if (var < minRightY) minRightY = var;
    if (buttonState[BTN_A]) { // Complete Calibration
      //RXLED1;
      delay(100);
      //RXLED0;
      delay(200);
      writeJoystickConfig(); // Update EEPROM
      serial_write(calibrationComplete);
      delay(50);
      calibrationStep = 1;
      calibrationMode = false;
    }
  }
}

void joystickInput() {
  int var = 0;
  var = readJoystick(rightJoyY, invertRightY);
  var = (var - minRightY) / 2;
  joystick.rx = rightYaxisMap(var);

  var = readJoystick(rightJoyX, invertRightX);
  var = (var - minRightX) / 2;
  joystick.z = rightXaxisMap(var);

  var = readJoystick(leftJoyY, invertLeftY);
  var = (var - minLeftY) / 2;
  joystick.y = leftYaxisMap(var);

  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  joystick.x = leftXaxisMap(var);

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  sensors.setMotion6(ax, ay, az, gx, gy, gz);
}

void pinModeInputPullup(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_IN);
    gpio_pull_up(pin);
}

void gamepad_init() {
  for (int i = 0; i < buttonCount; i++) { // Set all button pins as input pullup. Change to INPUT if using external resistors.
    pinModeInputPullup(buttonPins[i]);
  }
  for (int i = 0; i < 4; i++) { // Set all dpad button pins as input pullup.
    pinModeInputPullup(dpadPins[i]);
  }

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

    if (lastState != joystick && !calibrationMode && !menuMode) {
        tud_hid_report(REPORT_ID_GAMEPAD, &joystick, sizeof(joystick));
        lastState = joystick;
        return true;
    }

    return false;
}

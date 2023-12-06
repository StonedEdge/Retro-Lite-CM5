/*  
 * ATMEGA32u4 USB Joypad for project Retro Lite CM5.
 * Default configuration - 2 Joysticks, 14 Buttons, 1 Dpad.
 * 
 * Joystick calibration:
 * 1: Press the buttons Start and R3 simultaneously to put the controller into calibration mode.
 * 2: Centre both joysticks and press the A button  to record mid position, RX LED will blink once to confirm step complete.
 * 3: Move both joysticks to full extents of travel several times to record maximum axis values.
 * 4: Press button A again. RX LED will blink once more to confirm.
 * 5: Calibration is now complete and stored.
 * 
 * If you're using this controller with Windows, you will also need to run the windows joypad calibration once you've completed the internal calibration.
 * If you change joysticks or your centerpoint drifts (thanks, nintendo) re-running the internal calibration should solve the problem.
 * 
 * Note: Whilst controller is in calibration mode all USB control output will cease until calibration is complete or controller is restarted.
 * 
 * Created by - Daniel McKenzie
 * 03-12-2021 v1.4
 * Updated by - Ivan Podogov
 * 06-11-2023 v0.9
 */

#include <Arduino.h>
#include "wiring_private.h"
#include <EEPROM.h>
#include <Wire.h>
#include "Keyboard.h"
#include "Mouse.h"
#include <Joystick.h> // https://github.com/MHeironimus/ArduinoJoystickLibrary
#include "Multiaxis.h"
#include "I2Cdev.h"
#include "MPU6050.h"
#include <math.h>

// Serial Comms
byte brightnessUp = 0x00;
byte brightnessDn = 0x01;
byte calibrationStepOne = 0x02;
byte calibrationStepTwo = 0x03;
byte calibrationComplete = 0x04;
byte osKeyboardEnabled = 0x05;
byte osKeyboardLeft = 0x06;
byte osKeyboardRight = 0x07;
byte osKeyboardUp = 0x08;
byte osKeyboardDn = 0x09;
byte osKeyboardSelect = 0x10;
byte osKeyboardDisabled = 0x11;
byte menuOpen = 0x12;
byte menuClose = 0x13;
byte povModeEnable = 0x14;
byte povModeDisable = 0x15;
byte mouseEnable = 0x16; // Currently unused. May be used in future to toggle mouse from OSD menu
byte mouseDisable = 0x17; // Currently unused. May be used in future to toggle mouse from OSD menu
int serialButtonDelay = 150;

#define baudrate 19200 // Baudrate for serial.

struct at_pin {
  uint8_t port;
  uint8_t bit;
  uint8_t timer;
};

at_pin pinout[] = {
  { 6, 0, NOT_ON_TIMER },
  { 3, 6, TIMER3A },
  { 4, 2, NOT_ON_TIMER },
  { 4, 4, NOT_ON_TIMER },
  { 4, 3, NOT_ON_TIMER },
  { 4, 6, NOT_ON_TIMER },
  { 2, 4, NOT_ON_TIMER },
  { 2, 5, TIMER1A },
  { 2, 6, TIMER1B },
  { 2, 7, TIMER0A },
  { 3, 7, TIMER4A },
  { 4, 7, TIMER4D },
  { 2, 0, NOT_ON_TIMER },
  { 2, 3, NOT_ON_TIMER },
  { 5, 6, NOT_ON_TIMER },
  { 2, 1, NOT_ON_TIMER },
  { 2, 2, NOT_ON_TIMER },
  { 6, 1, NOT_ON_TIMER },
  { 4, 5, NOT_ON_TIMER },
};

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
byte buttonCount = 15; // Number of buttons in use. Change length of buttonPins, lastButtonState and currentButtonState to match.
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
byte dpadPinsState[4];       // Empty State array for dPad
byte lastButtonState[15];    // Empty State array for buttons last sent state. Must be same length as buttonPins
byte currentButtonState[15]; // Empty State array for buttons. Must be same length as buttonPins

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
boolean scaledJoystickOutput = true; // Enable joystick scaling. Needed for switch joysticks due to uneven axis travels. Disabling will save some compute time if your joystick works well without it.

// Main Joystick setup
Joystick_ joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_GAMEPAD,
  buttonCount, 1,       // Button Count, Hat Switch Count
  true, true, true,     // X Y Z
  true, true, true,     // Rx, Ry, Rz
  false, false,         // Rudder, Throttle
  false, false, false); // Accelerator, Brake, Steering

// Main Joystick setup
Multiaxis_ sensors;

// Default Joystick calibration settings and EEPROM storage Address
int minLeftX = 330; // EEPROM Adr = 1
int maxLeftX = 830; // EEPROM Adr = 3
int midLeftX = 512; // EEPROM Adr = 5

int minLeftY = 205; // EEPROM Adr = 7
int maxLeftY = 635; // EEPROM Adr = 9
int midLeftY = 477; // EEPROM Adr = 11

int minRightY = 215; // EEPROM Adr = 13
int maxRightY = 730; // EEPROM Adr = 15
int midRightY = 529; // EEPROM Adr = 17

int minRightX = 300; // EEPROM Adr = 19
int maxRightX = 780; // EEPROM Adr = 21
int midRightX = 525; // EEPROM Adr = 23

int xAccelOffset = 1055;  // EEPROM Adr = 25
int yAccelOffset = -3053; // EEPROM Adr = 27
int zAccelOffset = 1149;  // EEPROM Adr = 29
int xGyroOffset = 16;     // EEPROM Adr = 31
int yGyroOffset = -20;    // EEPROM Adr = 33
int zGyroOffset = 42;     // EEPROM Adr = 35

// All variables below general use, not used for configuration.
boolean calibrationMode = false;
int calibrationStep = 1;
// unsigned long keyboardTimer;
boolean L3Pressed = false;
boolean menuEnabled = false;
boolean povHatMode = true; // Enable to use POV Hat for Dpad instead of analog

// Mouse Variables
boolean mouseEnabled = false;
int mouseDivider = 8; // Adjust this value to control mouse sensitivity. Higher number = slower response.
unsigned long mouseTimer;
int mouseInterval = 10; // Interval in MS between mouse updates
unsigned long mouseModeTimer;
boolean mouseModeTimerStarted = false;

boolean calibrateMPU = false;

MPU6050 mpu;

void calibrateIMU(MPU6050 *accelgyro, int offsets[6]);

byte LUT(int i, int minIn, int midIn, int maxIn, int earlyStop, int deadBand) { // This function builds a lookup table for the given axis.
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

byte leftXLUT(int i) {
  return LUT(i, minLeftX, midLeftX, maxLeftX, earlyLeftX, deadBandLeft);
}

byte leftYLUT(int i) {
  return LUT(i, minLeftY, midLeftY, maxLeftY, earlyLeftY, deadBandLeft);
}

byte rightXLUT(int i) {
  return LUT(i, minRightX, midRightX, maxRightX, earlyRightX, deadBandRight);
}

byte rightYLUT(int i) {
  return LUT(i, minRightY, midRightY, maxRightY, earlyRightY, deadBandRight);
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar == 27) { // Escape
      Keyboard.press(KEY_ESC);
    } else if (inChar == 8) { // Backspace
      Keyboard.press(KEY_BACKSPACE);
    } else if (inChar == 13) { // Enter
      Keyboard.press(KEY_RETURN);
    } else if (inChar == 14) { // Right
      Keyboard.press(KEY_RIGHT_ARROW);
    } else if (inChar == 15) { // Left
      Keyboard.press(KEY_LEFT_ARROW);
    } else if (inChar == 17) { // Up
      Keyboard.press(KEY_UP_ARROW);
    } else if (inChar == 18) { // Down
      Keyboard.press(KEY_DOWN_ARROW);
    } else if (inChar == calibrationStepOne) {
      calibrationMode = true;
    } else if (inChar == menuClose) {
      menuEnabled = false;
    } else if (inChar == povModeDisable) {
      povHatMode = false;
    } else if (inChar == povModeEnable) {
      povHatMode = true;
    } else {
      Keyboard.press(inChar);
    }
    delay(10);
    Keyboard.releaseAll();
  }
}

int readJoystick(int joystickPin, boolean invertOutput) { // Reads raw joystick values and inverts if required
  int var = analogRead(joystickPin);
  if (invertOutput) {
    var = 1023 - var;
    return var;
  } else {
    return var;
  }
}

void mouseControl() {
  int var;

  // Calculate Y Value
  var = readJoystick(leftJoyY, invertLeftY); // read raw input
  var = (var - minLeftY) / 2; // scale to fit LUT
  var = leftYLUT(var); // Read value from LUT
  var = var - 127; // Shift to 0 centre.
  int yMove = var / mouseDivider; // Divide signal by the mouseDivider for mouse level output
 
  // Calculate X Value
  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  var = leftXLUT(var);
  var = var - 127;
  int xMove = var / mouseDivider;

  // Move Mouse
  Mouse.move(xMove, yMove, 0); // X, Y, Scroll Wheel
  
  // Left Click
  if (lastButtonState[6] == HIGH) { // Right ZR trigger
    if (!Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.press(MOUSE_LEFT);
    }
  } else {
    if (Mouse.isPressed(MOUSE_LEFT)) {
      Mouse.release(MOUSE_LEFT);
    }
  }
  
  // Right Click // Left ZR trigger
  if (lastButtonState[9] == HIGH) {
    if (!Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.press(MOUSE_RIGHT);
    }
  } else {
    if (Mouse.isPressed(MOUSE_RIGHT)) {
      Mouse.release(MOUSE_RIGHT);
    }
  }
}

static void turnOffPWM(uint8_t timer) {
  switch (timer) {
    case TIMER1A: cbi(TCCR1A, COM1A1); break;
    case TIMER1B: cbi(TCCR1A, COM1B1); break;
    case TIMER1C: cbi(TCCR1A, COM1C1); break;
    case TIMER0A: cbi(TCCR0A, COM0A1); break;
    case TIMER0B: cbi(TCCR0A, COM0B1); break;
    case TIMER3A: cbi(TCCR3A, COM3A1); break;
    case TIMER3B: cbi(TCCR3A, COM3B1); break;
    case TIMER3C: cbi(TCCR3A, COM3C1); break;
    case TIMER4A: cbi(TCCR4A, COM4A1); break;
    case TIMER4B: cbi(TCCR4A, COM4B1); break;
    case TIMER4D: cbi(TCCR4C, COM4D1); break;
  }
}

int atDigitalRead(uint8_t pin) {
  uint8_t port = pinout[pin].port;
  uint8_t bit = (1 << pinout[pin].bit);
  uint8_t timer = pinout[pin].timer;

  // If the pin that support PWM output, we need to turn it off
  // before getting a digital reading.
  if (timer != NOT_ON_TIMER) turnOffPWM(timer);

  if (*portInputRegister(port) & bit) return HIGH;
  return LOW;
}

void menuMode() {
  if (dpadPinsState[0] == 1) {
    Serial.write(osKeyboardUp);
    delay(serialButtonDelay);
  } else if (dpadPinsState[2] == 1) {
    Serial.write(osKeyboardDn);
    delay(serialButtonDelay);
  } else if (dpadPinsState[1] == 1) {
    Serial.write(osKeyboardRight);
    delay(serialButtonDelay);
  } else if (dpadPinsState[3] == 1) {
    Serial.write(osKeyboardLeft);
    delay(serialButtonDelay);
  } else if (lastButtonState[1] == 1) {
    Serial.write(osKeyboardSelect);
    delay(serialButtonDelay);
  }
}

void buttonRead() { // Read button inputs and set state arrays.
  for (int i = 0; i < buttonCount; i++) {
    int input = !atDigitalRead(buttonPins[i]);
    if (input != lastButtonState[i]) {
      lastButtonState[i] = input;
    }
  }
  for (int i = 0; i < 4; i++) {
    int input = !atDigitalRead(dpadPins[i]);
    if (input != dpadPinsState[i]) {
      dpadPinsState[i] = input;
    }
  }
}

void joypadButtons() { // Set joystick buttons for USB output
  for (int i = 0; i < buttonCount; i++) {
    if (lastButtonState[i] != currentButtonState[i]) {
      joystick.setButton(i, lastButtonState[i]);
      currentButtonState[i] = lastButtonState[i];
    }
  }
}

void dPadInput() { // D-Pad as RY and RZ Axis
  if (!povHatMode) {
    if (dpadPinsState[DPAD_UP] == HIGH && lastButtonState[BTN_SELECT] != HIGH) { // Up
      joystick.setRyAxis(2);
    } else if (dpadPinsState[DPAD_DOWN] == HIGH && lastButtonState[BTN_SELECT] != HIGH) { // Down
      joystick.setRyAxis(0);
    } else {
      joystick.setRyAxis(1);
    }
    if (dpadPinsState[DPAD_RIGHT] == HIGH) { // Right
      joystick.setRzAxis(2);
    } else if (dpadPinsState[DPAD_LEFT] == HIGH) { // Left
      joystick.setRzAxis(0);
    } else {
      joystick.setRzAxis(1);
    }
  } else { // POV Hat Mode
    int angle = -1;
    if (dpadPinsState[DPAD_UP] == HIGH && lastButtonState[BTN_SELECT] != HIGH) { // Up
      if (dpadPinsState[DPAD_RIGHT] == HIGH) {
        angle = 45;
      } else if (dpadPinsState[DPAD_LEFT] == HIGH) {
        angle = 315;
      } else {
        angle = 0;
      }
    } else if (dpadPinsState[DPAD_DOWN] == HIGH && lastButtonState[BTN_SELECT] != HIGH) { // Down
      if (dpadPinsState[DPAD_RIGHT] == HIGH) {
        angle = 135;
      } else if (dpadPinsState[DPAD_LEFT] == HIGH) {
        angle = 225;
      } else {
        angle = 180;
      }
    } else if (dpadPinsState[DPAD_RIGHT] == HIGH) { // Right
      angle = 90;
    } else if (dpadPinsState[DPAD_LEFT] == HIGH) { // Left
      angle = 270;
    }
    joystick.setHatSwitch(0, angle);
  }
}

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

void readJoystickConfig() { // Read joystick calibration from EEPROM
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
}

void writeJoystickConfig() { // Store joystick calibration in EEPROM
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
}

void eepromLoad() { // Loads stored settings from EEPROM
  if (readIntFromEEPROM(1) != -1) { // Check Joystick Calibration in EEPROM is not Empty
    readJoystickConfig(); // Load joystick calibration from EEPROM
  }
}

void joystickCalibration() { // Very rough at the moment but it works. Read usage instructions at top of page.
  if (calibrationStep == 1) {
    if (lastButtonState[BTN_A] == HIGH) {
      //RXLED1;
      delay(100);
      //RXLED0;
      midLeftX = readJoystick(leftJoyX, invertLeftX);
      midLeftY = readJoystick(leftJoyY, invertLeftY);
      midRightX = readJoystick(rightJoyX, invertRightX);
      midRightY = readJoystick(rightJoyY, invertRightY);
      calibrationStep = 2;
      Serial.write(calibrationStepTwo);
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
    if (lastButtonState[BTN_A] == HIGH) { // Complete Calibration
      //RXLED1;
      delay(100);
      //RXLED0;
      delay(200);
      writeJoystickConfig(); // Update EEPROM
      Serial.write(calibrationComplete);
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
  joystick.setRxAxis(rightYLUT(var));

  var = readJoystick(rightJoyX, invertRightX);
  var = (var - minRightX) / 2;
  joystick.setZAxis(rightXLUT(var));

  var = readJoystick(leftJoyY, invertLeftY);
  var = (var - minLeftY) / 2;
  joystick.setYAxis(leftYLUT(var));

  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  joystick.setXAxis(leftXLUT(var));

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  sensors.setMotion6(ax, ay, az, gx, gy, gz);
}

void atPinModeInputPullup(uint8_t pin) {
  uint8_t port = pinout[pin].port;
  uint8_t bit = (1 << pinout[pin].bit);

  volatile uint8_t *reg, *out;
  reg = portModeRegister(port);
  out = portOutputRegister(port);

  uint8_t oldSREG = SREG;
  cli();
  *reg &= ~bit;
  *out |= bit;
  SREG = oldSREG;
}

void setup() {
  // Needed for Retropie to detect axis.
  joystick.setXAxisRange(0, 254);
  joystick.setYAxisRange(0, 254);
  joystick.setZAxisRange(0, 254);
  joystick.setRxAxisRange(0, 254);
  joystick.setRyAxisRange(0, 2);
  joystick.setRzAxisRange(0, 2);

  Serial.begin(baudrate);
  //while (!Serial);
  joystick.begin(false); // Initialise joystick mode with auto sendState disabled as it has a huge processor time penalty for seemingly no benefit.
  sensors.begin();
  Keyboard.begin(); //Initialise keyboard for on screen keyboard input
  Mouse.begin(); // Initialise mouse control

  for (int i = 0; i < buttonCount; i++) { // Set all button pins as input pullup. Change to INPUT if using external resistors.
    atPinModeInputPullup(buttonPins[i]);
  }
  for (int i = 0; i < 4; i++) { // Set all dpad button pins as input pullup.
    atPinModeInputPullup(dpadPins[i]);
  }
  
  eepromLoad(); // Check for stored joystick settings and load if applicable.

  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  // initialize device
  mpu.initialize();

  // verify connection
  if (mpu.testConnection()) {
    // supply your own gyro offsets here, scaled for min sensitivity
    mpu.setXAccelOffset(xAccelOffset);
    mpu.setYAccelOffset(yAccelOffset);
    mpu.setZAccelOffset(zAccelOffset);
    mpu.setXGyroOffset(xGyroOffset);
    mpu.setYGyroOffset(yGyroOffset);
    mpu.setZGyroOffset(zGyroOffset);

    if (calibrateMPU) {
      // Calibration Time: generate offsets and calibrate our MPU6050
      mpu.CalibrateAccel(6);
      mpu.CalibrateGyro(6);
      mpu.PrintActiveOffsets();

      int offsets[6];
      calibrateIMU(&mpu, offsets); // takes a couple minutes to complete
      xAccelOffset = offsets[0];
      yAccelOffset = offsets[1];
      zAccelOffset = offsets[2];
      xGyroOffset = offsets[3];
      yGyroOffset = offsets[4];
      zGyroOffset = offsets[5];
      writeJoystickConfig(); // Update EEPROM
    }

    mpu.setFullScaleAccelRange(MPU6050_ACCEL_FS_4);
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_1000);
  }
}

void loop() {
  buttonRead();

  if (calibrationMode) {
    joystickCalibration();
  } else if (menuEnabled) {
    serialEvent();
    menuMode();
  } else {
    joypadButtons();
    joystickInput();
    dPadInput();
    joystick.sendState(); // Update input changes
    sensors.sendState();
  }

  // Mouse Toggle
  if (lastButtonState[BTN_SELECT] == HIGH && lastButtonState[BTN_R3] == HIGH) { // Left joystick click toggles the mouse cursor to an on/off state
    if (mouseModeTimerStarted) {
      if (millis() > mouseModeTimer + 2000) {
        if (mouseEnabled) {
          mouseEnabled = false;
        } else {
          mouseEnabled = true;
        }
        mouseModeTimerStarted = false;
      }
    } else {
      mouseModeTimerStarted = true;
      mouseModeTimer = millis();
    }
  } else {
    mouseModeTimerStarted = false;
  }

  // Mouse Mode
  if (mouseEnabled) {
    if (mouseTimer + mouseInterval < millis()) {
      mouseControl();
      mouseTimer = millis();
    }
  }
  if (lastButtonState[BTN_SELECT] == HIGH && dpadPinsState[DPAD_UP] == HIGH) { 
    Serial.write(brightnessUp);
    delay(serialButtonDelay);
  }
  if (lastButtonState[BTN_SELECT] == HIGH && dpadPinsState[DPAD_DOWN] == HIGH) {
    Serial.write(brightnessDn);
    delay(serialButtonDelay);
  }

  if (lastButtonState[BTN_SELECT] == HIGH && lastButtonState[BTN_R3] == HIGH) { //If this combination of buttons is pressed, Open Menu. (Select and right joystick button)
    if (menuEnabled) {
      Serial.write(menuClose);
      delay(200);
      menuEnabled = false;
    } else {
      Serial.write(menuOpen);
      delay(200);
      menuEnabled = true;
    }
  }

  if (lastButtonState[BTN_START] == HIGH && lastButtonState[BTN_R3] == HIGH) { // Start+R3 for jostick calibration
    calibrationStep = 1;
    calibrationMode = true;
  }
}

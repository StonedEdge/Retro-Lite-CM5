/*  
 * ATMEGA32u4 USB Joypad for project Retro Lite CM5.
 * Default configuration - 2 Joysticks, 13 Buttons, 1 Dpad.
 * 
 * Joystick calibration:
 * 1: Press the buttons on pins D0 and D3 simultaneously to put the controller into calibration mode.
 * 2: Centre both joysticks and press the button on pin D1 to record mid position, RX LED will blink once to confirm step complete.
 * 3: Move both joysticks to full extents of travel several times to record maximum axis values.
 * 4: Press button on D1 again. RX LED will blink once more to confirm.
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
#include <EEPROM.h>
#include <Wire.h>
#include <Joystick.h> //https://github.com/MHeironimus/ArduinoJoystickLibrary
//#include "Keyboard.h"
#include "Mouse.h"
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include <math.h>

//Serial Comms
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
byte mouseEnable = 0x16; //Currently unused. May be used in future to toggle mouse from OSD menu
byte mouseDisable = 0x17; //Currently unused. May be used in future to toggle mouse from OSD menu
int serialButtonDelay = 150;

#define baudrate 19200 //Baudrate for serial.

//Define button pins
byte buttonCount = 13; //Number of buttons in use. Change length of buttonPins, lastButtonState and currentButtonState to match.
byte buttonPins[13] = {0,1,2,3,4,5,6,7,12,13,14,15,16}; //Array to store digital pins used for buttons. Length must be the same as buttonCount.
byte dpadPins[4] = {8,11,9,10}; //Up, Right, Down, Left. Do not change order of directions.

//Button state arrays
byte dpadPinsState[4];  //Empty State array for dPad
byte lastButtonState[13]; //Empty State array for buttons last sent state. Must be same length as buttonPins
byte currentButtonState[13]; //Empty State array for buttons. Must be same length as buttonPins

//Define Analog Pins for joysticks
const int leftJoyX = A3;
const int leftJoyY = A2;
const int rightJoyX = A1;
const int rightJoyY = A0;

//Joystick Settings
const bool invertLeftY = false;  //------------------------------------------
const bool invertLeftX = false;  //Change these settings for Inverted mounting 
const bool invertRightY = true;  //of joysticks.
const bool invertRightX = true;  //------------------------------------------
const int deadBandLeft = 10;     //
const int deadBandRight = 10;    //Joystick deadband settings. Deadband is the same for both axis on each joystick.
const bool useDeadband = true;   //
const int earlyLeftX = 30;       //--------------------------------------------------
const int earlyLeftY = 30;       //Distance from end of travel to achieve full axis movement. 
const int earlyRightY = 30;      //This helps square out each axis response to allow full movement speed with direction input.
const int earlyRightX = 30;      //--------------------------------------------------
boolean scaledJoystickOutput = true; //Enable joystick scaling. Needed for switch joysticks due to uneven axis travels. Disabling will save some compute time if your joystick works well without it.

//Main Joystick setup
Joystick_ Joystick(
  JOYSTICK_DEFAULT_REPORT_ID,
  JOYSTICK_TYPE_GAMEPAD,
  buttonCount, 1,       // Button Count, Hat Switch Count
  true, true, true,     // X Y Z
  true, true, true,     // Rx, Ry, Rz
  false, false,         // Rudder, Throttle
  false, false, false); // Accelerator, Brake, Steering

//Default Joystick calibration settings and EEPROM storage Address
int minLeftX = 330; //EEPROM Adr = 1
int maxLeftX = 830; //EEPROM Adr = 3
int midLeftX = 512; //EEPROM Adr = 5

int minLeftY = 205; //EEPROM Adr = 7
int maxLeftY = 635; //EEPROM Adr = 9
int midLeftY = 477; //EEPROM Adr = 11

int minRightY = 215; //EEPROM Adr = 13
int maxRightY = 730; //EEPROM Adr = 15
int midRightY = 529; //EEPROM Adr = 17

int minRightX = 300; //EEPROM Adr = 19
int maxRightX = 780; //EEPROM Adr = 21
int midRightX = 525; //EEPROM Adr = 23
/*
//Joystick LookUp Tables, compiled at startup or after joystick calibration.
byte leftXLUT[350];
byte leftYLUT[350];
byte rightXLUT[350];
byte rightYLUT[350];
*/
//All variables below general use, not used for configuration.
boolean calibrationMode = false;
int calibrationStep = 1;
//unsigned long keyboardTimer;
boolean L3Pressed = false;
//boolean menuEnabled = false;
boolean povHatMode = true; //Enable to use POV Hat for Dpad instead of analog

//Mouse Variables
boolean mouseEnabled = false;
boolean airMouse = true;
int mouseDivider = 8; //Adjust this value to control mouse sensitivity. Higher number = slower response.
unsigned long mouseTimer;
int mouseInterval = 10; //Interval in MS between mouse updates
unsigned long mouseModeTimer;
boolean mouseModeTimerStarted = false;

static const float CURSOR_SPEED = 1024.0 / (M_PI / 4);
static const float STABILIZE_BIAS = 16.0;

float gYaw = 0;
float gPitch = 0;
float gdYaw = 0;
float gdPitch = 0;
bool firstRead = true;
bool stabilize = true;

MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t devStatus;      // return status after each device operation (0 = success, !0 = error)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

float clamp(float val) {
  while (val <= -M_PI) {
    val += 2 * M_PI;
  }
  while (val >= M_PI) {
    val -= 2 * M_PI;
  }
  return val;
}

float highpass(float oldVal, float newVal) {
  if (!stabilize) {
    return newVal;
  }
  float delta = clamp(oldVal - newVal);
  float alpha = max(0.0f, 1 - pow(fabsf(delta) * CURSOR_SPEED / STABILIZE_BIAS, 3.0f));
  return newVal + alpha * delta;
}

void sendCurrentState() {
  float dX = gdYaw * CURSOR_SPEED;
  float dY = gdPitch * CURSOR_SPEED;

  // Scale the shift down to fit the protocol.
  if (dX > 127) {
    dY *= 127.0 / dX;
    dX = 127;
  }
  if (dX < -127) {
    dY *= -127.0 / dX;
    dX = -127;
  }
  if (dY > 127) {
    dX *= 127.0 / dY;
    dY = 127;
  }
  if (dY < -127) {
    dX *= -127.0 / dY;
    dY = -127;
  }

  const int8_t x = (int8_t)floor(dX + 0.5);
  const int8_t y = (int8_t)floor(dY + 0.5);

  Mouse.move(x, y, 0);

  // Only subtract the part of the error that was already sent.
  if (x != 0) {
    gdYaw -= x / CURSOR_SPEED;
  }
  if (y != 0) {
    gdPitch -= y / CURSOR_SPEED;
  }
}

void onOrientation(const Quaternion &q) {
  float q1 = -q.x; // X * sin(T/2)
  float q2 = -q.y; // Y * sin(T/2)
  float q3 = -q.z; // Z * sin(T/2)
  float q0 = q.w; // cos(T/2)

  float yaw = atan2(2 * (q0 * q3 - q1 * q2), (1 - 2 * (q1 * q1 + q3 * q3)));
  float pitch = asin(2 * (q0 * q1 + q2 * q3));
  // float roll = atan2(2 * (q0 * q2 - q1 * q3), (1 - 2 * (q1 * q1 + q2 * q2)));

  if (yaw == NAN || pitch == NAN) {
    // NaN case, skip it
    return;
  }

  if (firstRead) {
    gYaw = yaw;
    gPitch = pitch;
    firstRead = false;
  } else {
    const float newYaw = highpass(gYaw, yaw);
    const float newPitch = highpass(gPitch, pitch);

    float dYaw = clamp(gYaw - newYaw);
    float dPitch = gPitch - newPitch;
    gYaw = newYaw;
    gPitch = newPitch;

    // Accumulate the error locally.
    gdYaw += dYaw;
    gdPitch += dPitch;
  }
}

void beginTracking() {
  if (dmpReady) {
    mpu.setDMPEnabled(true);
  }
}

void stepTracking() {
  // if programming failed, don't try to do anything
  if (!dmpReady) return;
  // read a packet from FIFO
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) { // Get the Latest packet 
    Quaternion q;
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    onOrientation(q);
    sendCurrentState();
  }
}

void stopTracking() {
  if (dmpReady) {
    mpu.setDMPEnabled(false);
  }
}

void initDmp() {
  Wire.begin();
  Wire.setClock(400000); // 400kHz I2C clock. Comment this line if having compilation difficulties

  // initialize device
  mpu.initialize();

  // verify connection
  mpu.testConnection(); // No diagnostics here

  // load and configure the DMP
  devStatus = mpu.dmpInitialize();

  // supply your own gyro offsets here, scaled for min sensitivity
  //mpu.setXGyroOffset(51);
  //mpu.setYGyroOffset(8);
  //mpu.setZGyroOffset(21);
  //mpu.setXAccelOffset(1150);
  //mpu.setYAccelOffset(-50);
  //mpu.setZAccelOffset(1060);
  // make sure it worked (returns 0 if so)
  if (devStatus == 0) {
    // Calibration Time: generate offsets and calibrate our MPU6050
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    mpu.PrintActiveOffsets();
    // set our DMP Ready flag so the main loop() function knows it's okay to use it
    dmpReady = true;
  } else {
    // ERROR!
    // 1 = initial memory load failed
    // 2 = DMP configuration updates failed
    // (if it's going to break, usually the code will be 1)
  }
}

byte LUT(int i, int minIn, int midIn, int maxIn, int earlyStop, int deadBand){ //This function builds a lookup table for the given axis.
  //Shift all joystick values to a base of zero. All values are halved due to ram limitations on the 32u4.
  int shiftedMin = 0;
  int shiftedMid = (midIn - minIn) /2;
  int shiftedMax = (maxIn - minIn) /2;
  int temp;
  
  if(i < shiftedMid){
    if(i > shiftedMin + earlyStop){
      temp = map(i, shiftedMin, shiftedMid - deadBand, 0, 127);
    } else {
      temp = 0;
    }
  } else {
    if(i < shiftedMax - earlyStop){
      temp = map(i, shiftedMid + deadBand, shiftedMax, 127, 254);
    } else {
      temp = 254;
    }
  }
  if(i < shiftedMid + deadBand && i > shiftedMid - deadBand){
    temp = 127;
  }

  return temp;
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
/*
void serialEvent(){
  while(Serial.available()){
    char inChar = (char)Serial.read();
    if(inChar == 27){ //Escape
      Keyboard.press(KEY_ESC);
    } else if(inChar == 8){ //Backspace
      Keyboard.press(KEY_BACKSPACE);
    } else if(inChar == 13){ //Enter
      Keyboard.press(KEY_RETURN);
    } else if(inChar == 14){ //Right
      Keyboard.press(KEY_RIGHT_ARROW);
    } else if(inChar == 15){ //Left
      Keyboard.press(KEY_LEFT_ARROW);
    } else if(inChar == 17){ //Up
      Keyboard.press(KEY_UP_ARROW);
    } else if(inChar == 18){ //Down
      Keyboard.press(KEY_DOWN_ARROW);
    } else if(inChar == calibrationStepOne){
      calibrationMode = true;
    } else if(inChar == menuClose){
      menuEnabled = false;
    } else if(inChar == povModeDisable){
      povHatMode = false;
    } else if(inChar == povModeEnable){
      povHatMode = true;
    } else {
      Keyboard.press(inChar);
    }
    delay(10);
    Keyboard.releaseAll();
  }
}
*/
int readJoystick(int joystickPin, boolean invertOutput){ //Reads raw joystick values and inverts if required
  int var = analogRead(joystickPin);
  if(invertOutput){
    var = 1023 - var;
    return var;
  } else {
    return var;
  }
}

void mouseControl() {
  if (airMouse) {
    stepTracking();
    return;
  }

  int var;

  //Calculate Y Value
  var = readJoystick(leftJoyY, invertLeftY); //read raw input
  var = (var - minLeftY) / 2; //scale to fit LUT
  var = leftYLUT(var); //Read value from LUT
  var = var - 127; //Shift to 0 centre.
  int yMove = var / mouseDivider; //Divide signal by the mouseDivider for mouse level output
 
  //Calculate X Value
  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  var = leftXLUT(var);
  var = var - 127;
  int xMove = var / mouseDivider;

  //Move Mouse
  Mouse.move(xMove, yMove, 0); //X, Y, Scroll Wheel
  
  //Left Click
  if(lastButtonState[6] == 1){ //Right ZR trigger
    if(!Mouse.isPressed(MOUSE_LEFT)){
      Mouse.press(MOUSE_LEFT);
    }
  } else {
    if(Mouse.isPressed(MOUSE_LEFT)){
      Mouse.release(MOUSE_LEFT);
    }
  }
  
  //Right Click //Left ZR trigger
  if(lastButtonState[9] == 1){
    if(!Mouse.isPressed(MOUSE_RIGHT)){
      Mouse.press(MOUSE_RIGHT);
    }
  } else {
    if(Mouse.isPressed(MOUSE_RIGHT)){
      Mouse.release(MOUSE_RIGHT);
    }
  }
}
/*
void menuMode(){
  if(dpadPinsState[0] == 1){
    Serial.write(osKeyboardUp);
    delay(serialButtonDelay);
  } else if(dpadPinsState[2] == 1){
    Serial.write(osKeyboardDn);
    delay(serialButtonDelay);
  } else if(dpadPinsState[1] == 1){
    Serial.write(osKeyboardRight);
    delay(serialButtonDelay);
  } else if(dpadPinsState[3] == 1){
    Serial.write(osKeyboardLeft);
    delay(serialButtonDelay);
  } else if(lastButtonState[1] == 1){
    Serial.write(osKeyboardSelect);
    delay(serialButtonDelay);
  }
}

void joystickBuildLUT(byte output[350], int minIn, int midIn, int maxIn, int earlyStop, int deadBand){ //This function builds a lookup table for the given axis.
  //Shift all joystick values to a base of zero. All values are halved due to ram limitations on the 32u4.
  int shiftedMin = 0;
  int shiftedMid = (midIn - minIn) /2;
  int shiftedMax = (maxIn - minIn) /2;
  int temp;
  
  for(int i = 0; i < 350; i++){
    if(i < shiftedMid){
      if(i > shiftedMin + earlyStop){
        temp = map(i, shiftedMin, shiftedMid - deadBand, 0, 127);
      } else {
        temp = 0;
      }
    } else {
      if(i < shiftedMax - earlyStop){
        temp = map(i, shiftedMid + deadBand, shiftedMax, 127, 254);
      } else {
        temp = 254;
      }
    }
    if(i < shiftedMid + deadBand && i > shiftedMid - deadBand){
      temp = 127;
    }
    output[i] = temp;
  }
}

void rebuildLUTs(){
  joystickBuildLUT(leftXLUT, minLeftX, midLeftX, maxLeftX, earlyLeftX, deadBandLeft);
  joystickBuildLUT(leftYLUT, minLeftY, midLeftY, maxLeftY, earlyLeftY, deadBandLeft);
  joystickBuildLUT(rightXLUT, minRightX, midRightX, maxRightX, earlyRightX, deadBandRight);
  joystickBuildLUT(rightYLUT, minRightY, midRightY, maxRightY, earlyRightY, deadBandRight);
}
*/
void buttonRead(){ //Read button inputs and set state arrays.
  for (int i = 0; i < buttonCount; i++){
    int input = !digitalRead(buttonPins[i]);
    if (input != lastButtonState[i]){
      lastButtonState[i] = input;
    }
  }
  for(int i = 0; i < 4; i++){
    int input = !digitalRead(dpadPins[i]);
    if (input != dpadPinsState[i]){
      dpadPinsState[i] = input;
    }
  }
  /*
  for (int i = 0; i < 32; i++) {
    char buf[16];
    int input = !digitalRead(i);
    sprintf(buf, "  %d-%d", i, input);
    Serial.print(buf);
  }
  char buf[16];
  int var = readJoystick(rightJoyY, invertRightY);
  sprintf(buf, "  A0-%d", var);
  Serial.print(buf);
  var = readJoystick(rightJoyX, invertRightX);
  sprintf(buf, "  A1-%d", var);
  Serial.print(buf);
  var = readJoystick(leftJoyY, invertLeftY);
  sprintf(buf, "  A2-%d", var);
  Serial.print(buf);
  var = readJoystick(leftJoyX, invertLeftX);
  sprintf(buf, "  A3-%d", var);
  Serial.print(buf);
  Serial.println();
  */
}

void joypadButtons(){ //Set joystick buttons for USB output
  for(int i = 0; i < buttonCount; i++){
    if(lastButtonState[i] != currentButtonState[i]){
      Joystick.setButton(i, lastButtonState[i]);
      currentButtonState[i] = lastButtonState[i];
    }
  }
}

void dPadInput(){ //D-Pad as RY and RZ Axis
  if(!povHatMode){
    if(dpadPinsState[0] == 1 && lastButtonState[8] != HIGH){ //Up
      Joystick.setRyAxis(2);
    } else if(dpadPinsState[2] == 1 && lastButtonState[8] != HIGH){ //Down
      Joystick.setRyAxis(0);
    } else {
      Joystick.setRyAxis(1);
    }
    if(dpadPinsState[1] == 1){ //Right
      Joystick.setRzAxis(2);
    } else if(dpadPinsState[3] == 1){ //Left
      Joystick.setRzAxis(0);
    } else {
      Joystick.setRzAxis(1);
    }
  } else { //POV Hat Mode
    int angle = -1;
    if(dpadPinsState[0] == 1 && lastButtonState[8] != HIGH){ //Up
      if(dpadPinsState[1] == 1) {
        angle = 45;
      } else if(dpadPinsState[3] == 1) {
        angle = 315;
      } else {
        angle = 0;
      }
    } else if(dpadPinsState[2] == 1 && lastButtonState[8] != HIGH){ //Down
      if(dpadPinsState[1] == 1) {
        angle = 135;
      } else if(dpadPinsState[3] == 1) {
        angle = 225;
      } else {
        angle = 180;
      }
    } else if(dpadPinsState[1] == 1){ //Right
      angle = 90;
    } else if(dpadPinsState[3] == 1){ //Left
      angle = 270;
    }
    Joystick.setHatSwitch(0, angle);
  }
}

void writeIntIntoEEPROM(int address, int number){ //Splits Int into BYTES for EEPROM
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address){ //Converts BYTES to INT from EEPROM
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

void readJoystickConfig(){ //Read joystick calibration from EEPROM
//Left X
  minLeftX = readIntFromEEPROM(1);
  maxLeftX = readIntFromEEPROM(3);
  midLeftX = readIntFromEEPROM(5);
//Left Y
  minLeftY = readIntFromEEPROM(7);
  maxLeftY = readIntFromEEPROM(9);
  midLeftY = readIntFromEEPROM(11);
//Right Y
  minRightY = readIntFromEEPROM(13);
  maxRightY = readIntFromEEPROM(15);
  midRightY = readIntFromEEPROM(17);
// Right X
  minRightX = readIntFromEEPROM(19);
  maxRightX = readIntFromEEPROM(21);
  midRightX = readIntFromEEPROM(23);
}

void writeJoystickConfig(){ //Store joystick calibration in EEPROM
//Left X
  writeIntIntoEEPROM(1, minLeftX);
  writeIntIntoEEPROM(3, maxLeftX);
  writeIntIntoEEPROM(5, midLeftX);
//Left Y
  writeIntIntoEEPROM(7, minLeftY);
  writeIntIntoEEPROM(9, maxLeftY);
  writeIntIntoEEPROM(11, midLeftY);
//Right Y
  writeIntIntoEEPROM(13, minRightY);
  writeIntIntoEEPROM(15, maxRightY);
  writeIntIntoEEPROM(17, midRightY);
// Right X
  writeIntIntoEEPROM(19, minRightX);
  writeIntIntoEEPROM(21, maxRightX);
  writeIntIntoEEPROM(23, midRightX);
}

void eepromLoad(){ //Loads stored settings from EEPROM
  if(readIntFromEEPROM(1) != -1){ //Check Joystick Calibration in EEPROM is not Empty
    readJoystickConfig(); //Load joystick calibration from EEPROM
  }
}

void joystickCalibration(){ //Very rough at the moment but it works. Read usage instructions at top of page.
  if(calibrationStep == 1){
    if(lastButtonState[1] == 1){
      RXLED1;
      delay(100);
      RXLED0;
      midLeftX = readJoystick(leftJoyX, invertLeftX);
      midLeftY = readJoystick(leftJoyY, invertLeftY);
      midRightX = readJoystick(rightJoyX, invertRightX);
      midRightY = readJoystick(rightJoyY, invertRightY);
      calibrationStep = 2;
      Serial.write(calibrationStepTwo);
      delay(50);
    }
  } else if(calibrationStep == 2){
      RXLED1;
      delay(100);
      RXLED0;
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
  } else if(calibrationStep == 3){
    int var = readJoystick(leftJoyX, invertLeftX);
    if(var > maxLeftX) maxLeftX = var;
    if(var < minLeftX) minLeftX = var;
    var = readJoystick(leftJoyY, invertLeftY);
    if(var > maxLeftY) maxLeftY = var;
    if(var < minLeftY) minLeftY = var;
    var = readJoystick(rightJoyX, invertRightX);
    if(var > maxRightX) maxRightX = var;
    if(var < minRightX) minRightX = var;
    var = readJoystick(rightJoyY, invertRightY);
    if(var > maxRightY) maxRightY = var;
    if(var < minRightY) minRightY = var;
    buttonRead();
    if(lastButtonState[1] == 1){ //Complete Calibration
      RXLED1;
      delay(100);
      RXLED0;
      delay(200);
      writeJoystickConfig(); //Update EEPROM
      //rebuildLUTs();
      Serial.write(calibrationComplete);
      delay(50);
      calibrationStep = 1;
      calibrationMode = false;
    }
  }
}

void joystickInput(){
  int var = 0;
  var = readJoystick(rightJoyY, invertRightY);
  var = (var - minRightY) / 2;
  Joystick.setRxAxis(rightYLUT(var));

  var = readJoystick(rightJoyX, invertRightX);
  var = (var - minRightX) / 2;
  Joystick.setZAxis(rightXLUT(var));

  var = readJoystick(leftJoyY, invertLeftY);
  var = (var - minLeftY) / 2;
  Joystick.setYAxis(leftYLUT(var));

  var = readJoystick(leftJoyX, invertLeftX);
  var = (var - minLeftX) / 2;
  Joystick.setXAxis(leftXLUT(var));
}

void setup() {
  //Needed for Retropie to detect axis.
  Joystick.setXAxisRange(0, 254);
  Joystick.setYAxisRange(0, 254);
  Joystick.setZAxisRange(0, 254);
  Joystick.setRxAxisRange(0, 254);
  Joystick.setRyAxisRange(0, 2);
  Joystick.setRzAxisRange(0, 2);

  Serial.begin(baudrate);
  //while (!Serial); //Needed on 32u4 based arduinos for predictable serial output.
  Joystick.begin(false); //Initialise joystick mode with auto sendState disabled as it has a huge processor time penalty for seemingly no benefit.
  //Keyboard.begin(); //Initialise keyboard for on screen keyboard input
  Mouse.begin(); //Initialise mouse control
  
  for(int i = 0; i < buttonCount; i++){ //Set all button pins as input pullup. Change to INPUT if using external resistors.
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  for(int i = 0; i < 4; i++){ //Set all dpad button pins as input pullup.
    pinMode(dpadPins[i], INPUT_PULLUP);
  }
  
  eepromLoad(); //Check for stored joystick settings and load if applicable.
  //rebuildLUTs(); //Build joystick LUT's from stored calibration values.

  initDmp();
}

void loop() {
  buttonRead();

  if(!calibrationMode /*&& !menuEnabled*/){
    joypadButtons();
    joystickInput();
    dPadInput();
    Joystick.sendState(); //Update input changes
  } else if(calibrationMode){
    joystickCalibration();
  }/* else if(menuEnabled){
    serialEvent();
    menuMode();
  }*/
  
  //Mouse Toggle
  if(lastButtonState[12] == HIGH){ //Left joystick click toggles the mouse cursor to an on/off state
    if(!mouseModeTimerStarted){
      mouseModeTimerStarted = true;
      mouseModeTimer = millis();
    } else {
      if(mouseModeTimer + 2000 < millis()){
        if(mouseEnabled){
          mouseEnabled = false;
          stopTracking();
        } else {
          mouseEnabled = true;
          beginTracking();
        }
        mouseModeTimerStarted = false;
      }
    }
  } else {
    mouseModeTimerStarted = false;
  }

  //Mouse Mode
  if(mouseEnabled){
    if(mouseTimer + mouseInterval < millis()){
      mouseControl();
      mouseTimer = millis();
    }
  }
  if(lastButtonState[8] == HIGH && dpadPinsState[0] == 1){ 
    Serial.write(brightnessUp);
    delay(serialButtonDelay);
  }
  if(lastButtonState[8] == HIGH && dpadPinsState[2] == 1){
    Serial.write(brightnessDn);
    delay(serialButtonDelay);
  }
  /*
  if(lastButtonState[8] == HIGH && lastButtonState[0] == HIGH){ //If this combination of buttons is pressed, Open Menu. (Select and right joystick button)
    if(menuEnabled){
      Serial.write(menuClose);
      delay(200);
      menuEnabled = false;
    } else {
      Serial.write(menuOpen);
      delay(200);
      menuEnabled = true;
    }
  }
  */
}

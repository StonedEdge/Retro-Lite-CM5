#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"
#include "toggle.h"

// Serial Comms
const uint8_t brightnessUp = 0x00;
const uint8_t brightnessDn = 0x01;
const uint8_t calibrationStepOne = 0x02;
const uint8_t calibrationStepTwo = 0x03;
const uint8_t calibrationComplete = 0x04;
const uint8_t osKeyboardEnabled = 0x05;
const uint8_t osKeyboardLeft = 0x06;
const uint8_t osKeyboardRight = 0x07;
const uint8_t osKeyboardUp = 0x08;
const uint8_t osKeyboardDn = 0x09;
const uint8_t osKeyboardSelect = 0x10;
const uint8_t osKeyboardDisabled = 0x11;
const uint8_t menuOpen = 0x12;
const uint8_t menuClose = 0x13;
const uint8_t povModeEnable = 0x14;
const uint8_t povModeDisable = 0x15;

// Currently unused. May be used in future to toggle mouse from OSD menu
const uint8_t mouseEnable = 0x16;
const uint8_t mouseDisable = 0x17;

const int buttonCount = 16;

#define BTN_A 0
#define BTN_B 1
#define BTN_X 2
#define BTN_Y 3
#define BTN_START 4
#define BTN_SELECT 5
#define BTN_L2_HALF 6
#define BTN_R2_HALF 7
#define BTN_L2_FULL 8
#define BTN_R2_FULL 9
#define BTN_R1 10
#define BTN_L1 11
#define BTN_L3 12
#define BTN_R3 13
#define BTN_HOTKEY_MINUS 14
#define BTN_HOTKEY_PLUS 15

extern bool buttonState[buttonCount];

#define DPAD_UP 0
#define DPAD_RIGHT 1
#define DPAD_DOWN 2
#define DPAD_LEFT 3

extern bool dpadPinState[4];

#define JOY_RIGHT_Y 0
#define JOY_RIGHT_X 1
#define JOY_LEFT_Y 2
#define JOY_LEFT_X 3

int8_t mapJoystick(int axis);
void writeJoystickConfig();
bool checkButtons();

// All variables below general use, not used for configuration.
extern bool calibrationMode;
extern int calibrationStep;
extern Toggle menuEnabled;
extern Toggle povHatMode;

// Mouse Variables
extern Toggle mouseEnabled;

extern int xAccelOffset;
extern int yAccelOffset;
extern int zAccelOffset;
extern int xGyroOffset;
extern int yGyroOffset;
extern int zGyroOffset;

extern hid_gamepad_report_t joystick;

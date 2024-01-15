#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

const int buttonCount = 16; // Number of buttons in use. Change length of buttonPins, buttonState and newButtonState to match.

#define BTN_A             0
#define BTN_B             1
#define BTN_X             2
#define BTN_Y             3
#define BTN_START         4
#define BTN_SELECT        5
#define BTN_L2_HALF       6
#define BTN_R2_HALF       7
#define BTN_L2_FULL       8
#define BTN_R2_FULL       9
#define BTN_R1           10
#define BTN_L1           11
#define BTN_L3           12
#define BTN_R3           13
#define BTN_HOTKEY_MINUS 14
#define BTN_HOTKEY_PLUS  15

extern bool buttonState[buttonCount];    // Empty State array for buttons last sent state. Must be same length as buttonPins

#define DPAD_UP    0
#define DPAD_DOWN  1
#define DPAD_LEFT  2
#define DPAD_RIGHT 3

extern bool dpadPinState[4];       // Empty State array for dPad

// All variables below general use, not used for configuration.
extern bool calibrationMode = false;
extern int calibrationStep = 1;
extern bool menuEnabled = false;

// Mouse Variables
extern bool mouseEnabled = false;

extern hid_gamepad_report_t joystick;

uint8_t leftXaxisMap(int i);
uint8_t leftYaxisMap(int i);
uint8_t rightXaxisMap(int i);
uint8_t rightYaxisMap(int i);

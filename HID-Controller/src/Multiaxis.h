#include "Joystick.h" // https://github.com/MHeironimus/ArduinoJoystickLibrary

class Multiaxis_
{
private:

    // Multiaxis Settings
    uint8_t   _hidReportId;

    // Multiaxis State
    int16_t   _xAxis;
    int16_t   _yAxis;
    int16_t   _zAxis;
    int16_t   _xAxisRotation;
    int16_t   _yAxisRotation;
    int16_t   _zAxisRotation;

protected:
    int buildAndSetValue(int16_t axisValue, uint8_t dataLocation[]);

public:
    Multiaxis_(uint8_t hidReportId = JOYSTICK_DEFAULT_REPORT_ID + 1);
    void begin();

    // Set Axis Values
    void setMotion6(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz);
    void sendState();
};

#include "Multiaxis.h"

#define MULTIAXIS_REPORT_SIZE 12

Multiaxis_::Multiaxis_(uint8_t hidReportId)
{
    // Set the USB HID Report ID
    _hidReportId = hidReportId;

	// Initialize Multiaxis State
	_xAxis = 0;
	_yAxis = 0;
	_zAxis = 0;
	_xAxisRotation = 0;
	_yAxisRotation = 0;
	_zAxisRotation = 0;

    uint8_t tempHidReportDescriptor[] = {
		0x05, 0x01,			// USAGE_PAGE (Generic Desktop)
		0x09, 0x08,			// USAGE (Joystick - 0x04, Gamepad - 0x05, Multi-axis Controller - 0x08)
		0xa1, 0x01,			// COLLECTION (Application)
		0x85, _hidReportId,	// REPORT_ID (Default: 4)
		0x05, 0x01,			// USAGE_PAGE (Generic Desktop)
		0x09, 0x08,			// USAGE (Multi-axis)
		0x16, 0x00, 0x80,	// LOGICAL_MINIMUM (-32768)
		0x26, 0XFF, 0X7F,	// LOGICAL_MAXIMUM (32767)
		0x35, 0xFC,			// PHYSICAL_MINIMUM (-4)
		0x45, 0x04,			// PHYSICAL_MAXIMUM (4)
		0x75, 0x10,			// REPORT_SIZE (16)
		0x95, 0x03,			// REPORT_COUNT (3)
		0xA1, 0x00,			// COLLECTION (Physical)
		0x09, 0x30,			// USAGE (X)
		0x09, 0x31,			// USAGE (Y)
		0x09, 0x32,			// USAGE (Z)
		0x81, 0x02,			// INPUT (Data,Var,Abs)
		0xC0,				// END_COLLECTION (Physical)
		0x09, 0x08,			// USAGE (Multi-axis)
		0x16, 0x00, 0x80,	// LOGICAL_MINIMUM (-32768)
		0x26, 0XFF, 0X7F,	// LOGICAL_MAXIMUM (32767)
		0x36, 0x18, 0xFC,	// PHYSICAL_MINIMUM (-1000)
		0x46, 0xE8, 0x03,	// PHYSICAL_MAXIMUM (1000)
		0x75, 0x10,			// REPORT_SIZE (16)
		0x95, 0x03,			// REPORT_COUNT (3)
		0xA1, 0x00,			// COLLECTION (Physical)
		0x09, 0x33,			// USAGE (Rx)
		0x09, 0x34,			// USAGE (Ry)
		0x09, 0x35,			// USAGE (Rz)
		0x81, 0x02,			// INPUT (Data,Var,Abs)
		0xC0,				// END_COLLECTION (Physical)
		0xC0				// END_COLLECTION
	};

	// Create a copy of the HID Report Descriptor template that is just the right size
	uint8_t *customHidReportDescriptor = new uint8_t[sizeof(tempHidReportDescriptor)];
	memcpy(customHidReportDescriptor, tempHidReportDescriptor, sizeof(tempHidReportDescriptor));
	
	// Register HID Report Description
	DynamicHIDSubDescriptor *node = new DynamicHIDSubDescriptor(customHidReportDescriptor, sizeof(tempHidReportDescriptor), false);
	DynamicHID().AppendDescriptor(node);
}

void Multiaxis_::begin()
{
	sendState();
}

void Multiaxis_::setMotion6(int16_t ax, int16_t ay, int16_t az, int16_t gx, int16_t gy, int16_t gz)
{
	_xAxis = ax;
	_yAxis = ay;
	_zAxis = az;
	_xAxisRotation = gx;
	_yAxisRotation = gy;
	_zAxisRotation = gz;
}

int Multiaxis_::buildAndSetValue(int16_t axisValue, uint8_t dataLocation[]) 
{
	dataLocation[0] = (uint8_t)(axisValue & 0x00FF);
	dataLocation[1] = (uint8_t)(axisValue >> 8);
	return 2;
}

void Multiaxis_::sendState()
{
	uint8_t data[MULTIAXIS_REPORT_SIZE];
	int index = 0;

	// Set Axis Values
	index += buildAndSetValue(_xAxis, &(data[index]));
	index += buildAndSetValue(_yAxis, &(data[index]));
	index += buildAndSetValue(_zAxis, &(data[index]));
	index += buildAndSetValue(_xAxisRotation, &(data[index]));
	index += buildAndSetValue(_yAxisRotation, &(data[index]));
	index += buildAndSetValue(_zAxisRotation, &(data[index]));

	DynamicHID().SendReport(_hidReportId, data, MULTIAXIS_REPORT_SIZE);
}

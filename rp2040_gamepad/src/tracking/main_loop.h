#pragma once

#include <stdint.h>

#define ACC_DATA_READY (1 << 0)
#define GYR_DATA_READY (1 << 1)

//bool imu_begin();
//void imu_end();
int imu_read(double* vec);

typedef bool (*MouseMoveCallback)(int8_t x, int8_t y, void* context);

void calibration_begin();
bool calibration_step();
void calibration_end();

void tracking_begin();
void tracking_step(MouseMoveCallback mouse_move, void* context);
void tracking_end();

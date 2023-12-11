#include "main_loop.h"

#include "orientation_tracker.h"
#include "calibration_data.h"

#include <stdio.h>
#include "pico/stdlib.h"

static const float CURSOR_SPEED = 1024.0 / (M_PI / 4);
static const float STABILIZE_BIAS = 16.0;

class TrackingState {
private:
    float yaw;
    float pitch;
    float dYaw;
    float dPitch;
    bool firstRead;
    bool stabilize;
    CalibrationData calibration;
    cardboard::OrientationTracker tracker;

private:
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
        float alpha = (float) std::max(0.0, 1 - std::pow(std::fabs(delta) * CURSOR_SPEED / STABILIZE_BIAS, 3.0));
        return newVal + alpha * delta;
    }

    void sendCurrentState(MouseMoveCallback mouse_move) {
        float dX = dYaw * CURSOR_SPEED;
        float dY = dPitch * CURSOR_SPEED;

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

        const int8_t x = (int8_t)std::floor(dX + 0.5);
        const int8_t y = (int8_t)std::floor(dY + 0.5);

        mouse_move(x, y);

        // Only subtract the part of the error that was already sent.
        if (x != 0) {
            dYaw -= x / CURSOR_SPEED;
        }
        if (y != 0) {
            dPitch -= y / CURSOR_SPEED;
        }
    }

    void onOrientation(cardboard::Vector4& quaternion) {
        float q1 = quaternion[0]; // X * sin(T/2)
        float q2 = quaternion[1]; // Y * sin(T/2)
        float q3 = quaternion[2]; // Z * sin(T/2)
        float q0 = quaternion[3]; // cos(T/2)

        float yaw = std::atan2(2 * (q0 * q3 - q1 * q2), (1 - 2 * (q1 * q1 + q3 * q3)));
        float pitch = std::asin(2 * (q0 * q1 + q2 * q3));
        // float roll = std::atan2(2 * (q0 * q2 - q1 * q3), (1 - 2 * (q1 * q1 + q2 * q2)));

        if (yaw == NAN || pitch == NAN) {
            // NaN case, skip it
            return;
        }

        if (firstRead) {
            this->yaw = yaw;
            this->pitch = pitch;
            firstRead = false;
        } else {
            const float newYaw = highpass(this->yaw, yaw);
            const float newPitch = highpass(this->pitch, pitch);

            float dYaw = clamp(this->yaw - newYaw);
            float dPitch = this->pitch - newPitch;
            this->yaw = newYaw;
            this->pitch = newPitch;

            // Accumulate the error locally.
            this->dYaw += dYaw;
            this->dPitch += dPitch;
        }
    }

public:
    TrackingState()
        : yaw(0)
        , pitch(0)
        , dYaw(0)
        , dPitch(0)
        , firstRead(true)
        , stabilize(true)
        , tracker(10000000l) { // 10 ms / 100 Hz
    }

    void beginCalibration() {
        calibration.reset();
    }

    bool stepCalibration() {
        if (calibration.isComplete())
            return true;

        double vec[6];
        if (imu_read(vec) & GYR_DATA_READY) {
            cardboard::Vector3 data(vec[3], vec[4], vec[5]);
            sleep_ms(9); // Artificially limit to ~100Hz
            return calibration.add(data);
        }

        return false;
    }

    void saveCalibration() {
        /*
        CalibrationMedian store;
        cardboard::Vector3 median = calibration.getMedian();
        store.x = median[0];
        store.y = median[1];
        store.z = median[2];
        CALIBRATION_DATA_SAVE(&store);
        */
    }

    void loadCalibration() {
        /*
        CalibrationMedian store;
        cardboard::Vector3 median = calibration.getMedian();
        if (CALIBRATION_DATA_LOAD(&store)) {
            median[0] = store.x;
            median[1] = store.y;
            median[2] = store.z;
        }

        tracker.SetCalibration(median);
        */
    }

    void beginTracking() {
        loadCalibration();
        tracker.Resume();
    }

    void stepTracking(MouseMoveCallback mouse_move) {
        double vec[6];
        int ret = imu_read(vec);
        if (ret != 0) {
            absolute_time_t now = get_absolute_time();
            uint64_t t = to_us_since_boot(now) * 1000llu;
            if (ret & ACC_DATA_READY) {
                cardboard::AccelerometerData adata
                    = { .system_timestamp = t, .sensor_timestamp_ns = t,
                        .data = cardboard::Vector3(vec[0], vec[1], vec[2]) };
                tracker.OnAccelerometerData(adata);
            }
            if (ret & GYR_DATA_READY) {
                cardboard::GyroscopeData gdata
                    = { .system_timestamp = t, .sensor_timestamp_ns = t,
                        .data = cardboard::Vector3(vec[3], vec[4], vec[5]) };
                cardboard::Vector4 pose = tracker.OnGyroscopeData(gdata);
                onOrientation(pose);
                sendCurrentState(mouse_move);
            }
        }
    }

    void stopTracking() {
        tracker.Pause();
    }
};

static TrackingState g_state;

void calibration_begin() {
    g_state.beginCalibration();
    printf("Calibrating");
}

bool calibration_step() {
    return g_state.stepCalibration();
}

void calibration_end() {
    g_state.saveCalibration();
}

void tracking_begin() {
    g_state.beginTracking();
}

void tracking_step(MouseMoveCallback mouse_move) {
    g_state.stepTracking(mouse_move);
}

void tracking_end() {
    g_state.stopTracking();
}

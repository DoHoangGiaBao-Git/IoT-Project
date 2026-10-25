#pragma once

#include "config.h"
#include "sensor_data.h"

struct AlarmState {
    bool highTemperature = false;
    bool smokeDetected = false;

    bool active() const { return highTemperature || smokeDetected; }
};

// Khong phu thuoc GPIO/RTOS, co the tai su dung cho cac buoc sau.
inline AlarmState evaluateAlarm(const SensorReading& reading, AlarmState previous) {
    // DHT loi: giu canh bao nhiet do cu den khi co mau hop le.
    if (reading.temperatureValid) {
        if (reading.temperatureC >= Config::TEMP_ON_C) previous.highTemperature = true;
        else if (reading.temperatureC <= Config::TEMP_OFF_C) previous.highTemperature = false;
    }
    if (reading.smokeReady) {
        if (reading.smokeRaw >= Config::SMOKE_ON) previous.smokeDetected = true;
        else if (reading.smokeRaw <= Config::SMOKE_OFF) previous.smokeDetected = false;
    }
    return previous;
}


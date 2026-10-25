#pragma once

#include <stdint.h>

// Ban sao du lieu gui qua queue; khong chia se bien cam bien giua cac task.
struct SensorReading {
    uint32_t uptimeMs;
    float temperatureC;
    uint16_t smokeRaw;
    bool temperatureValid;
    bool smokeReady;
};


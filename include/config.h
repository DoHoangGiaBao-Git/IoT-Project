#pragma once

#include <stdint.h>

namespace Config {
constexpr uint8_t DHT_PIN = 4;
constexpr uint8_t MQ2_PIN = 34;  // ADC1, doc chan AO qua chia ap.
constexpr uint8_t LED_PIN = 25;
constexpr uint8_t BUZZER_PIN = 26;  // Buzzer chu dong, HIGH = keu.

constexpr uint32_t SAMPLE_MS = 2000;  // DHT11: khong doc qua nhanh.
constexpr uint32_t STALE_MS = 6000;
constexpr uint32_t MQ2_WARMUP_MS = 60000;  // Cho on dinh sau khi bat nguon.
constexpr uint32_t FAULT_BLINK_MS = 500;

// Nguong demo, can hieu chinh tren phan cung thuc te.
constexpr float TEMP_ON_C = 50.0f;
constexpr float TEMP_OFF_C = 45.0f;
constexpr uint16_t SMOKE_ON = 2000;   // ADC raw 0..4095, KHONG phai ppm.
constexpr uint16_t SMOKE_OFF = 1700;

static_assert(TEMP_OFF_C < TEMP_ON_C, "Temperature hysteresis invalid");
static_assert(SMOKE_OFF < SMOKE_ON, "Smoke hysteresis invalid");
static_assert(SAMPLE_MS >= 2000 && STALE_MS > SAMPLE_MS, "Timing invalid");
}  // namespace Config


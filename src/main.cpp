#include <Arduino.h>
#include <DHT.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <math.h>

#include "alarm_logic.h"

static DHT dht(Config::DHT_PIN, DHT11);
static QueueHandle_t readingsQueue;

static void sensorTask(void*) {
    TickType_t lastWake = xTaskGetTickCount();
    const uint32_t startedAt = millis();
    for (;;) {
        SensorReading reading{};
        reading.temperatureC = dht.readTemperature();
        reading.temperatureValid = isfinite(reading.temperatureC);
        reading.smokeRaw = analogRead(Config::MQ2_PIN);
        reading.uptimeMs = millis();
        reading.smokeReady = (reading.uptimeMs - startedAt >= Config::MQ2_WARMUP_MS);
        // Queue 1 phan tu: luon giu mau moi nhat, khong tich tu mau cu.
        xQueueOverwrite(readingsQueue, &reading);
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(Config::SAMPLE_MS));
    }
}

static void alarmTask(void*) {
    AlarmState state;
    bool fault = true;
    uint32_t lastSampleMs = millis();
    for (;;) {
        SensorReading reading{};
        if (xQueueReceive(readingsQueue, &reading, pdMS_TO_TICKS(100)) == pdTRUE) {
            lastSampleMs = reading.uptimeMs;
            state = evaluateAlarm(reading, state);
            fault = !reading.temperatureValid || !reading.smokeReady;
            Serial.printf("T=%.1f C | MQ2=%u | DHT=%s | MQ2=%s | ALARM=%s\n",
                          reading.temperatureC, static_cast<unsigned>(reading.smokeRaw),
                          reading.temperatureValid ? "OK" : "ERROR",
                          reading.smokeReady ? "READY" : "WARMUP",
                          state.active() ? "ON" : "OFF");
        }

        const bool stale = (millis() - lastSampleMs >= Config::STALE_MS);
        // Co nguy co: LED sang + coi lien tuc. Khoi dong/loi: LED nhap nhay.
        const bool blink = ((millis() / Config::FAULT_BLINK_MS) % 2) != 0;
        digitalWrite(Config::LED_PIN, state.active() || ((fault || stale) && blink));
        digitalWrite(Config::BUZZER_PIN, state.active());
    }
}

static void startupFailure() {
    Serial.println("ERROR: Khong tao duoc queue/task. Restart sau 1 giay.");
    delay(1000);
    ESP.restart();
    for (;;) { delay(1000); }
}

void setup() {
    Serial.begin(115200);
    pinMode(Config::LED_PIN, OUTPUT);
    pinMode(Config::BUZZER_PIN, OUTPUT);
    digitalWrite(Config::LED_PIN, LOW);
    digitalWrite(Config::BUZZER_PIN, LOW);
    pinMode(Config::MQ2_PIN, INPUT);
    analogReadResolution(12);
    analogSetPinAttenuation(Config::MQ2_PIN, ADC_11db);
    dht.begin();

    readingsQueue = xQueueCreate(1, sizeof(SensorReading));
    if (readingsQueue == nullptr) startupFailure();
    if (xTaskCreate(alarmTask, "alarm", 3072, nullptr, 2, nullptr) != pdPASS) startupFailure();
    if (xTaskCreate(sensorTask, "sensor", 3072, nullptr, 1, nullptr) != pdPASS) startupFailure();
}

void loop() {
    // Cong viec da duoc xu ly trong hai task FreeRTOS.
    vTaskDelete(nullptr);
}

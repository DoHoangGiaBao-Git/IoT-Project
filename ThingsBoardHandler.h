#ifndef ThingsBoardHandler_H
#define ThingsBoardHandler_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Arduino_MQTT_Client.h>
#include <ThingsBoard.h>

// Network & ThingsBoard Credentials
const char* WIFI_SSID = "UET-Wifi-Office-Free 2.4Ghz"; // Wifi Name
const char* WIFI_PASSWORD = ""; // Wifi Password
const char* TB_SERVER = "thingsboard.cloud"; // ThingsBoard Cloud Server
constexpr uint16_t TB_PORT = 8883U; // ThingsBoard Cloud MQTT TLS port
const char* TOKEN = "kylk41rG2hBPJ5ZEyQ15"; // ThingsBoard Access Token

const unsigned long WIFI_TIMEOUT = 10000;

// Create a MQTT client to connect to ThingsBoard
WiFiClientSecure secureClient;
Arduino_MQTT_Client mqttClient(secureClient);
extern ThingsBoard tb(mqttClient);

void initConnection() {
    unsigned long timestamp = millis();
    secureClient.setInsecure();

    // Connect to Wi-Fi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while ((WiFi.status() != WL_CONNECTED) && (millis() - timestamp < WIFI_TIMEOUT)) {
        delay(500);
    }

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to Wi-Fi");
        return; // Replace return with 4G module connection later.
    }
}

void connectToThingsBoard() {
    if (!tb.connected()) {
        if (!tb.connect(TB_SERVER, TOKEN, TB_PORT)) {
            Serial.println("Failed to connect to ThingsBoard");
            delay(5000); // Wait 5 seconds before retrying if connection fails
            return;
        }
    }
}

void sendTelemetryData(const char* key, float value) {
    tb.sendTelemetryData(key, value);
}

#endif
#ifndef WIFIAPMANAGER_H
#define WIFIAPMANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

/**
 * @brief Infrastructure Manager untuk Wi-Fi Access Point
 * Single Responsibility: Mengelola inisialisasi Wi-Fi SoftAP, IP Address, dan Tx Power.
 */
class WiFiAPManager {
public:
    WiFiAPManager();

    bool begin(const char* ssid = "Indikator External Display", const char* password = "12345678");
    String getIPAddress() const;
};

#endif // WIFIAPMANAGER_H

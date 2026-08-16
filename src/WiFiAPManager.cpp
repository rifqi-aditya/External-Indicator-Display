#include "WiFiAPManager.h"

WiFiAPManager::WiFiAPManager() {}

bool WiFiAPManager::begin(const char* ssid, const char* password) {
    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(ssid, password);
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
    delay(300);
    return result;
}

String WiFiAPManager::getIPAddress() const {
    return WiFi.softAPIP().toString();
}

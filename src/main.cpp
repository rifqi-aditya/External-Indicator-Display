#include <Arduino.h>
#include "ConfigManager.h"
#include "ScaleParser.h"
#include "WebPortal.h"
#include "DisplayManager.h"

// ============================================================================
// EXTERNAL DISPLAY SCALE INDICATOR + WEB PORTAL (AP MODE)
// Board: ESP32-WROOM-32U (Dual-Core 240MHz + External IPEX Antenna)
// Indicator: Yaohua XK3190-A12E (RS232 Format: wn-003.17kg @ 9600 Baud 8N1)
// Display: P10 HUB12 DIP (64x16 Pixels)
// Architecture: Clean Architecture + Single Responsibility Principle (SRP)
// ============================================================================

#define PIN_OE   4  // P10 OE (Pin D4)
#define PIN_A    21 // P10 A (Pin D21)
#define PIN_B    19 // P10 B (Pin D19)
#define PIN_CLK  18 // P10 CLK (Pin D18)
#define PIN_LAT  5  // P10 LAT/STB (Pin D5)
#define PIN_DATA 23 // P10 DATA/R1 (Pin D23)
#define PIN_LED  2  // On-board LED ESP32 WROOM-32 (Pin D2)
#define PIN_RX   25 // Serial 2 RX2 (Pin D25) <- Dari TXD MAX3232
#define PIN_TX   26 // Serial 2 TX2 (Pin D26) -> Dari RXD MAX3232

// Objek Utama (Clean Architecture Layers)
DisplayManager displayManager(2, 1);
ConfigManager  configMgr;
ScaleParser    scaleParser(PIN_RX, PIN_TX);
WebPortal      webPortal(configMgr, scaleParser);

AppConfig config;

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println("\n==================================================");
    Serial.println("  ESP32-WROOM-32U YAOHUA A12E SCALE INDICATOR P10 ");
    Serial.println("  Clean Architecture & Single Responsibility Mode ");
    Serial.println("==================================================");

    pinMode(PIN_LED, OUTPUT);

    // 1. Load NVS Preferences
    configMgr.begin();
    configMgr.loadConfig(config);
    config.displayMode = MODE_LIVE_SCALE; // Force default Mode 0: Live Weight Timbangan
    config.baudRate = 9600; // Force 9600 Baud

    Serial.printf("[BOOT] Config: Mode=%d, Baud=%lu, Text='%s'\n", 
                  config.displayMode, config.baudRate, config.staticText.c_str());

    // 2. Start RS232 Scale Parser (Yaohua A12E Spec @ 9600 Baud)
    scaleParser.begin(config.baudRate);
    Serial.printf("[BOOT] RS232 Serial2 Listening on RX=GPIO%d, TX=GPIO%d @ %lu Baud\n", 
                  PIN_RX, PIN_TX, config.baudRate);

    // 3. Start Wi-Fi AP & Web Portal
    webPortal.begin(config);

    // 4. Start P10 Display Driver & Renderer
    P10Pins pins = {
        .pin_oe   = PIN_OE,
        .pin_a    = PIN_A,
        .pin_b    = PIN_B,
        .pin_clk  = PIN_CLK,
        .pin_lat  = PIN_LAT,
        .pin_data = PIN_DATA
    };
    displayManager.begin(pins);

    Serial.println("[BOOT] System Fully Ready!\n");
}

void loop() {
    unsigned long now = millis();

    // 1. Update Hardware Communication & Web Service
    scaleParser.update();
    webPortal.update();

    // 2. Realtime Stream Log ke Serial Monitor (Dinonaktifkan untuk Mode Produksi agar tidak ada overhead UART)
    if (scaleParser.hasNewFrame()) {
        // Serial.printf("[RS232 REALTIME] RAW: \"%s\"  =>  PARSED: %s\n", 
        //               scaleParser.getLastRawData().c_str(), 
        //               scaleParser.getWeightString().c_str());
    }

    // 3. Update Konfigurasi jika diubah via Web UI
    if (webPortal.isConfigUpdated()) {
        config = webPortal.getConfig();
        webPortal.clearConfigUpdatedFlag();
        displayManager.notifyConfigChanged();
        Serial.println("[SYSTEM] Configuration updated via Web UI");
    }

    // 4. Update Render Display P10
    displayManager.update(config, scaleParser.getWeightString());

    // 5. Heartbeat & LED Indicator
    static unsigned long lastHeartbeat = 0;
    if (now - lastHeartbeat > 5000) {
        lastHeartbeat = now;
        if (!scaleParser.isConnected()) {
            Serial.println("[RS232 STATUS] Menunggu sinyal data dari MAX3232 di RX=GPIO25...");
        }
    }

    static unsigned long lastBlink = 0;
    if (now - lastBlink > 500) {
        lastBlink = now;
        digitalWrite(PIN_LED, !digitalRead(PIN_LED));
    }

    delay(5);
}
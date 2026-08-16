#ifndef WEBPORTAL_H
#define WEBPORTAL_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>
#include "ConfigManager.h"
#include "ScaleParser.h"
#include "WiFiAPManager.h"

/**
 * @brief Web Server Controller & Presentation Layer untuk Web UI Setup
 * Single Responsibility: Mengelola endpoint HTTP REST API dan LittleFS Static File Serving.
 */
class WebPortal {
public:
    WebPortal(ConfigManager &configMgr, ScaleParser &scaleParser);
    void begin(AppConfig &currentConfig);
    void update();

    bool isConfigUpdated() const { return _configUpdated; }
    void clearConfigUpdatedFlag() { _configUpdated = false; }
    AppConfig getConfig() const { return _config; }

private:
    ConfigManager &_configMgr;
    ScaleParser &_scaleParser;
    WiFiAPManager _wifiManager;
    WebServer _server;
    AppConfig _config;
    bool _configUpdated;
    bool _fsReady;

    void handleRoot();
    void handleSave();
    void handleApiWeight();
    void handleApiConfig();
    bool handleFileRead(String path);
};

#endif // WEBPORTAL_H

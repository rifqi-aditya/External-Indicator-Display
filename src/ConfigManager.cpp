#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

void ConfigManager::begin() {
    _prefs.begin("p10scale", false);
}

void ConfigManager::loadConfig(AppConfig &config) {
    _prefs.begin("p10scale", true);
    config.displayMode  = _prefs.getUChar("mode", 0);
    config.staticText   = _prefs.isKey("stext") ? _prefs.getString("stext") : "SYMETRA LAB";
    config.runningText  = _prefs.isKey("rtext") ? _prefs.getString("rtext") : "EXTERNAL DISPLAY TIMBANGAN DIGITAL - READY";
    config.baudRate     = _prefs.getULong("baud", 9600);
    config.scrollSpeed  = _prefs.getUChar("speed", 35);
    _prefs.end();
}

void ConfigManager::saveConfig(const AppConfig &config) {
    _prefs.begin("p10scale", false);
    _prefs.putUChar("mode", config.displayMode);
    _prefs.putString("stext", config.staticText);
    _prefs.putString("rtext", config.runningText);
    _prefs.putULong("baud", config.baudRate);
    _prefs.putUChar("speed", config.scrollSpeed);
    _prefs.end();
}

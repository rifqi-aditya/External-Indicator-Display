#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <Arduino.h>
#include <Preferences.h>

enum DisplayMode {
    MODE_LIVE_SCALE = 0,
    MODE_STATIC_TEXT = 1,
    MODE_RUNNING_TEXT = 2
};

struct AppConfig {
    uint8_t displayMode;    // 0: Live Scale, 1: Static Text, 2: Running Text
    String staticText;
    String runningText;
    uint32_t baudRate;
    uint8_t scrollSpeed;
};

class ConfigManager {
public:
    ConfigManager();
    void begin();
    void loadConfig(AppConfig &config);
    void saveConfig(const AppConfig &config);

private:
    Preferences _prefs;
};

#endif // CONFIGMANAGER_H

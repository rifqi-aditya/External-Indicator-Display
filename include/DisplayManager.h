#ifndef DISPLAYMANAGER_H
#define DISPLAYMANAGER_H

#include <Arduino.h>
#include "P10Display.h"
#include "ConfigManager.h"

/**
 * @brief Application Presentation Manager untuk Display P10
 * Menerapkan Single Responsibility Principle (SRP):
 * Mengelola logika mode tampilan (Live Weight, Static Text, Running Text),
 * animasi running text, dan urutan pembaruan layar P10.
 */
class DisplayManager {
public:
    DisplayManager(int panelsWide = 2, int panelsHigh = 1);
    
    void begin(P10Pins pins);
    void update(const AppConfig &config, const String &weightStr);
    void notifyConfigChanged();

private:
    P10Display _display;
    int _scrollPos;
    unsigned long _lastScrollTime;
    unsigned long _lastLiveUpdateTime;
    bool _needsRedraw;
    String _lastWeightStr;
};

#endif // DISPLAYMANAGER_H

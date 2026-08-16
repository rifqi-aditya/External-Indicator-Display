#include "DisplayManager.h"

DisplayManager::DisplayManager(int panelsWide, int panelsHigh)
    : _display(panelsWide, panelsHigh), 
      _scrollPos(64), 
      _lastScrollTime(0), 
      _lastLiveUpdateTime(0), 
      _needsRedraw(true) {}

void DisplayManager::begin(P10Pins pins) {
    _display.begin(pins);
    _display.clear();
    _scrollPos = _display.getWidth();
}

void DisplayManager::notifyConfigChanged() {
    _needsRedraw = true;
    _scrollPos = _display.getWidth();
}

void DisplayManager::update(const AppConfig &config, const String &weightStr) {
    unsigned long now = millis();

    switch (config.displayMode) {
        case MODE_LIVE_SCALE: {
            if (_needsRedraw || weightStr != _lastWeightStr || now - _lastLiveUpdateTime > 100) {
                if (_needsRedraw || weightStr != _lastWeightStr) {
                    _needsRedraw = false;
                    _lastWeightStr = weightStr;
                    _lastLiveUpdateTime = now;
                    _display.clear();
                    _display.drawScaleWeightAlignedRight(weightStr.c_str());
                    _display.commit();
                }
            }
            break;
        }

        case MODE_STATIC_TEXT: {
            if (_needsRedraw) {
                _needsRedraw = false;
                _display.clear();
                _display.drawScaleStrCenter(config.staticText.c_str());
                _display.commit();
            }
            break;
        }

        case MODE_RUNNING_TEXT: {
            if (now - _lastScrollTime > config.scrollSpeed) {
                _lastScrollTime = now;
                _display.clear();
                _display.drawScaleStr(_scrollPos, config.runningText.c_str());
                _display.commit();
                _scrollPos--;
                if (_scrollPos < -_display.getScaleStrWidth(config.runningText.c_str())) {
                    _scrollPos = _display.getWidth();
                }
            }
            break;
        }
    }
}

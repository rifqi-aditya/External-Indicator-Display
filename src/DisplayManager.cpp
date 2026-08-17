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

        case MODE_STATIC_TEXT: { // Mode 1: Smart Custom Text (Auto Statis / Running)
            const char* txt = config.staticText.length() > 0 ? config.staticText.c_str() : config.runningText.c_str();
            int strWidth = _display.getScaleStrWidth(txt);

            if (strWidth <= _display.getWidth()) {
                // Teks muat di layar -> Tampil Statis Ditengah
                if (_needsRedraw) {
                    _needsRedraw = false;
                    _display.clear();
                    _display.drawScaleStrCenter(txt);
                    _display.commit();
                }
            } else {
                // Teks melebihi lebar layar -> Otomatis Running Text (Berjalan)
                if (now - _lastScrollTime > (config.scrollSpeed > 0 ? config.scrollSpeed : 40)) {
                    _lastScrollTime = now;
                    _display.clear();
                    _display.drawScaleStr(_scrollPos, txt);
                    _display.commit();
                    _scrollPos--;
                    if (_scrollPos < -strWidth) {
                        _scrollPos = _display.getWidth();
                    }
                }
            }
            break;
        }

        case MODE_RUNNING_TEXT: { // Mode 2: Paksa Running Text (Berjalan)
            const char* txt = config.runningText.length() > 0 ? config.runningText.c_str() : config.staticText.c_str();
            int strWidth = _display.getScaleStrWidth(txt);

            if (now - _lastScrollTime > (config.scrollSpeed > 0 ? config.scrollSpeed : 40)) {
                _lastScrollTime = now;
                _display.clear();
                _display.drawScaleStr(_scrollPos, txt);
                _display.commit();
                _scrollPos--;
                if (_scrollPos < -strWidth) {
                    _scrollPos = _display.getWidth();
                }
            }
            break;
        }
    }
}


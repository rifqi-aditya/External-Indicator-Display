#ifndef P10DISPLAY_H
#define P10DISPLAY_H

#include <Arduino.h>

struct P10Pins {
    int pin_oe;
    int pin_a;
    int pin_b;
    int pin_clk;
    int pin_lat;
    int pin_data;
};

class P10Display {
public:
    P10Display(int panelsWide = 2, int panelsHigh = 1);
    ~P10Display();

    void begin(P10Pins pins);
    void clear();
    void setPixel(int x, int y, bool on);
    void drawRect(int x, int y, int w, int h);
    
    // Standard 5x7 Font
    void drawChar(int x, int y, char c);
    void drawStr(int x, int y, const char* s);
    void drawStrCenter(int y, const char* s);
    int getStrWidth(const char* s);

    // ScaleFont16 (Proportional 14px Rounded BOLD Font for All Display Modes)
    void drawScaleChar(int x, int y, char c);
    void drawScaleStr(int x, const char* s);
    void drawScaleStrCenter(const char* s);
    int getScaleCharWidth(char c);
    int getScaleStrWidth(const char* s);

    void drawScaleWeightAlignedRight(const char* fullWeightStr);

    void commit();

    int getWidth() const { return _width; }
    int getHeight() const { return _height; }

private:
    int _panelsWide;
    int _panelsHigh;
    int _width;
    int _height;
    int _stride;
    P10Pins _pins;

    uint8_t** _drawFb;
    uint8_t** _scanFb;

    static P10Display* _instance;
    static void scanTask(void* pvParameters);
    void runScanLoop();
    void shiftByte(uint8_t val);
};

#endif // P10DISPLAY_H

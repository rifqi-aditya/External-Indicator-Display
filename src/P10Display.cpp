#include "P10Display.h"
#include "Fonts.h"

P10Display* P10Display::_instance = nullptr;

// Bit reversal table (MSB <-> LSB) untuk P10 DIP PCB zigzag layout
static const uint8_t flipBits[256] PROGMEM = {
    0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
    0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
    0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
    0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
    0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
    0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
    0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
    0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
    0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
    0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
    0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
    0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
    0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
    0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
    0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
    0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

// Canvas Buffer Dinamis
static uint8_t** _fb = nullptr;

P10Display::P10Display(int panelsWide, int panelsHigh)
    : _panelsWide(panelsWide), _panelsHigh(panelsHigh), _drawFb(nullptr), _scanFb(nullptr) {
    _width = _panelsWide * 32;
    _height = _panelsHigh * 16;
    _stride = _width / 8;
    _instance = this;
}

P10Display::~P10Display() {
    if (_drawFb) {
        for (int i = 0; i < _height; i++) {
            delete[] _drawFb[i];
        }
        delete[] _drawFb;
    }
    if (_scanFb) {
        for (int i = 0; i < _height; i++) {
            delete[] _scanFb[i];
        }
        delete[] _scanFb;
    }
}

void P10Display::begin(P10Pins pins) {
    _pins = pins;

    // Alokasi memori Framebuffer Double-Buffering
    _drawFb = new uint8_t*[_height];
    _scanFb = new uint8_t*[_height];
    for (int i = 0; i < _height; i++) {
        _drawFb[i] = new uint8_t[_stride];
        _scanFb[i] = new uint8_t[_stride];
        memset(_drawFb[i], 0x00, _stride);
        memset(_scanFb[i], 0x00, _stride);
    }

    // Inisialisasi Pin GPIO
    int p[] = {_pins.pin_oe, _pins.pin_a, _pins.pin_b, _pins.pin_clk, _pins.pin_lat, _pins.pin_data};
    for (int i = 0; i < 6; i++) {
        pinMode(p[i], OUTPUT);
        digitalWrite(p[i], LOW);
    }
    digitalWrite(_pins.pin_oe, HIGH); // Blanking awal

    // Jalankan FreeRTOS Scanning Task khusus pada Core 0 dengan Prioritas 3
    // sehingga scanning display tidak pernah terganggu oleh loop() / Serial / RS232 di Core 1
    xTaskCreatePinnedToCore(scanTask, "P10ScanTask", 4096, this, 3, NULL, 0);
}

void P10Display::clear() {
    if (!_drawFb) return;
    for (int i = 0; i < _height; i++) {
        memset(_drawFb[i], 0x00, _stride);
    }
}

void P10Display::commit() {
    if (!_drawFb || !_scanFb) return;
    for (int i = 0; i < _height; i++) {
        memcpy(_scanFb[i], _drawFb[i], _stride);
    }
}

void P10Display::setPixel(int x, int y, bool on) {
    if (x < 0 || x >= _width || y < 0 || y >= _height || !_drawFb) return;
    int byteIdx = x / 8;
    int bitIdx = 7 - (x % 8);
    if (on) {
        _drawFb[y][byteIdx] |= (1 << bitIdx);
    } else {
        _drawFb[y][byteIdx] &= ~(1 << bitIdx);
    }
}

void P10Display::drawRect(int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++) {
        setPixel(i, y, true);
        setPixel(i, y + h - 1, true);
    }
    for (int j = y; j < y + h; j++) {
        setPixel(x, j, true);
        setPixel(x + w - 1, j, true);
    }
}

void P10Display::drawChar(int x, int y, char c) {
    int idx = getGlyphIndex(c);
    const uint8_t* glyph = Font5x7[idx];
    for (int col = 0; col < 5; col++) {
        uint8_t line = pgm_read_byte(&glyph[col]);
        for (int row = 0; row < 7; row++) {
            bool on = (line >> row) & 0x01;
            setPixel(x + col, y + row, on);
        }
    }
}

int P10Display::getStrWidth(const char* s) {
    int w = 0;
    while (*s) {
        if (*s == ' ') w += 3;
        else w += 6;
        s++;
    }
    return w > 0 ? w - 1 : 0;
}

void P10Display::drawStr(int x, int y, const char* s) {
    while (*s) {
        if (*s == ' ') {
            x += 3;
        } else {
            drawChar(x, y, *s);
            x += 6;
        }
        s++;
    }
}

void P10Display::drawStrCenter(int y, const char* s) {
    int w = getStrWidth(s);
    int x = (_width - w) / 2;
    if (x < 0) x = 0;
    drawStr(x, y, s);
}

int P10Display::getScaleCharWidth(char c) {
    return getScaleFontWidth(c);
}

int P10Display::getScaleStrWidth(const char* s) {
    int w = 0;
    while (*s) {
        int cw = getScaleCharWidth(*s);
        if (cw > 0) {
            w += cw + 1; // Spasi 1px antar karakter
        } else {
            w += 3;
        }
        s++;
    }
    return w > 0 ? w - 1 : 0;
}

void P10Display::drawScaleChar(int x, int y, char c) {
    uint8_t charWidth = getScaleFontWidth(c);
    const uint8_t* glyph = getScaleFontData(c);
    if (!glyph || charWidth == 0) return;

    for (int col = 0; col < charWidth; col++) {
        uint8_t byte0 = pgm_read_byte(&glyph[col * 2]);     // Bit 0..7 (Baris 0..7)
        uint8_t byte1 = pgm_read_byte(&glyph[col * 2 + 1]); // Bit 8..15 (Baris 8..15)

        for (int row = 0; row < 8; row++) {
            setPixel(x + col, y + row, (byte0 >> row) & 0x01);
        }
        for (int row = 0; row < 8; row++) {
            setPixel(x + col, y + 8 + row, (byte1 >> row) & 0x01);
        }
    }
}

void P10Display::drawScaleStr(int x, const char* s) {
    while (*s) {
        char c = *s;
        drawScaleChar(x, 0, c);
        int cw = getScaleCharWidth(c);
        if (cw > 0) {
            x += cw + 1;
        } else {
            x += 3;
        }
        s++;
    }
}

void P10Display::drawScaleStrCenter(const char* s) {
    int w = getScaleStrWidth(s);
    int x = (_width - w) / 2;
    if (x < 0) x = 0;
    drawScaleStr(x, s);
}

void P10Display::drawScaleWeightAlignedRight(const char* fullWeightStr) {
    String raw = String(fullWeightStr);
    raw.trim();

    // Hilangkan suffix KG bila ada
    if (raw.endsWith("KG") || raw.endsWith("kg")) {
        raw = raw.substring(0, raw.length() - 2);
        raw.trim();
    }

    int weight = round(raw.toFloat());
    String fullStr = String(weight) + " KG";

    int totalWidth = getScaleStrWidth(fullStr.c_str());
    int xStart = _width - totalWidth;
    if (xStart < 0) xStart = 0;

    drawScaleStr(xStart, fullStr.c_str());
}

inline void P10Display::shiftByte(uint8_t val) {
    val = ~val; // Active LOW
    for (int i = 7; i >= 0; i--) {
        if (val & (1 << i)) {
            GPIO.out_w1ts = (1 << _pins.pin_data);
        } else {
            GPIO.out_w1tc = (1 << _pins.pin_data);
        }
        GPIO.out_w1ts = (1 << _pins.pin_clk);
        delayMicroseconds(1);
        GPIO.out_w1tc = (1 << _pins.pin_clk);
        delayMicroseconds(1);
    }
}

void P10Display::scanTask(void* pvParameters) {
    P10Display* display = (P10Display*)pvParameters;
    display->runScanLoop();
}

void P10Display::runScanLoop() {
    while (true) {
        for (int phase = 0; phase < 4; phase++) {
            // 1. BLANKING: OE HIGH
            GPIO.out_w1ts = (1 << _pins.pin_oe);
            delayMicroseconds(5); // Dead-time discharge

            // 2. DATA SHIFT
            bool flipRow = ((_height & 0x10) == 0);

            for (int y = 0; y < _height; y += 16) {
                if (!flipRow) {
                    uint8_t* d0 = _scanFb[y + phase];
                    uint8_t* d1 = _scanFb[y + phase + 4];
                    uint8_t* d2 = _scanFb[y + phase + 8];
                    uint8_t* d3 = _scanFb[y + phase + 12];
                    for (int x = 0; x < _stride; x++) {
                        shiftByte(d3[x]);
                        shiftByte(d2[x]);
                        shiftByte(d1[x]);
                        shiftByte(d0[x]);
                    }
                    flipRow = true;
                } else {
                    uint8_t* d0 = _scanFb[y + 16 - 1 - phase];
                    uint8_t* d1 = _scanFb[y + 16 - 1 - phase - 4];
                    uint8_t* d2 = _scanFb[y + 16 - 1 - phase - 8];
                    uint8_t* d3 = _scanFb[y + 16 - 1 - phase - 12];
                    for (int x = _stride - 1; x >= 0; x--) {
                        shiftByte(pgm_read_byte(&(flipBits[d3[x]])));
                        shiftByte(pgm_read_byte(&(flipBits[d2[x]])));
                        shiftByte(pgm_read_byte(&(flipBits[d1[x]])));
                        shiftByte(pgm_read_byte(&(flipBits[d0[x]])));
                    }
                    flipRow = false;
                }
            }

            // 3. ADDRESS SELECT: A & B
            if (phase & 1) GPIO.out_w1ts = (1 << _pins.pin_a);
            else           GPIO.out_w1tc = (1 << _pins.pin_a);
            if (phase & 2) GPIO.out_w1ts = (1 << _pins.pin_b);
            else           GPIO.out_w1tc = (1 << _pins.pin_b);

            // 4. LATCH PULSE
            GPIO.out_w1ts = (1 << _pins.pin_lat);
            delayMicroseconds(2);
            GPIO.out_w1tc = (1 << _pins.pin_lat);
            delayMicroseconds(2);

            // 5. DISPLAY ON: OE LOW
            GPIO.out_w1tc = (1 << _pins.pin_oe);

            // 6. ON-TIME DURATION (450 µs)
            delayMicroseconds(450);

            // 7. BLANKING AKHIR
            GPIO.out_w1ts = (1 << _pins.pin_oe);
            delayMicroseconds(5);
        }
        // Yield sejenak (1 tick) untuk stabilitas Task Watchdog & Core 0
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

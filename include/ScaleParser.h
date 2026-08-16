#ifndef SCALEPARSER_H
#define SCALEPARSER_H

#include <Arduino.h>
#include "protocols/IProtocolParser.h"
#include "protocols/YaohuaA12EParser.h"

/**
 * @brief Hardware Serial Manager & Scale Controller
 * Menerapkan Single Responsibility Principle (SRP):
 * Mengelola komunikasi Hardware Serial2 UART, pembacaan byte, framing buffer,
 * serta mendelongasikan ekstraksi protokol ke IProtocolParser.
 */
class ScaleParser {
public:
    ScaleParser(int rxPin = 25, int txPin = 26, IProtocolParser* parser = nullptr);
    ~ScaleParser();

    void begin(uint32_t baudRate);
    void update();
    void setBaudRate(uint32_t baudRate);

    String getWeightString() const { return _weightStr; }
    String getLastRawData() const { return _lastRawData; }
    float getNumericWeight() const { return _numericWeight; }
    bool hasNewFrame();
    bool isConnected() const;

private:
    void feedChar(char c);
    int _rxPin;
    int _txPin;
    uint32_t _baudRate;
    String _rxBuffer;
    String _lastRawData;
    String _weightStr;
    float _numericWeight;
    bool _newFrameFlag;
    bool _ownsParser;
    unsigned long _lastRxTime;

    IProtocolParser* _protocolParser;
};

#endif // SCALEPARSER_H

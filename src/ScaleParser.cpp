#include "ScaleParser.h"

ScaleParser::ScaleParser(int rxPin, int txPin, IProtocolParser* parser)
    : _rxPin(rxPin), 
      _txPin(txPin), 
      _baudRate(9600), 
      _lastRawData(""), 
      _weightStr("0.0 KG"), 
      _numericWeight(0.0f),
      _newFrameFlag(false), 
      _ownsParser(false),
      _lastRxTime(0),
      _protocolParser(parser) 
{
    if (!_protocolParser) {
        _protocolParser = new YaohuaA12EParser();
        _ownsParser = true;
    }
}

ScaleParser::~ScaleParser() {
    if (_ownsParser && _protocolParser) {
        delete _protocolParser;
        _protocolParser = nullptr;
    }
}

void ScaleParser::setProtocolParser(IProtocolParser* parser) {
    if (_ownsParser && _protocolParser) {
        delete _protocolParser;
    }
    _protocolParser = parser;
    _ownsParser = false;
}

void ScaleParser::begin(uint32_t baudRate) {
    _baudRate = baudRate;
    Serial2.begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);
}

void ScaleParser::setBaudRate(uint32_t baudRate) {
    if (_baudRate != baudRate) {
        _baudRate = baudRate;
        Serial2.end();
        Serial2.begin(_baudRate, SERIAL_8N1, _rxPin, _txPin);
    }
}

void ScaleParser::feedChar(char c) {
    _lastRxTime = millis();

    // Pemicu parse & reset buffer pada enter (\r, \n) atau saat 'g' (akhir 'kg')
    if (c == '\n' || c == '\r' || c == 'g') {
        if (c == 'g') _rxBuffer += c;
        if (_rxBuffer.length() >= 4) {
            _lastRawData = _rxBuffer;
            _newFrameFlag = true;
            
            // Clean Architecture: Delegasikan ekstraksi ke IProtocolParser
            if (_protocolParser) {
                float parsedWeight = 0.0f;
                if (_protocolParser->parse(_rxBuffer, parsedWeight)) {
                    _numericWeight = parsedWeight;
                    _weightStr = _protocolParser->formatWeight(parsedWeight);
                }
            }
        }
        _rxBuffer = "";
    } else if ((uint8_t)c >= 32 && (uint8_t)c <= 126) {
        _rxBuffer += c;
    }
}

void ScaleParser::update() {
    while (Serial2.available()) {
        char c = (char)Serial2.read();
        feedChar(c);
    }
}

bool ScaleParser::hasNewFrame() {
    if (_newFrameFlag) {
        _newFrameFlag = false;
        return true;
    }
    return false;
}

bool ScaleParser::isConnected() const {
    return (millis() - _lastRxTime < 3000);
}

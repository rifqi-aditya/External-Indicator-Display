#include "protocols/YaohuaA12EParser.h"

YaohuaA12EParser::YaohuaA12EParser() : _lastDecimalPlaces(2) {}

bool YaohuaA12EParser::parse(const String& rawFrame, float& weight) {
    int startIdx = rawFrame.indexOf("ww");
    if (startIdx == -1) startIdx = rawFrame.indexOf("wn");

    int endIdx = rawFrame.indexOf("kg");
    if (endIdx == -1) endIdx = rawFrame.indexOf("lb");

    if (startIdx != -1 && endIdx != -1 && endIdx > startIdx) {
        String weightRawStr = rawFrame.substring(startIdx + 2, endIdx);
        
        // Bersihkan karakter non-angka kecuali minus (-) & titik desimal (.)
        String cleanNum = "";
        bool hasDecimal = false;
        int decimalPlaces = 0;

        for (size_t i = 0; i < weightRawStr.length(); i++) {
            char c = weightRawStr[i];
            if ((c >= '0' && c <= '9') || c == '-' || c == '.') {
                cleanNum += c;
                if (c == '.') hasDecimal = true;
                else if (hasDecimal && (c >= '0' && c <= '9')) decimalPlaces++;
            }
        }

        if (cleanNum.length() > 0) {
            weight = cleanNum.toFloat();
            _lastDecimalPlaces = hasDecimal ? decimalPlaces : 0;
            return true;
        }
    }
    return false;
}

String YaohuaA12EParser::formatWeight(float weight) const {
    char formatted[16];
    snprintf(formatted, sizeof(formatted), "%.0f KG", weight);
    return String(formatted);
}

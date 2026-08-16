#ifndef YAOHUAA12EPARSER_H
#define YAOHUAA12EPARSER_H

#include "IProtocolParser.h"

/**
 * @brief Parser Khusus Indikator Yaohua XK3190-A12 / A12E
 * Format Net Weight   : wn-003.17kg  atau  wn0003.17kg
 * Format Gross Weight : ww-003.17kg  atau  ww0003.17kg
 * Single Responsibility: Hanya bertugas melakukan ekstraksi & validasi protokol Yaohua A12E.
 */
class YaohuaA12EParser : public IProtocolParser {
public:
    YaohuaA12EParser();
    bool parse(const String& rawFrame, float& weight) override;
    String formatWeight(float weight) const override;
    String getProtocolName() const override { return "Yaohua XK3190-A12E"; }

private:
    mutable int _lastDecimalPlaces;
};

#endif // YAOHUAA12EPARSER_H

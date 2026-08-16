#ifndef IPROTOCOLPARSER_H
#define IPROTOCOLPARSER_H

#include <Arduino.h>

/**
 * @brief Abstract Interface untuk Protocol Parser Timbangan
 * Menerapkan Open/Closed Principle (OCP) agar protokol baru bisa ditambahkan tanpa mengubah Hardware Layer.
 */
class IProtocolParser {
public:
    virtual ~IProtocolParser() {}

    /**
     * @brief Parse string raw dari indikator menjadi nilai numerik berat
     * @param rawFrame String mentah yang diterima dari Serial
     * @param weight Output variabel float hasil parsing
     * @return true jika parsing berhasil, false jika format tidak cocok
     */
    virtual bool parse(const String& rawFrame, float& weight) = 0;

    /**
     * @brief Format nilai numerik berat menjadi string tampilan (misal: "-3.17 KG")
     */
    virtual String formatWeight(float weight) const = 0;

    /**
     * @brief Mendapatkan nama protokol
     */
    virtual String getProtocolName() const = 0;
};

#endif // IPROTOCOLPARSER_H

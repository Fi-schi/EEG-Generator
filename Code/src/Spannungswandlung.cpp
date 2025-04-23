#include "./Spannungswandlung.hpp"
#include "Global_Var.hpp"
#include "PinMapping.hpp"

void ausgabe(uint8_t Channel, uint8_t Data) {
    // Setze die Datenleitungen auf den gewünschten Wert
    digitalWrite(DB0, (Data >> 0) & 0x01);
    digitalWrite(DB1, (Data >> 1) & 0x01);
    digitalWrite(DB2, (Data >> 2) & 0x01);
    digitalWrite(DB3, (Data >> 3) & 0x01);
    digitalWrite(DB4, (Data >> 4) & 0x01);
    digitalWrite(DB5, (Data >> 5) & 0x01);
    digitalWrite(DB6, (Data >> 6) & 0x01);
    digitalWrite(DB7, (Data >> 7) & 0x01);

    // Setze die Adressleitungen auf den gewünschten Kanal
    digitalWrite(ADD0, (Channel >> 0) & 0x01);
    digitalWrite(ADD1, (Channel >> 1) & 0x01);
    digitalWrite(ADD2, (Channel >> 2) & 0x01);
    digitalWrite(ADD3, (Channel >> 3) & 0x01);

    // Steuerleitungen aktivieren
    digitalWrite(RST, LOW); // Reset aktivieren
    delayMicroseconds(10);
    digitalWrite(RST, HIGH); // Reset deaktivieren

    // Daten laden
    digitalWrite(Load_Data, HIGH);
    delayMicroseconds(10);
    digitalWrite(Load_Data, LOW);

    // Ausgabe aktivieren
    digitalWrite(P_AV, HIGH);
    delayMicroseconds(10);
}

void abspielen() {
    Serial.println("Starte Abspielen der Daten...");

    constexpr float minEEG = -100.0f; // mV
    constexpr float maxEEG = 100.0f;  // mV

    // Maximale Länge aller Kanäle bestimmen
    size_t maxLength = 0;
    for (const auto& [_, data] : kanalDaten) {
        if (data.size() > maxLength) {
            maxLength = data.size();
        }
    }

    for (size_t i = 0; i < maxLength; ++i) {
        for (const auto& [channel, data] : kanalDaten) {
            // Wenn Index gültig, echten Wert verwenden, sonst 0 mV
            float value = (i < data.size()) ? data[i] : 0.0f;

            float normalized = (value - minEEG) / (maxEEG - minEEG);
            normalized = constrain(normalized, 0.0f, 1.0f);
            uint8_t intValue = static_cast<uint8_t>(normalized * 255.0f);

            Serial.print("Kanal ");
            Serial.print(channel);
            Serial.print(" → ");
            Serial.print(value, 2);
            Serial.print(" mV → DAC-Wert: ");
            Serial.println(intValue);

            ausgabe(channel.toInt(), intValue);
        }
        delay(10); // Taktrate
    }

    Serial.println("Abspielen der Daten abgeschlossen.");
}

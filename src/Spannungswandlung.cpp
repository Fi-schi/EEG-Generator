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
    delayMicroseconds(10); // Kurze Pause für Reset
    digitalWrite(RST, HIGH); // Reset deaktivieren

    // Daten laden
    digitalWrite(Load_Data, HIGH); // Daten laden aktivieren
    delayMicroseconds(10); // Kurze Pause für Laden der Daten
    digitalWrite(Load_Data, LOW); // Daten laden deaktivieren

    // Ausgabe aktivieren
    digitalWrite(P_AV, HIGH); // Ausgabe aktivieren
    delayMicroseconds(10); // Kurze Pause für Laden der Daten

    
}
  

void abspielen() {
    Serial.println("Starte Abspielen der Daten...");
  
    for (const auto& [channel, data] : kanalDaten) {
        Serial.print("Kanal: ");
        Serial.print(channel.c_str());
        Serial.print(" - Daten: ");
        for (const auto& value : data) {
            Serial.print(value, 2);
            Serial.print(" ");
        }
        Serial.println();
        
        // Hier wird die Funktion zum Ausgeben der Daten aufgerufen
        ausgabe(channel.toInt(), static_cast<uint8_t>(data[0]));
    }
}
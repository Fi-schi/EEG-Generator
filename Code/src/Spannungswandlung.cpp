#include "./Spannungswandlung.hpp"
#include "Global_Var.hpp"
#include "PinMapping.hpp"
#include <Arduino.h>

void ausgabe(char Channel, uint16_t Data) {
    Data &= 0x0FFF; // Nur untere 12 Bit zulassen
  
    digitalWrite(R_W, LOW);        // Schreiben aktivieren
    digitalWrite(Load_Data, LOW);  // LDAC aktivieren
  
  
    // Adressleitungen A0, A1 setzen → DAC A–D
    switch(Channel) {
        case 'A':
            digitalWrite(ADD0, LOW);     // A0
            digitalWrite(ADD1, LOW);    // A1
            break;
        case 'B':
            digitalWrite(ADD0, LOW);    // A0
            digitalWrite(ADD1, HIGH);     // A1
            break;
        case 'C':
            digitalWrite(ADD0, HIGH);    // A0
            digitalWrite(ADD1, LOW);     // A1
            break;
        case 'D':
            digitalWrite(ADD0, HIGH);     // A0
            digitalWrite(ADD1, HIGH);    // A1
            break;
        default:
            Serial.println("Ungültiger Kanal!"); // Ungültiger Kanal
            return; // Funktion beenden
    }
  
    // Datenleitungen setzen (12 Bit)
    digitalWrite(DB0,  (Data >> 0)  & 0x01);
    digitalWrite(DB1,  (Data >> 1)  & 0x01);
    digitalWrite(DB2,  (Data >> 2)  & 0x01);
    digitalWrite(DB3,  (Data >> 3)  & 0x01);
    digitalWrite(DB4,  (Data >> 4)  & 0x01);
    digitalWrite(DB5,  (Data >> 5)  & 0x01);
    digitalWrite(DB6,  (Data >> 6)  & 0x01);
    digitalWrite(DB7,  (Data >> 7)  & 0x01);
    digitalWrite(DB8,  (Data >> 8)  & 0x01);
    digitalWrite(DB9,  (Data >> 9)  & 0x01);
    digitalWrite(DB10, (Data >> 10) & 0x01);
    digitalWrite(DB11, (Data >> 11) & 0x01);
  
    // Schreiben vorbereiten
    digitalWrite(CS, LOW);         // Chip aktivieren
    delayMicroseconds(1);          // Setup-Zeit (tWS ≥ 0 ns)
    digitalWrite(CS, HIGH);       // Lesen deaktivieren (Schreiben aktivieren)
   
  
    // Daten übernehmen (Load)
  
    //delayMicroseconds(1);        // tLDW ≥ 170 ns laut Datenblatt
   // digitalWrite(Load_Data, HIGH); // LDAC deaktivieren
  
    // Schreiben beenden
   // digitalWrite(R_W, HIGH);       // Schreiben beenden (nicht zwingend nötig, aber klar)
    //digitalWrite(CS, HIGH);        // Chip deaktivieren
  
  }

// FreeRTOS Task Handle
TaskHandle_t abspielTaskHandle = nullptr;
extern int ausgabeFrequenzHz;

// Task-Funktion
void abspielTask(void* parameter) {
    Serial.println("Starte Abspielen der Daten (FreeRTOS Task)...");
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

            ausgabe(channel[0], intValue);
            yield();
        }
        vTaskDelay(pdMS_TO_TICKS(1000 / ausgabeFrequenzHz)); // Frequenz dynamisch
    }

    Serial.println("Abspielen der Daten abgeschlossen.");
    abspielTaskHandle = nullptr;
    vTaskDelete(nullptr); // Task selbst löschen
}

// Startfunktion für den Task
void startAbspielTask() {
    if (abspielTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(
            abspielTask,         // Task-Funktion
            "AbspielTask",       // Name
            4096,                // Stack-Größe
            nullptr,             // Parameter
            1,                   // Priorität
            &abspielTaskHandle,  // Handle
            1                    // Core (1 = App Core auf ESP32)
        );
    } else {
        Serial.println("Abspiel-Task läuft bereits!");
    }
}

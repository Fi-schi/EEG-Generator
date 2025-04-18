#include <Arduino.h>
#include "PinMapping.hpp"

void initPinModes() {
  // Datenleitungen
  pinMode(DB0, OUTPUT);
  pinMode(DB1, OUTPUT);
  pinMode(DB2, OUTPUT);
  pinMode(DB3, OUTPUT);
  pinMode(DB4, OUTPUT);
  pinMode(DB5, OUTPUT);
  pinMode(DB6, OUTPUT);
  pinMode(DB7, OUTPUT);

  // Adressleitungen
  pinMode(ADD0, OUTPUT);
  pinMode(ADD1, OUTPUT);
  pinMode(ADD2, OUTPUT);
  pinMode(ADD3, OUTPUT);

  // Steuerleitungen
  pinMode(RST, OUTPUT);
  pinMode(Load_Data, OUTPUT);
  pinMode(P_AV, OUTPUT);
  pinMode(P_RD, OUTPUT);
}

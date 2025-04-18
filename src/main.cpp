#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <DNSServer.h>
#include <vector>
#include <map>
#include <ArduinoJson.h>
#include <regex>
#include <cmath>
#include "Global_Var.hpp"
#include "Server.hpp"
#include "PinMapping.hpp"
#include "Spannungswandlung.hpp"

const char* ssid = "EEG-Simulator";
const char* password = "123456789";

AsyncWebServer server(80);
DNSServer dnsServer;

std::map<String, std::vector<float>> kanalDaten;
std::vector<FileData> uploadedFiles;
std::map<AsyncWebServerRequest*, ActiveUpload> activeUploads;



void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 startet...");
  initPinModes();
  setupWiFi();
  setupFileSystem();
  dnsServer.start(53, "*", WiFi.softAPIP());
  setupWebServer();
  Serial.println("Setup abgeschlossen. ESP32 bereit.");
}

void loop() {
  dnsServer.processNextRequest();
}

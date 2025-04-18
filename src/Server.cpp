#include "Server.hpp"

void setupWiFi() {
    WiFi.softAP(ssid, password);
    Serial.println("WiFi gestartet");
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.softAPIP());
  }
  
  void setupFileSystem() {
    if (!SPIFFS.begin(true)) {
      Serial.println("Fehler: SPIFFS konnte nicht gestartet werden!");
      return;
    }
    Serial.println("SPIFFS erfolgreich gestartet.");
  }
  
  std::vector<float> extractNumbersRegex(const String &content) {
    std::vector<float> numbers;
    std::string s = content.c_str();
    std::regex floatRegex("\\b-?[0-9]+(?:\\.[0-9]+)?\\b");
    auto numbersBegin = std::sregex_iterator(s.begin(), s.end(), floatRegex);
    auto numbersEnd = std::sregex_iterator();
    for (std::sregex_iterator i = numbersBegin; i != numbersEnd; ++i) {
      std::smatch match = *i;
      try {
        float value = std::stof(match.str());
        numbers.push_back(value);
      } catch (...) {}
    }
    return numbers;
  }
  
  String generateUniqueFileName(const String& baseName) {
    String uniqueName = baseName;
    int counter = 1;
    while (SPIFFS.exists("/" + uniqueName)) {
      uniqueName = baseName + "(" + String(counter++) + ")";
    }
    return uniqueName;
  }
  
  String getUploadedFilesList() {
    String filesList = "{\"files\":[";
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    bool first = true;
    while (file) {
      if (!first) filesList += ",";
      String name = String(file.name());
      if (name.startsWith("/")) name = name.substring(1);
      filesList += "\"" + name + "\"";
      first = false;
      file = root.openNextFile();
    }
    filesList += "]}";
    return filesList;
  }
  
  void setupWebServer() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (SPIFFS.exists("/HTML_Server.html")) {
        request->send(SPIFFS, "/HTML_Server.html", "text/html");
      } else {
        request->send(404, "text/plain", "Fehler: HTML_Server.html nicht gefunden.");
      }
    });
  
    server.onNotFound([](AsyncWebServerRequest *request) {
      request->redirect("/");
    });
  
    server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(204, "text/html", "");
    });
  
    server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->redirect("/");
    });
  
    server.on("/HS-Wismar_Logo-FIW_V1_RGB.png", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (SPIFFS.exists("/HS-Wismar_Logo-FIW_V1_RGB.png")) {
        request->send(SPIFFS, "/HS-Wismar_Logo-FIW_V1_RGB.png", "image/png");
      } else {
        request->send(404, "text/plain", "Fehler: Logo nicht gefunden.");
      }
    });
  
    server.on("/storage", HTTP_GET, [](AsyncWebServerRequest *request) {
      size_t totalBytes = SPIFFS.totalBytes();
      size_t usedBytes = SPIFFS.usedBytes();
      String json = "{";
      json += "\"total\": " + String(totalBytes) + ",";
      json += "\"used\": " + String(usedBytes);
      json += "}";
      request->send(200, "application/json", json);
    });
  
    server.on("/upload", HTTP_POST,
      [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Upload erfolgreich");
      },
      [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final) {
        if (index == 0) {
          String path = "/" + generateUniqueFileName(filename);
          File file = SPIFFS.open(path, "w");
          if (!file) {
            Serial.println("Fehler beim Öffnen der Datei: " + path);
            return;
          }
          ActiveUpload au;
          au.file = file;
          au.path = path;
          activeUploads[request] = au;
        }
        if (activeUploads.find(request) != activeUploads.end()) {
          activeUploads[request].file.write(data, len);
        }
        if (final) {
          if (activeUploads.find(request) != activeUploads.end()) {
            activeUploads[request].file.close();
            FileData fd;
            fd.filename = activeUploads[request].path;
            uploadedFiles.push_back(fd);
            activeUploads.erase(request);
            Serial.println("Datei gespeichert: " + fd.filename);
          }
        }
      }
    );
  
    server.on("/getFiles", HTTP_GET, [](AsyncWebServerRequest *request) {
      String filesList = getUploadedFilesList();
      request->send(200, "application/json", filesList);
    });
  
    server.on("/delete", HTTP_DELETE, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("name")) {
        request->send(400, "text/plain", "Fehler: Kein Dateiname angegeben.");
        return;
      }
      String fileName = "/" + request->getParam("name")->value();
      if (SPIFFS.exists(fileName)) {
        SPIFFS.remove(fileName);
        request->send(200, "text/plain", "Datei erfolgreich gelöscht.");
      } else {
        request->send(404, "text/plain", "Datei nicht gefunden.");
      }
    });
  
    server.on("/processFiles", HTTP_POST, [](AsyncWebServerRequest *request) {
      String channelsJson = "";
      if (request->hasParam("channels", true)) {
        channelsJson = request->getParam("channels", true)->value();
      }
  
      StaticJsonDocument<2048> resultDoc;
      JsonArray results = resultDoc["results"].to<JsonArray>();
  
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, channelsJson);
      if (error) {
        request->send(400, "text/plain", "Fehler beim Parsen des channels JSON.");
        return;
      }
  
      JsonArray channelsArray = doc.as<JsonArray>();
      for (JsonObject elem : channelsArray) {
        const char* name = elem["name"];
        const char* channel = elem["channel"];
        String filePath = "/" + String(name);
  
        JsonObject res = results.add<JsonObject>();
        res["filename"] = String(name);
        res["channel"] = channel;
  
        if (SPIFFS.exists(filePath)) {
          File file = SPIFFS.open(filePath, "r");
          if (file) {
            String content;
            while (file.available()) content += (char)file.read();
            file.close();
  
            std::vector<float> numbers = extractNumbersRegex(content);
            kanalDaten[String(channel)] = numbers;
  
            if (content.length() == 0 || numbers.empty()) {
              res["selfCheck"] = "Keine Zahlen gefunden. Datei übersprungen.";
              res["error"] = "Keine Zahlen erkannt";
              continue;
            }
  
            JsonArray nums = res["numbers"].to<JsonArray>();
            for (float n : numbers) nums.add(n);
  
            res["numberCount"] = (int)numbers.size();
            res["selfCheck"] = "OK";
          } else {
            res["error"] = "Fehler beim Öffnen der Datei.";
            res["selfCheck"] = "Fehler";
          }
        } else {
          res["error"] = "Datei nicht gefunden.";
          res["selfCheck"] = "Fehler";
        }
      }
  
      String response;
      serializeJson(resultDoc, response);
      request->send(200, "application/json", response);
    });
    server.on("/play", HTTP_POST, [](AsyncWebServerRequest *request) {
      abspielen();
      request->send(200, "text/plain", "Wiedergabe gestartet");
    });
    server.serveStatic("/script.js", SPIFFS, "/script.js");
  
    server.begin();
    Serial.println("Webserver gestartet.");
  }
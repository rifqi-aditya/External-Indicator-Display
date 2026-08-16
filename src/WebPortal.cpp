#include "WebPortal.h"

WebPortal::WebPortal(ConfigManager &configMgr, ScaleParser &scaleParser)
    : _configMgr(configMgr), _scaleParser(scaleParser), _server(80), _configUpdated(false), _fsReady(false) {}

void WebPortal::begin(AppConfig &currentConfig) {
    _config = currentConfig;

    // 1. Mount LittleFS Filesystem
    if (LittleFS.begin(true)) {
        _fsReady = true;
        Serial.println("[FS] LittleFS Mounted Successfully!");
    } else {
        Serial.println("[FS] LittleFS Mount Failed!");
    }

    // 2. Inisialisasi Wi-Fi AP via WiFiAPManager
    _wifiManager.begin("TIMBANGAN-P10-SETUP", "12345678");
    Serial.printf("[WEB] Wi-Fi AP: TIMBANGAN-P10-SETUP (IP: %s)\n", _wifiManager.getIPAddress().c_str());

    // 3. Routing WebServer Endpoints
    _server.on("/", HTTP_GET, std::bind(&WebPortal::handleRoot, this));
    _server.on("/save", HTTP_POST, std::bind(&WebPortal::handleSave, this));
    _server.on("/api/weight", HTTP_GET, std::bind(&WebPortal::handleApiWeight, this));
    _server.on("/api/config", HTTP_GET, std::bind(&WebPortal::handleApiConfig, this));

    // Handle Static Files (/style.css, /app.js, dll)
    _server.onNotFound([this]() {
        if (!handleFileRead(_server.uri())) {
            _server.send(404, "text/plain", "404: Not Found");
        }
    });

    _server.begin();
}

void WebPortal::update() {
    _server.handleClient();
}

void WebPortal::handleRoot() {
    if (_fsReady && LittleFS.exists("/index.html")) {
        File file = LittleFS.open("/index.html", "r");
        _server.streamFile(file, "text/html");
        file.close();
    } else {
        // Fallback jika LittleFS belum diupload
        _server.send(200, "text/html", "<h1>LittleFS Flash Data Required</h1><p>Upload folder data menggunakan <b>pio run -t uploadfs</b></p>");
    }
}

bool WebPortal::handleFileRead(String path) {
    if (path.endsWith("/")) path += "index.html";
    
    String contentType = "text/plain";
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".ico")) contentType = "image/x-icon";
    else if (path.endsWith(".svg")) contentType = "image/svg+xml";

    if (_fsReady && LittleFS.exists(path)) {
        File file = LittleFS.open(path, "r");
        _server.streamFile(file, contentType);
        file.close();
        return true;
    }
    return false;
}

void WebPortal::handleSave() {
    if (_server.hasArg("mode")) {
        _config.displayMode = _server.arg("mode").toInt();
    }
    if (_server.hasArg("stext")) {
        _config.staticText = _server.arg("stext");
    }
    if (_server.hasArg("rtext")) {
        _config.runningText = _server.arg("rtext");
    }
    if (_server.hasArg("baud")) {
        _config.baudRate = _server.arg("baud").toInt();
    }

    _configMgr.saveConfig(_config);
    _scaleParser.setBaudRate(_config.baudRate);
    _configUpdated = true;

    _server.send(200, "application/json", "{\"status\":\"ok\"}");
}

void WebPortal::handleApiWeight() {
    String json = "{\"weight\":\"" + _scaleParser.getWeightString() + 
                  "\",\"raw\":\"" + _scaleParser.getLastRawData() + 
                  "\",\"connected\":" + String(_scaleParser.isConnected() ? "true" : "false") + "}";
    _server.send(200, "application/json", json);
}

void WebPortal::handleApiConfig() {
    String json = "{\"mode\":" + String(_config.displayMode) + 
                  ",\"stext\":\"" + _config.staticText + 
                  "\",\"rtext\":\"" + _config.runningText + 
                  "\",\"baud\":" + String(_config.baudRate) + "}";
    _server.send(200, "application/json", json);
}

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SD.h>
#include <SPI.h>

#include <WebSocketsServer.h>

WebSocketsServer webSocket = WebSocketsServer(81);



#define SD_CS 5

const char* ssid = "Savinu";
const char* password = "slom5217";

WebServer server(80);

// 📁 Generate HTML with file list and send buttons
String generateFileList(const char* dirname = "/") {
    String html = "<ul>";
    File root = SD.open(dirname);
    if (!root || !root.isDirectory()) {
        return "<p>Failed to open SD directory</p>";
    }

    File file = root.openNextFile();
    while (file) {
        String filename = String(file.name());
        html += "<li>";
        html += "<a href=\"/files" + filename + "\">" + filename + "</a> ";
        html += "(" + String(file.size()) + " bytes) ";
        html += "<a href=\"/send?file=" + filename + "\"><button>Send to Serial</button></a>";
        html += "</li>";
        file = root.openNextFile();
    }
    html += "</ul>";
    return html;
}

// 🌐 Upload + file list page
void handleRoot() {
    String html = R"rawliteral(
        <!DOCTYPE html>
        <html>
        <head>
            <title>ESP32 G-code Manager</title>
            <script>
                let socket = new WebSocket("ws://" + location.hostname + ":81/");
                socket.onmessage = function(event) {
                    const log = document.getElementById("log");
                    log.innerText += event.data + "\\n";
                    log.scrollTop = log.scrollHeight;
                };
            </script>
        </head>
        <body>
            <h2>Upload G-code File</h2>
            <form method='POST' action='/upload' enctype='multipart/form-data'>
                <input type='file' name='file'>
                <input type='submit' value='Upload'>
            </form>
            <h3>Files on SD Card</h3>
    )rawliteral";

    html += generateFileList();

    html += R"rawliteral(
            <h3>Serial Log</h3>
            <pre id="log" style="height: 300px; overflow-y: scroll; background: #f0f0f0; padding: 10px;"></pre>
        </body>
        </html>
    )rawliteral";

    server.send(200, "text/html", html);
}


// 📤 Handle file upload
void handleUpload() {
    HTTPUpload& upload = server.upload();
    static File uploadFile;

    if (upload.status == UPLOAD_FILE_START) {
        String filename = "/" + upload.filename;
        uploadFile = SD.open(filename, FILE_WRITE);
        Serial.printf("Uploading: %s\n", filename.c_str());
    } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (uploadFile) uploadFile.write(upload.buf, upload.currentSize);
    } else if (upload.status == UPLOAD_FILE_END) {
        if (uploadFile) uploadFile.close();
        Serial.println("Upload complete.");
    }
}

// 🧾 Serve static file
void handleFileDownload() {
    String path = server.uri();  // e.g., /files/filename
    path.remove(0, 6);           // Remove "/files"
    String fullPath = "/" + path;

    if (SD.exists(fullPath)) {
        File file = SD.open(fullPath, FILE_READ);
        server.streamFile(file, "application/octet-stream");
        file.close();
    } else {
        server.send(404, "text/plain", "File not found");
    }
}

// 🚀 Send file line by line to Serial
// 🚀 Send G-code file line by line with 'ok' handshake and skip comments
void handleSendToSerial() {
    if (!server.hasArg("file")) {
        server.send(400, "text/plain", "Missing file name");
        return;
    }

    String filename = "/" + server.arg("file");
    if (!SD.exists(filename)) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    File file = SD.open(filename);
    if (!file || file.isDirectory()) {
        server.send(500, "text/plain", "Failed to open file");
        return;
    }

    Serial.printf("\n--- Sending file: %s ---\n", filename.c_str());
    webSocket.broadcastTXT("📤 Sending file: " + filename);

    while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    int commentIndex = line.indexOf(';');
        if (commentIndex != -1) {
            line = line.substring(0, commentIndex);
            line.trim();
        }

        if (line.length() > 0) {
            Serial.println(line);
            webSocket.broadcastTXT("➡️ Sent: " + line);

            // 🕓 Wait for 'ok' from printer
            unsigned long startTime = millis();
            String response = "";

            while (millis() - startTime < 30000) {   // allow longer (30s) because some moves take time
                while (Serial.available()) {
                    char c = Serial.read();
                    response += c;

                    // split lines
                    if (c == '\n' || c == '\r') {
                        response.trim();

                        if (response.length() > 0) {
                            if (response.startsWith("ok")) {
                                webSocket.broadcastTXT("✅ Received: " + response + "\n");
                                goto nextLine;   // proceed to next G-code
                            }
                            else if (response.startsWith("echo:busy")) {
                                // keep waiting, but notify web client
                                webSocket.broadcastTXT("⏳ Printer busy: " + response);
                                // reset timer so it doesn’t timeout while still busy
                                startTime = millis();
                            }
                            else if (response.startsWith("echo:")) {
                                // informational message
                                webSocket.broadcastTXT("💬 " + response);
                            }
                            else {
                                // any other message (errors, temps, etc.)
                                webSocket.broadcastTXT("📩 " + response);
                            }
                        }
                        response = ""; // reset buffer for next line
                    }
                }
            }

            webSocket.broadcastTXT("⚠️ Timeout waiting for 'ok' response.");
            break;

            nextLine:
            continue;
        }
    }


    file.close();
    server.send(200, "text/plain", "✅ File sent to Serial");
}


void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Connected! IP address: ");
    Serial.println(WiFi.localIP());

    if (!SD.begin(SD_CS)) {
        Serial.println("SD Card Mount Failed");
        return;
    }

    server.on("/", HTTP_GET, handleRoot);
    server.on("/upload", HTTP_POST, []() {
        server.sendHeader("Location", "/");
        server.send(303);
    }, handleUpload);
    server.on("/files/", HTTP_GET, handleFileDownload);
    server.on("/send", HTTP_GET, handleSendToSerial);

    server.begin();
    Serial.println("HTTP server started.");

    webSocket.begin();
    webSocket.onEvent([](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
    // You can handle incoming messages here if needed
    });

}

void loop() {
    server.handleClient();
    webSocket.loop();

}

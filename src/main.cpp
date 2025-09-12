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
    String html = "<ul style='padding:0;'>";
    File root = SD.open(dirname);
    if (!root || !root.isDirectory()) {
        return "<p>Failed to open SD directory</p>";
    }

    File file = root.openNextFile();
    while (file) {
        String filename = String(file.name());

        html += "<li style='"
                "display:flex; "
                "justify-content:space-between; "
                "align-items:center; "
                "padding:12px; "
                "margin:8px 0; "
                "background:white; "
                "border-radius:8px; "
                "box-shadow:0 2px 6px rgba(0,0,0,0.08);'>";

        // Left side: file name + size
        html += "<div style='flex:1;'>";
        html += "<a href=\"/files" + filename + "\" style='font-weight:bold; color:#0073e6; text-decoration:none;'>" + filename + "</a>";
        float kb = file.size() / 1024.0;
        html += " <span style='color:#666; font-size:14px;'>(" + String(kb, 2) + " KB)</span>";
        html += "</div>";

        // Right side: buttons
        html += "<div style='flex-shrink:0; display:flex; gap:10px;'>";
        html += "<a href=\"/send?file=" + filename + "\"><button style='background:#28a745; color:white; border:none; padding:6px 12px; border-radius:6px; cursor:pointer;'>Send</button></a>";
        html += "<a href=\"/delete?file=" + filename + "\" onclick=\"return confirm('Delete " + filename + "?')\"><button style='background:#dc3545; color:white; border:none; padding:6px 12px; border-radius:6px; cursor:pointer;'>Delete</button></a>";
        html += "</div>";

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
        <html lang="en">
        <head>
            <meta charset="UTF-8">
            <title>ESP32 G-code Manager</title>
            <style>
                body {
                    font-family: Arial, sans-serif;
                    margin: 0;
                    padding: 0;
                    background: #f7f9fc;
                    color: #333;
                }
                header {
                    background: #0073e6;
                    color: white;
                    padding: 20px;
                    text-align: center;
                    box-shadow: 0 2px 8px rgba(0,0,0,0.2);
                }
                h2, h3 {
                    color: #0073e6;
                }
                main {
                    padding: 20px;
                    max-width: 900px;
                    margin: auto;
                }
                form {
                    margin-bottom: 20px;
                    padding: 15px;
                    background: white;
                    border-radius: 8px;
                    box-shadow: 0 2px 6px rgba(0,0,0,0.1);
                }
                input[type="file"] {
                    padding: 6px;
                    border: 1px solid #ccc;
                    border-radius: 6px;
                }
                button.uploadBtn {
                    background: #0073e6;
                    color: white;
                    border: none;
                    padding: 8px 14px;
                    border-radius: 6px;
                    cursor: pointer;
                    margin-left: 10px;
                }
                button.uploadBtn:hover {
                    background: #005bb5;
                }
                .progress-container {
                    width: 100%;
                    background: #e0e0e0;
                    border-radius: 8px;
                    margin-top: 10px;
                    height: 20px;
                    overflow: hidden;
                }
                .progress-bar {
                    height: 100%;
                    width: 0%;
                    background: #28a745;
                    text-align: center;
                    color: white;
                    font-size: 12px;
                    line-height: 20px;
                    transition: width 0.2s;
                }
                ul {
                    list-style: none;
                    padding: 0;
                }
                li {
                    background: white;
                    margin: 8px 0;
                    padding: 12px;
                    border-radius: 8px;
                    display: flex;
                    justify-content: space-between;
                    align-items: center;
                    box-shadow: 0 2px 6px rgba(0,0,0,0.08);
                }
                li a {
                    text-decoration: none;
                    color: #0073e6;
                    font-weight: bold;
                }
                li button {
                    background: #28a745;
                    color: white;
                    border: none;
                    padding: 6px 12px;
                    border-radius: 6px;
                    cursor: pointer;
                }
                li button:hover {
                    background: #218838;
                }
                #log {
                    height: 300px;
                    overflow-y: scroll;
                    background: black;
                    color: #0f0;
                    padding: 12px;
                    font-family: monospace;
                    border-radius: 8px;
                    box-shadow: inset 0 0 6px rgba(0,0,0,0.5);
                }
                footer {
                    text-align: center;
                    padding: 15px;
                    font-size: 14px;
                    color: #666;
                }
            </style>
            <script>
                let socket = new WebSocket("ws://" + location.hostname + ":81/");
                socket.onmessage = function(event) {
                    const log = document.getElementById("log");
                    log.innerText += event.data + "\\n";
                    log.scrollTop = log.scrollHeight;
                };

                function uploadFile(event) {
                    event.preventDefault();
                    const fileInput = document.querySelector("input[name='file']");
                    if (!fileInput.files.length) {
                        alert("Please select a file first!");
                        return;
                    }

                    const file = fileInput.files[0];
                    const formData = new FormData();
                    formData.append("file", file);

                    const xhr = new XMLHttpRequest();
                    xhr.open("POST", "/upload", true);

                    xhr.upload.onprogress = function(e) {
                        if (e.lengthComputable) {
                            const percent = Math.round((e.loaded / e.total) * 100);
                            const bar = document.getElementById("progress-bar");
                            bar.style.width = percent + "%";
                            bar.textContent = percent + "%";
                        }
                    };

                    xhr.onload = function() {
                        if (xhr.status == 200 || xhr.status == 303) {
                            alert("Upload complete!");
                            location.reload(); // refresh to show new file
                        } else {
                            alert("Upload failed!");
                        }
                    };

                    xhr.send(formData);
                }
            </script>
        </head>
        <body>
            <header>
                <h1>⚙️ ESP32 G-code Manager</h1>
                <p>Manage, upload, and send G-code files to your printer</p>
            </header>
            <main>
                <h2>📤 Upload G-code File</h2>
                <form onsubmit="uploadFile(event)">
                    <input type='file' name='file' required>
                    <button class="uploadBtn" type="submit">Upload</button>
                    <div class="progress-container">
                        <div id="progress-bar" class="progress-bar">0%</div>
                    </div>
                </form>

                

                <h3>📁 Files on SD Card</h3>
    )rawliteral";

    html += generateFileList();

    html += R"rawliteral(
                <h3>🖨️ Serial Log</h3>
                <pre id="log"></pre>
            </main>
            <footer>
                <p>Drop to print © 2025</p>
            </footer>
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

// 🗑️ Delete file
void handleDeleteFile() {
    if (!server.hasArg("file")) {
        server.send(400, "text/plain", "Missing file name");
        return;
    }

    String filename = "/" + server.arg("file");
    if (!SD.exists(filename)) {
        server.send(404, "text/plain", "File not found");
        return;
    }

    if (SD.remove(filename)) {
        Serial.printf("Deleted file: %s\n", filename.c_str());
        server.sendHeader("Location", "/"); // refresh UI
        server.send(303);
    } else {
        server.send(500, "text/plain", "Failed to delete file");
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
    server.on("/delete", HTTP_GET, handleDeleteFile);

    server.on("/upload", HTTP_POST, []() {
    server.send(200, "text/plain", "Upload complete");
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

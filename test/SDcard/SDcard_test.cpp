#include <Arduino.h>
#include <unity.h>

#include <FS.h>
#include <SD.h>o
#include <SPI.h>

#define SD_CS 5  // Chip Select pin (change if needed)

void listDir(fs::FS &fs, const char *dirname, uint8_t levels) {
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if (!root || !root.isDirectory()) {
        Serial.printf("Failed to open directory: %s\n", dirname);
        return;
    }

    File file = root.openNextFile();
    while (file) {
        if (file.isDirectory()) {
            // Construct absolute path for subdirectory
            String path = String(dirname);
            if (!path.endsWith("/")) path += "/";
            path += file.name();

            Serial.print("DIR : ");
            Serial.println(path);

            if (levels > 0) {
                listDir(fs, path.c_str(), levels - 1);
            }
        } else {
            Serial.print("FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Initializing SD card...");

    if (!SD.begin(SD_CS)) {
        Serial.println("❌ SD card mount failed!");
        return;
    }

    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        Serial.println("❌ No SD card attached.");
        return;
    }

    Serial.print("✅ SD Card Type: ");
    switch (cardType) {
        case CARD_MMC:  Serial.println("MMC"); break;
        case CARD_SD:   Serial.println("SDSC"); break;
        case CARD_SDHC: Serial.println("SDHC"); break;
        default:        Serial.println("UNKNOWN"); break;
    }

    Serial.printf("📦 Card Size: %llu MB\n", SD.cardSize() / (1024 * 1024));
    listDir(SD, "/", 2);  // List root and 1 level of subdirectories
}

void loop() {
    // Nothing here
}

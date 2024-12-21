#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal.h>
#include <SD.h>

#define SS_PIN 10
#define RST_PIN 9
#define RELAY_PIN 4
#define BUZZER_PIN 3
#define ADD_USER_BUTTON_PIN 2
#define REMOVE_USER_BUTTON_PIN A0

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
LiquidCrystal lcd(7, 8, 5, 6, 4, 3); // Initialize LCD
File logFile;

const int maxUsers = 10; // Maximum number of authorized users
String authorizedUIDs[maxUsers]; // Array to store authorized UIDs
int userCount = 0; // Current number of authorized users

void setup() {
    Serial.begin(9600); // Initialize serial communications
    SPI.begin();        // Init SPI bus
    mfrc522.PCD_Init(); // Init MFRC522
    pinMode(RELAY_PIN, OUTPUT); // Set relay pin as output
    pinMode(BUZZER_PIN, OUTPUT); // Set buzzer pin as output
    pinMode(ADD_USER_BUTTON_PIN, INPUT_PULLUP); // Set button pin as input
    pinMode(REMOVE_USER_BUTTON_PIN, INPUT_PULLUP); // Set button pin as input
    digitalWrite(RELAY_PIN, LOW); // Ensure relay is off initially

    lcd.begin(16, 2); // Initialize LCD
    lcd.print("RFID Hair Dryer");
    delay(2000);
    lcd.clear();

    if (!SD.begin(4)) {
        lcd.print("SD Card Error");
        return;
    }
    logFile = SD.open("log.txt", FILE_WRITE);
    if (!logFile) {
        lcd.print("Log File Error");
    }
    Serial.println("Place your RFID card near the reader...");
}

void loop() {
    // Look for new cards
    if (!mfrc522.PICC_IsNewCardPresent()) {
        return;
    }

    // Select one of the cards
    if (!mfrc522.PICC_ReadCardSerial()) {
 return;
    }

    // Show UID on serial monitor
    String uid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += String(mfrc522.uid.uidByte[i], HEX);
        if (i < mfrc522.uid.size - 1) {
            uid += " ";
        }
    }
    uid.toUpperCase(); // Convert to uppercase for comparison
    Serial.print("UID tag: ");
    Serial.println(uid);
    lcd.clear();
    lcd.print("UID: ");
    lcd.println(uid);

    // Check if the UID is authorized
    if (isAuthorized(uid)) {
        Serial.println("Authorized access! Activating hair dryer...");
        lcd.clear();
        lcd.print("Access Granted!");
        digitalWrite(RELAY_PIN, HIGH); // Turn on the relay (hair dryer)
        tone(BUZZER_PIN, 1000, 500); // Buzzer sound for 500ms
        delay(5000); // Keep the relay on for 5 seconds
        digitalWrite(RELAY_PIN, LOW); // Turn off the relay
        Serial.println("Hair dryer deactivated.");
        logAccess(uid, true);
        lcd.clear();
        lcd.print("Hair Dryer Off");
    } else {
        Serial.println("Access denied.");
        logAccess(uid, false);
        lcd.clear();
        lcd.print("Access Denied!");
        tone(BUZZER_PIN, 500, 500); // Buzzer sound for 500ms
    }

    // Check for button presses to add or remove users
    if (digitalRead(ADD_USER_BUTTON_PIN) == LOW) {
        addUser (uid);
    }
    if (digitalRead(REMOVE_USER_BUTTON_PIN) == LOW) {
        removeUser (uid);
    }
}

bool isAuthorized(String uid) {
    for (int i = 0; i < userCount; i++) {
        if (uid.equalsIgnoreCase(authorizedUIDs[i])) {
            return true; // UID is authorized
        }
    }
    return false; // UID is not authorized
}

void addUser (String uid) {
    if (userCount < maxUsers) {
        authorizedUIDs[userCount] = uid;
        userCount++;
        Serial.println("User  added: " + uid);
        lcd.clear();
        lcd.print("User  Added!");
        delay(2000);
    } else {
        Serial.println("User  list full!");
        lcd.clear();
        lcd.print("User  List Full!");
        delay(2000);
    }
}

void removeUser (String uid) {
    for (int i = 0; i < userCount; i++) {
        if (uid.equalsIgnoreCase(authorizedUIDs[i])) {
            authorizedUIDs[i] = authorizedUIDs[userCount - 1]; // Replace with last user
            userCount--;
            Serial.println("User  removed: " + uid);
            lcd.clear();
            lcd.print("User  Removed!");
            delay(2000);
            return;
        }
    }
    Serial.println("User  not found!");
    lcd.clear();
    lcd.print("User  Not Found!");
    delay(2000);
}

void logAccess(String uid, bool accessGranted) {
    if (logFile) {
        logFile.print("UID: ");
        logFile.print(uid);
        logFile.print(" - Access: ");
        logFile.println(accessGranted ? "Granted" : "Denied");
        logFile.flush(); // Ensure data is written to the SD card
    }
}
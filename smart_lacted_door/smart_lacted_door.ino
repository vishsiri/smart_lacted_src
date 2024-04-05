#define BLYNK_TEMPLATE_ID "TMPL6PdSWvWmQ"
#define BLYNK_TEMPLATE_NAME "Dv1"
#define BLYNK_AUTH_TOKEN "9CIfjcRGZRoiF1fRa-Oc-aTotO-0GY8y"
#define BLYNK_PRINT Serial

// #define LINE_TOKEN "dGVz4XvSOqJxEb6GjiFdrJMJTArkFkx21WYCz8BG3H6a"
#include <Adafruit_Fingerprint.h>
#include <ESP32Firebase.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
// #include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>
#include "time.h"
#include "pj_fingerprint.h"
#include "smart_lacted_door.h"
#include "oled_screen.h"
// PROTOCOL SETUP
#define _SSID "vishsiri"                                                                              // Your WiFi SSID
#define _PASSWORD "11111111"                                                                          // Your WiFi Password
#define REFERENCE_URL "https://esp32test-trigger-default-rtdb.asia-southeast1.firebasedatabase.app/"  // Your Firebase project reference url

// VARIABLE SETUP
String inputString = "";            // String to hold the entered keys
String _FIREBASE_DEVICE_PATH = "";  //Set Device Path
String cardUIDs[10] = {
  "12E73A4C",
  "7D3DB5C3",
  "118F3A1D",
  "", "", "", "", "", "", ""
};
// Array to hold card UIDs (assuming a maximum of 10 cards)
String cardUIDText = "";
String TimeString = "";
bool doorStatus = false;
//Time
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

Firebase firebase(REFERENCE_URL);
// SETUP PIN
#define BUZZER_PIN 12
#define RED_BUTTON_PIN 27
#define RELAY_LOCK 14
#define OLED_RESET -1
// #define LED_R 4
#define LED_G 3
#define LED_B 1
#define MENETIC_SWITCH 4

//TASK CORE
TaskHandle_t Core1Task;




// NUMPAD SETUP
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  { '*', '0', '#', 'D' },
  { '7', '8', '6', 'C' },
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' }
};

byte colPins[ROWS] = { 32, 33, 25, 26 };  // Connect to the column pinouts of the keypad
byte rowPins[COLS] = { 36, 39, 34, 35 };  // Connect to the row pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// RFID SETUP
#define SS_PIN 5
#define RST_PIN 0

MFRC522 mfrc522(SS_PIN, RST_PIN);


void core1Task(void* pvParameters) {
  unsigned long previousMillis = 0;
  const unsigned long interval = 10000; // 10 seconds in milliseconds
  bool switchPressed = false;

  while (1) {
    TimeSetting();
    TimeString = getTimeString();
    String pathh, data22;
    unsigned long currentMillis = millis();
    if (digitalRead(MENETIC_SWITCH) == 1) {
      if (!switchPressed) {
        // Switch was just pressed, start the timer
        previousMillis = currentMillis;
        switchPressed = true;
      }
      if (currentMillis - previousMillis >= interval) {
        beepDoorOpen();
                beepDoorOpen();
                        beepDoorOpen();
                                beepDoorOpen();
                                        beepDoorOpen();
                                                beepDoorOpen();
        pathh = _FIREBASE_DEVICE_PATH + "/AllHis/";
        data22 = "DoorNotClose -- | " + TimeString;
        firebase.pushString(_FIREBASE_DEVICE_PATH + "/AllHis/", data22); 
        switchPressed = false; // Reset the switch state
      }
    } else {
      switchPressed = false; // Reset the switch state if it's released
    }

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

//RGB
// OLED SCREEN


// FINGER PRINT MODULE


// RFID MODULE

// BLYNK


void defaultExecuteOnRun() {
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(RELAY_LOCK, LOW);
  SPI.begin();         // Initiate  SPI bus
  mfrc522.PCD_Init();  // Initiate MFRC522
}

void pressButtonOpenDoor() {
  if (digitalRead(RED_BUTTON_PIN) == LOW) {
    digitalWrite(RELAY_LOCK, HIGH);
  } else {
    digitalWrite(RELAY_LOCK, LOW);
  }
}

void LEDColor(int r2, int r3) {
  analogWrite(LED_G, r2);
    vTaskDelay(pdMS_TO_TICKS(200));
  analogWrite(LED_B, r3);
    vTaskDelay(pdMS_TO_TICKS(200));

}

void pinModeSetup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_LOCK, OUTPUT);
  pinMode(MENETIC_SWITCH, INPUT_PULLUP);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void beepConnectingSuccess() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));  // Delay for 100 milliseconds
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));  // Delay for another 100 milliseconds
  }
}

void beepDoorOpen() {
  for (int i = 0; i < 1; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100));
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void beepDoorNotOpen() {
  for (int i = 0; i < 1; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(1000));
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

void connectWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Connect to WiFi
  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(_SSID);
  WiFi.begin(_SSID, _PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(pdMS_TO_TICKS(500));

    Serial.print("-");
    displayWiFiIcon_Disconnected();
  }
  displayWiFiIcon_Connected();
  Serial.println("");
  Serial.println("WiFi Connected");
  beepConnectingSuccess();
  vTaskDelay(pdMS_TO_TICKS(2000));
}

void addCard() {
  bool cardDetected = false;

  while (!cardDetected) {
    display_REQUEST_CARD();
    Serial.println("WAIT CARD TAG TO ADD");
    char key = keypad.getKey();
    if (key == '*') {
      loopKEYY(true);
      return;
    }
    if (mfrc522.PICC_IsNewCardPresent()) {
      // Select the card
      if (mfrc522.PICC_ReadCardSerial()) {
        beepConnectingSuccess();
        Serial.println("NOT CARD");
        String content = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        content.toUpperCase();
        content.replace(" ", "");

        // Check if the card is already in the Firebase database
        bool exists = checkExists_firebase(content);

        if (exists) {
          Serial.println("Card already exists.");
          display_ALREADY();
        } else {
          display_SUCCESS_ADD();
          Serial.println("ADD Card Success!");
          addCard_firebase(content);
        }

        cardDetected = true;  // Exit the loop since card is detected
      }
    }

    // delay(500);  // Wait for a short period before checking again
  }
}



bool checkExists_firebase(String UID) {
  String check = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  String ckUID = firebase.getString(check);
  if (ckUID == "OK") {
    return true;
  } else {
    return false;
  }
}

void addCard_firebase(String UID) {
  String path = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  firebase.setString(path, "OK");
  Serial.print("Add Complete\n");
}

void revokeCard() {
  bool cardDetected = false;

  while (!cardDetected) {
    display_REQUEST_CARD();
    if (mfrc522.PICC_IsNewCardPresent()) {
      // Select the card
      if (mfrc522.PICC_ReadCardSerial()) {
        // Show UID on serial monitor
        beepConnectingSuccess();
        Serial.print("UID tag :");
        String content = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
          Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
          Serial.print(mfrc522.uid.uidByte[i], HEX);
          content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
          content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }
        Serial.println();
        Serial.print("Message : ");
        content.toUpperCase();
        content.replace(" ", "");

        // Check if the card exists in the stored UID array
        bool exists = checkExists_firebase(content);

        if (exists) {
          display_SUCCESS_ADD();
          Serial.println("Card have");
          revokeCard_firebase(content);
          Serial.println("ADD De Success!");
          // display_ALREADY();
        } else {
          Serial.println("REVORK : CARD NOT FOUND IN DATABASE");
          display_NOT_FOUND();
        }
        cardDetected = true;  // Exit the loop since card is detected
      }
    }

    delay(500);  // Wait for a short period before checking again
  }
}


void revokeCard_firebase(String UID) {
  String path = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  firebase.setString(path, "Deleted");
  Serial.print("Delete Complete\n");
}

void checkCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  display_DOOR_CHECKING();
  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();
  content.replace(" ", "");
  Serial.println(content);
  // Check if the card UID exists in the stored UID array
  bool exists = checkCard_firebase(content);

  if (exists) {  // Compare with the card UID directly
    // openDoor("KEY CARD");
  }

  // If the card UID doesn't exist in the stored UID array
  if (!exists) {
    display_DOOR_CHECKING_DENIED();
    Serial.println("ERROR: NOT FOUND");
    notOpenDoor("KEY CARD");
  }

  // Add delay to prevent rapid reading of the same card
  delay(1000);  // Adjust delay as needed
}

bool checkCard_firebase(String UID) {
  String path, data2;
  String check = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  String ckUID = firebase.getString(check);
  if (ckUID == "OK") {
    display_DOOR_CHECKING_GRANT();
    openDoor("KEY CARD");
    path = _FIREBASE_DEVICE_PATH + "/AllHis/" + UID;
    data2 = "KeyCard #" + UID + " -- | " + TimeString;
    firebase.pushString(_FIREBASE_DEVICE_PATH + "/AllHis/", data2);  //folder รวมประวัติทั้งหมด
    // firebase.pushString(path, TimeString);                                   //folder แยกประวัติ
    Serial.print("Detected\t  ");
    Serial.print(UID);
    Serial.print("\n");
    return true;
  } else {
    Serial.print("Not Found Data");
    return false;
  }
}

void TimeSetting() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "Failed to obtain time";
  }
  char buf[30];
  strftime(buf, sizeof(buf), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  return String(buf);
}

void setup() {
  pinModeSetup();
  defaultExecuteOnRun();
  oled_setup();
  Serial.begin(57600);
  fingerprint_verify();
  connectWiFi();
  Blynk.begin(BLYNK_AUTH_TOKEN, _SSID, _PASSWORD);
  // LINE.setToken(LINE_TOKEN);
  xTaskCreatePinnedToCore(
    core1Task,    // Task function
    "Core1Task",  // Task name
    10000,        // Stack size (bytes)
    NULL,         // Task parameters
    1,            // Priority
    &Core1Task,   // Task handle
    1             // Core number (Core 1)
  );
}


void openDoor(String type) {

  digitalWrite(RELAY_LOCK, HIGH);
  beepDoorOpen();
  // LEDColor(2, 0);
  //LINE.notify("DOOR OPEN WITH " + type);
  vTaskDelay(pdMS_TO_TICKS(2000));
  digitalWrite(RELAY_LOCK, LOW);
}

void notOpenDoor(String type) {
  beepDoorNotOpen();
  // LEDColor(0, 2, 0);
  //LINE.notify("DOOR NOT OPEN WITH" + type);
  vTaskDelay(pdMS_TO_TICKS(1000));
}

bool keyPinCheck() {
  // Define your PIN
  const String correctPIN = "8888";  // Change this to your desired PIN

  String enteredPIN = "";  // String to store the entered PIN

  while (enteredPIN.length() < correctPIN.length()) {
    char key = keypad.getKey();
    if (key == '*') {
      display_CLEAR();
      break;  // Exit the while loop if '*' key is pressed
      return false;
    }
    if (key != NO_KEY && isDigit(key)) {
      // Append the pressed key to the entered PIN
      enteredPIN += key;
      Serial.print(key);  // Print the pressed key to serial monitor
      delay(200);         // Delay to prevent multiple key presses
    }
  }

  // Check if the entered PIN matches the correct PIN
  if (enteredPIN == correctPIN) {
    // If the PIN is correct, unlock the door
    // openDoor("KEYPAD");
    Serial.println("\nDoor Unlocked");
    return true;
  } else {
    // If the PIN is incorrect, do nothing or display an error message
    // notOpenDoor("KEY PAD");
    Serial.println("\nIncorrect PIN");
    return false;
  }
}

BLYNK_WRITE(V1) {
  int value = param.asInt();
  openDoor("controller");
  vTaskDelay(pdMS_TO_TICKS(2000));
  Blynk.virtualWrite(V1, 0);
}

const char* desiredPasskey = "8888";
char passkey[5];  // Assuming the passkey is 4 characters long
bool passkeyEntered = false;

void loop() {
  display_DOOR_IDLE();
  Blynk.run();
  char key = keypad.getKey();
  bool redButtonState = digitalRead(RED_BUTTON_PIN);

  if (key == 'A') {
    passkeyEntered = false;  // Reset passkeyEntered flag
    loopKEYY(passkeyEntered);
  }

  if (redButtonState == HIGH || doorStatus) {
    openDoor("a");
    String pathh, data22;
    pathh = _FIREBASE_DEVICE_PATH + "/AllHis/";
    data22 = "Leave -- | " + TimeString;
    firebase.pushString(_FIREBASE_DEVICE_PATH + "/AllHis/", data22);  //folder รวมประวัติทั้งหมด
  } else {
    // Regardless of other conditions, check for a card
    Serial.println("Hold Card!");
    //led(255, 0, 0);
    checkCard();
    vTaskDelay(pdMS_TO_TICKS(100));
    fingerprint_check();
  }
}

void loopKEYY(bool passkeyEntered) {
  while (!passkeyEntered) {
    display_ADMIN_LOGIN();
    char exitKey = keypad.getKey();
    bool checkkkPin = keyPinCheck();
    if (checkkkPin) {
      passkeyEntered = true;
      // Selector menu for options 1-5
      int option = 0;
      while (option < 1 || option > 5) {
        display_admin_menu_main();
        char selectorKey = keypad.getKey();
        if (selectorKey >= '1' && selectorKey <= '5') {
          option = selectorKey - '0';  // Convert char to int
          handle_menu_option(option);
        }
      }
    }
    if (!checkkkPin) {
      display_login_admin_fail();
      break;
    }
  }
}

void handle_menu_option(int option) {
  // Handle menu options 1-5
  switch (option) {
    case 1:
      addCard();
      break;
    case 2:
      revokeCard();
      break;
    case 3:
      fingerprint_register();
      break;
    case 4:
      fingerprint_delete();
      break;
    case 5:
      // Handle option 5
      fingerprint_resetALL();
      break;
    case 0:
      break;
    default:
      // Handle invalid option
      break;
  }
}

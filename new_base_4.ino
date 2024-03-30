#define BLYNK_TEMPLATE_ID "TMPL67-vBBOHR"
#define BLYNK_TEMPLATE_NAME "IoT"
#define BLYNK_AUTH_TOKEN "yboj7bZVEVRiCMJa2NwjFfU4JFOgMpJs"
#define BLYNK_PRINT Serial

// #define LINE_TOKEN "dGVz4XvSOqJxEb6GjiFdrJMJTArkFkx21WYCz8BG3H6a"
#include <ESP32Firebase.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Keypad.h>
#include <MFRC522.h>
#include <time.h>
#include <BlynkSimpleEsp32.h>
#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <Arduino.h>
#include "time.h"
#include "pj_fingerprint.h"
// PROTOCOL SETUP
#define _SSID "vishsiri"                                                                              // Your WiFi SSID
#define _PASSWORD "11111111"                                                                          // Your WiFi Password
#define REFERENCE_URL "https://esp32test-trigger-default-rtdb.asia-southeast1.firebasedatabase.app/"  // Your Firebase project reference url

// VARIABLE SETUP
String inputString = "";  // String to hold the entered keys
String _FIREBASE_DEVICE_PATH = ""; //Set Device Path
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
const long  gmtOffset_sec = 7 * 3600;
const int   daylightOffset_sec = 0;

Firebase firebase(REFERENCE_URL);

//TASK CORE
TaskHandle_t Core1Task;

void core1Task(void *pvParameters) {
  while (1) {
    TimeSetting();
    TimeString = getTimeString();
    fingerprint_read();
    vTaskDelay(pdMS_TO_TICKS(100));
    fingerprint_write();
  }
}


// SETUP PIN
#define BUZZER_PIN 12
#define RED_BUTTON_PIN 27
#define RELAY_LOCK 14
#define OLED_RESET 16
#define LED_R 4
#define LED_G 3
#define LED_B 1
// #define MENETIC_SWITCH


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
#define RST_PIN 17
MFRC522 mfrc522(SS_PIN, RST_PIN);

//RGB
// OLED SCREEN


// FINGER PRINT MODULE


// RFID MODULE

// BLYNK
Adafruit_SSD1306 display(OLED_RESET);

void defaultExecuteOnRun() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);  //สั่งให้จอ OLED เริ่มทำงานที่ Address 0x3C
  display.clearDisplay();
  display.display();
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

void LEDColor(int r1, int r2, int r3) {
  static unsigned long previousMillis = 0;
  static unsigned long interval = 100;  // Adjust interval as needed
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    analogWrite(LED_R, r1);
    analogWrite(LED_G, r2);
    analogWrite(LED_B, r3);
    previousMillis = currentMillis;  // Update previousMillis
  }
}

void pinModeSetup() {
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RED_BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_LOCK, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void clearDisplay() {
  display.clearDisplay();
  display.display();
}

void updateDisplay(String str) {
  display.clearDisplay();
  display.setTextSize(2);  // Set text size to 2 (bigger)
  display.setTextColor(SSD1306_WHITE);

  // Calculate the x-coordinate to center the text
  int16_t x = (display.width() - (str.length() * 16 * 2)) / 2;  // 16 pixels per character

  // Draw the text at the calculated position
  display.setCursor(x, 0);

  // Replace each character with a star (*) and display
  for (int i = 0; i < str.length(); i++) {
    display.print('*');
  }

  display.display();
}

void beepConnectingSuccess() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for 100 milliseconds
    digitalWrite(BUZZER_PIN, LOW);
    vTaskDelay(pdMS_TO_TICKS(100)); // Delay for another 100 milliseconds
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
    vTaskDelay(pdMS_TO_TICKS(100));
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
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  beepConnectingSuccess();
}

void addCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : " + content);
  content.toUpperCase();
  content.replace(" ", "");

  // Check if the card is already in the Firebase database
  bool exists = checkExists_firebase(content);

  if (exists) {
    Serial.println("Card already exists.");

  } else {
    Serial.println("ADD Card Sucess!");

    addCard_firebase(content);
  }
}

bool checkExists_firebase(String UID) {
  String check = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  String ckUID = firebase.getString(check);
  if(ckUID == "OK"){
    return true;
  }
  else{
    return false;
  }
}

void addCard_firebase(String UID) {
  String path = _FIREBASE_DEVICE_PATH + "/RFID/AllowCard/" + UID;
  firebase.setString(path, "OK");
  Serial.print("Add Complete\n");
}

void revokeCard() {
  if (!mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  // Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  Serial.print("Message : ");
  content.toUpperCase();

  // Check if the card exists in the stored UID array
  bool exists = false;
  for (int i = 0; i < 10; i++) {
    if (cardUIDs[i] == content) {
      exists = true;
      // Remove the card UID from the array
      cardUIDs[i] = "";
      Serial.println("Card exists. Revoking access...");
      // Add logic to revoke access for the card
      // For testing purposes, you can simply indicate that access has been revoked
      Serial.println("Access revoked for card with UID: " + content);
      break;
    }
  }

  if (!exists) {
    Serial.println("Card not found in database.");
    // Add logic to indicate that the card is not found in the stored UID array
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
  if(ckUID == "OK"){
    openDoor("KEY CARD");
    path = _FIREBASE_DEVICE_PATH + "/RFID/AllHis/" + UID;
    data2 = UID + " -- : "+ TimeString;
    firebase.pushString(_FIREBASE_DEVICE_PATH + "/RFID/AllHis/All", data2);     //folder รวมประวัติทั้งหมด
    firebase.pushString(path, TimeString);     //folder แยกประวัติ
    Serial.print("Detected\t  ");
    Serial.print(UID);
    Serial.print("\n");
    return true;
  }
  else{
    Serial.print("Not Found Data");
    return false;
  }
}

void TimeSetting(){
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

String getTimeString() {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
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
  Serial.begin(115200);
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
  // LEDColor(2, 0, 0);
  //LINE.notify("DOOR OPEN WITH " + type);
  vTaskDelay(pdMS_TO_TICKS(2000));
}

void notOpenDoor(String type) {
  beepDoorNotOpen();
  // LEDColor(0, 2, 0);
  //LINE.notify("DOOR NOT OPEN WITH" + type);
  vTaskDelay(pdMS_TO_TICKS(1000));
}

void keyPinCheck() {
  // Define your PIN
  const String correctPIN = "8888";  // Change this to your desired PIN

  String enteredPIN = "";  // String to store the entered PIN

  while (enteredPIN.length() < correctPIN.length()) {
    char key = keypad.getKey();
    if (key != NO_KEY && isDigit(key)) {
      // Append the pressed key to the entered PIN
      enteredPIN += key;
      Serial.print(key);  // Print the pressed key to serial monitor
      delay(200);         // Delay to prevent multiple key presses

      // Display the entered PIN on the OLED display
      updateDisplay(enteredPIN);
    }
  }

  // Check if the entered PIN matches the correct PIN
  if (enteredPIN == correctPIN) {
    // If the PIN is correct, unlock the door
    openDoor("KEYPAD");
    Serial.println("\nDoor Unlocked");
  } else {
    // If the PIN is incorrect, do nothing or display an error message
    notOpenDoor("KEY PAD");
    Serial.println("\nIncorrect PIN");
  }
}

void keyPressEvent(char key) {
  switch (key) {
    case '1':
      break;
    case '2':
      break;
    case 'A':
      addCard();
      break;
    case 'B':
      revokeCard();
      break;
    default:
      break;
  }
}

BLYNK_WRITE(V0)
{
  int value = param.asInt();
  openDoor("controller");
  vTaskDelay(pdMS_TO_TICKS(2000));
  Blynk.virtualWrite(V0, 0);
}

void loop() {
  Blynk.run();
  char key = keypad.getKey();
  bool redButtonState = digitalRead(RED_BUTTON_PIN);

  if (key != '\0') {
    Serial.println(key);
    keyPressEvent(key);  // Call keyPressEvent() function when a key is pressed
    
    // Wait until a new key is pressed
    while (keypad.getKey() != '\0') {
      delay(50); // Adjust delay time as needed
    }
  }

  if (redButtonState == HIGH || doorStatus) {
    openDoor("a");
  } else {
    // Regardless of other conditions, check for a card
    Serial.println("Hold Card!");
    //led(255, 0, 0);
    checkCard();
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

#include <Arduino.h>
#include "pj_fingerprint.h"
#include <Adafruit_Fingerprint.h>
#include "smart_lacted_door.h"
#include "oled_screen.h"
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
uint8_t id;
uint8_t ids;
uint8_t idd;
bool fingers = false;
bool mismatch = false;
void fingerprint_verify() {
  finger.begin(57600);
  if (finger.verifyPassword()) {
    Serial.println("ตรวจพบเซ็นเซอร์!");
  } else {
    Serial.println("ตรวจไม่พบเซ็นเซอร์ :(");
    while (1) {
      delay(1);
    }
  }
}

uint8_t getFingerprintAmt() {
  finger.getTemplateCount();
  int amt = finger.templateCount;
  Serial.print("ข้อมูลภายในเซ็นเซอร์ ");
  Serial.print(amt);
  Serial.println(" รูปแบบ");
  finger.getTemplateCount();
  Serial.println("ข้อมูลภายในเซ็นเซอร์ ");
  Serial.print(finger.templateCount);
  Serial.println(" รูปแบบ");
  return amt;
}

void fingerprint_check() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) {
    Serial.println("Error capturing fingerprint image.");
    return;
  }

  fingers = getFingerprintID();
  if (fingers != 0) {  // Check if fingerprint ID is not 0
    display_DOOR_CHECKING_GRANT();
    openDoor("finger");
    String pathh, data22;
    pathh = _FIREBASE_DEVICE_PATH + "/AllHis/";
    data22 = "Finger #";
    data22 += id;
    data22 += " -- | ";
    data22 += TimeString;
    firebase.pushString(_FIREBASE_DEVICE_PATH + "/AllHis/", data22);  //folder รวมประวัติทั้งหมด
  } else {
    display_DOOR_CHECKING_DENIED();
    Serial.println("ERROR: NOT FOUND");
    notOpenDoor("KEY CARD");
    return;
    // Serial.println("Valid fingerprint ID not obtained.");
  }
}



bool check_firebase_exists(int id) {
  String check = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + id;
  String ckUID = firebase.getString(check);
  Serial.println(ckUID);
  if (ckUID == "OK") {
    return false;
  } else {
    return true;
  }
}

void fingerprint_resetALL() {
  for (int i = 0; i < 2; i++) {
    finger.emptyDatabase();
    Serial.println("Now database is empty :)");
    delay(500);
  }
  firebase_reset_fingerprint();
  display_SUCCESS_ADD();
}

void fingerprint_register() {
  display_HOLD_FINGER();
  Serial.println("พร้อมสำหรับการบันทึกข้อมูลลายนิ้วมือ!");
  // id =getFingerprintID();
  // // // Check if fingerprint ID already exists in the module
  // bool check = check_firebase_exists(id);
  // if (check) {
  //   firebase_add_fingerprint(id);
  //   return;
  // }

  ids = getFingerprintAmt();
  ids += 1;
  Serial.print("หมายเลข ID #");
  Serial.println(ids);
  while (!getFingerprintEnroll())
    ;
  if(mismatch) {
    display_login_admin_fail();
  }
  if(!mismatch) {
    display_SUCCESS_ADD();
  } 
}


void fingerprint_delete() {

  display_HOLD_FINGER();
  Serial.println("พร้อมสำหรับการบันทึกข้อมูลลายนิ้วมือ!");
  while (!getFingerprintID())
    ;
  idd = id;
  Serial.print("หมายเลข ID #");
  Serial.println(idd);
  firebase_remove_fingerprint(idd);
  Serial.println("ADD DELETTEE SUSCESS@@@");
  Serial.print(idd);
  deleteFingerprint(idd);
  display_SUCCESS_ADD();
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}




uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("wait finger #");
  Serial.println(ids);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("image finger");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("asdasd");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("qweqwe");
        break;
      default:
        Serial.println("tyjtyj");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("แปลงรูปภาพ");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("รูปภาพไม่ชัดเจน");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("ไม่พบคุณสมบัติลายนิ้วมือ");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("ไม่พบคุณสมบัติลายนิ้วมือ");
      return p;
    default:
      Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
      return p;
  }
  display_REMOVE();
  Serial.println("โปรดนำนิ้วมือออกจากเซ็นเซอร์");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(ids);
  p = -1;
  display_AGAIN();
  Serial.println("วางนิ้วมือเดิมลงอีกครั้ง");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("ภาพลายนิ้วมือ");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("พบข้อผิดพลาดของภาพลายนิ้วมือ");
        break;
      default:
        Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("แปลงรูปภาพ");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("รูปภาพไม่ชัดเจน");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("ไม่พบคุณสมบัติลายนิ้วมือ");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("ไม่พบคุณสมบัติลายนิ้วมือ");
      return p;
    default:
      Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
      return p;
  }

  // OK converted!
  Serial.print("สร้างแบบจำลองสำหรับ #");
  Serial.println(ids);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("ลายนิ้วมือตรงกัน!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("ลายนิ้วมือไม่ตรงกัน");
    mismatch = true;
    return p;
  } else {
    Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
    return p;
  }

  Serial.print("ID ");
  Serial.println(ids);
  p = finger.storeModel(ids);
  if (p == FINGERPRINT_OK) {
    mismatch = false;
    Serial.println("บันทึกลายนิ้วมือ!");
    firebase_add_fingerprint(ids);
    Serial.println("ADD FINGER SUSCESS@@@");
    Serial.print(ids);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("ไม่สามารถจัดเก็บในตำแหน่งนั้นได้");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("เกิดข้อผิดพลาดในการบันทึกข้อมูล");
    return p;
  } else {
    Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
    return p;
  }
}


uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x");
    Serial.println(p, HEX);
  }

  return p;
}


bool getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return false;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return false;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return false;

  // found a match!
  Serial.print("พบลายนิ้วมือ ID #");
  Serial.print(finger.fingerID);
  Serial.print(" ซึ่งมีความใกล้เคียงกัน ");
  Serial.println(finger.confidence);

  bool have = firebase_check_fingerprint(finger.fingerID);
  id = finger.fingerID;
  return have;
}


void firebase_remove_fingerprint(int id) {
  String path = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + id;
  firebase.setString(path, "Delete");
  Serial.print("Delete Complete\n");
}

void firebase_reset_fingerprint() {
  firebase.deleteData(_FIREBASE_DEVICE_PATH + "/Finger/AllowFinger");
  Serial.print("Delete All Data Complete\n");
  String path = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + 0;
  firebase.setString(path, "Delete");
  Serial.print("Set Defalut Complete!");
}

void firebase_add_fingerprint(int id) {
  String path = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + id;
  firebase.setString(path, "OK");
  Serial.print("Add Complete\n");
}

bool firebase_check_fingerprint(int id) {
  String check = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + id;
  String ckUID = firebase.getString(check);
  Serial.println(ckUID);
  if (ckUID == "OK") {
    return true;
  } else {
    return false;
  }
}
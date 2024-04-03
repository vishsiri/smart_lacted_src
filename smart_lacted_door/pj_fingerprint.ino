#include <Arduino.h>
#include "pj_fingerprint.h"
#include <Adafruit_Fingerprint.h>
#include "smart_lacted_door.h"
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&Serial2);
uint8_t id;


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

int getFingerprintAmt() {
  int amt = finger.getTemplateCount();
  Serial.print("ข้อมูลภายในเซ็นเซอร์ ");
  Serial.print(finger.templateCount);
  Serial.println(" รูปแบบ");
  return amt;
}

void fingerprint_check() {
  int data = finger.getTemplateCount();
  vTaskDelay(pdMS_TO_TICKS(50));
  bool finger_id = getFingerprintID();
  if (finger_id) {
    openDoor("a");
  } else {
    Serial.println("NOT HAVE FINGER");
  }
}

void fingerprint_register() {
  Serial.println("พร้อมสำหรับการบันทึกข้อมูลลายนิ้วมือ!");
  Serial.println("โปรดเลือก ID # (จาก 1 to 127) ที่คุณต้องการบันทึกข้อมูลลายนิ้วมือ...");
  id = finger.templateCount + 1;
  if (id == 0) {
    return;
  }
  Serial.print("หมายเลข ID #");
  Serial.println(id);
  while (!getFingerprintEnroll())
    ;
}

void fingerprint_delete(int id) {
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
  Serial.print("กำลังรอนิ้วมือเพื่อบันทึกข้อมูล #");
  Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("ภาพลายนิ้วมือ");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
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

  Serial.println("โปรดนำนิ้วมือออกจากเซ็นเซอร์");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
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
  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("ลายนิ้วมือตรงกัน!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("พบข้อผิดพลาดของการเชื่อมต่อ");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("ลายนิ้วมือไม่ตรงกัน");
    return p;
  } else {
    Serial.println("พบข้อผิดพลาดที่ไม่รู้จัก");
    return p;
  }

  Serial.print("ID ");
  Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("บันทึกลายนิ้วมือ!");
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


bool getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) return -1;

  // found a match!
  Serial.print("พบลายนิ้วมือ ID #");
  Serial.print(finger.fingerID);
  Serial.print(" ซึ่งมีความใกล้เคียงกัน ");
  Serial.println(finger.confidence);

  bool have = firebase_check_fingerprint(finger.fingerID);

  return have;
}


bool firebase_check_fingerprint(int id) {
  String check = _FIREBASE_DEVICE_PATH + "/Finger/AllowFinger/" + id;
  String ckUID = firebase.getString(check);
  if (ckUID == "OK") {
    return true;
  } else {
    return false;
  }
}
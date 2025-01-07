#include <SPI.h>
#include "RF24.h"
#include <printf.h>
#include <Servo.h>
RF24 rf24(7, 8, 4000000);  // CE腳, CSN腳
Servo myservo;

// ===========設定為接收模式===========
byte addr[] = "1Node";
byte pipe = 1;  // 指定通道編號
void setreceiveMode() {
  rf24.setChannel(83);  // 設定頻道編號
  rf24.setPALevel(RF24_PA_MIN);
  rf24.setDataRate(RF24_250KBPS);
  rf24.openReadingPipe(pipe, addr);  // 讀取地址設定
  rf24.startListening();             // 開始監聽無線廣播
}
// ===========設定為傳送模式===========
byte addr0[] = "0Node";
void setsendMode() {
  rf24.setChannel(84);  // 設定頻道編號
  rf24.setPALevel(RF24_PA_MIN);
  rf24.setDataRate(RF24_250KBPS);
  rf24.openWritingPipe(addr0);  // 讀取地址設定
  rf24.stopListening();         // 開始傳送資料
}

void setup() {
  //設定通訊模塊
  Serial.begin(9600);
  rf24.begin();
  setreceiveMode();
  printf_begin();
  rf24.printDetails();
  // 設定電變驅動(無刷風扇)
  myservo.attach(3);
  // 設定馬達 (輪子)
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  // 設定電壓監控
  pinMode(A0, INPUT);
  delay(100);
  myservo.write(60);
  delay(1000);
  myservo.write(1000);
  delay(1000);
}
unsigned long previousMillis = 0;
int sr = 50;
int fly_sw = 0;
bool fly = false;
bool jump = false;
void loop() {
  //傳送模式
  setsendMode();
  while (millis() - previousMillis < 50) {
    float voltage = analogRead(A0) * (5.0 / 1023.0) * 2;
    String v = String(voltage) + ";" + String(sr);
    Serial.println(v);
    char msg1[32];
    v.toCharArray(msg1, v.length() + 1);
    rf24.write(&msg1, sizeof(msg1));  // 傳送資料
    // 還回發射模式
  }
  previousMillis = millis();
  setreceiveMode();
  unsigned long previousMillis2 = millis();
  while (millis() - previousMillis < 150) {
    if (rf24.available(&pipe)) {
      char msg[32] = "";
      rf24.read(&msg, sizeof(msg));
      String cat = String(msg);
      int a = getValue(cat, ';', 0).toInt();
      int b = getValue(cat, ';', 1).toInt();
      int c = getValue(cat, ';', 2).toInt();
      //風扇
      if (a > 100) {
        if (millis() - previousMillis2 >= 50) {
          previousMillis2 = millis();
          
          if (!jump) {
            // 長按開啟
            if (!fly) {
              fly_sw++;
              if (fly_sw > 50) {
                fly_sw = 0;
                fly = true;
                myservo.write(80);
                jump = true;
                sr = 180;
              }
              // 長按關閉
            } else {
              fly_sw++;
              if (fly_sw > 50) {
                fly_sw = 0;
                fly = false;
                myservo.write(0);
                jump = true;
                sr = 0;
              }
            }
          }
        }
      }else{
        jump = false;
      }



      // if (a > 100) {
      //   if (millis() - previousMillis2 >= 50) {
      //     previousMillis2 = millis();
      //     if (sr <= 179) {
      //       if (sr < 49) {
      //         sr = 49;
      //       } else {
      //         sr++;
      //       }
      //     }
      //   }
      //   myservo.write(sr);
      //   continue;
      // } else {
      //   sr = 0;
      //   myservo.write(sr);
      // }





      //輪子控制
      // 左右
      if (c < 250) {
        setLeft();
        continue;
      } else if (c > 800) {
        setRight();
        continue;
      }
      if (b < 250) {
        setCW();
        continue;
      } else if (b > 800) {
        setCCW();
        continue;
      } else {
        stopALL();
      }
    }
  }
  previousMillis = millis();
  setsendMode();
  while (millis() - previousMillis < 50) {
    float voltage = analogRead(A0) * (5.0 / 1023.0) * 2;
    String v = String(voltage) + ";" + String(sr);
    Serial.println(v);
    char msg1[32];
    v.toCharArray(msg1, v.length() + 1);
    rf24.write(&msg1, sizeof(msg1));  // 傳送資料
  }
}

// 馬達控制函數
void setCW() {
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);
}
void setCCW() {
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
}
void setLeft() {
  digitalWrite(5, HIGH);
  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, HIGH);
}
void setRight() {
  digitalWrite(5, LOW);
  digitalWrite(6, HIGH);
  digitalWrite(9, HIGH);
  digitalWrite(10, LOW);
}
void stopALL() {
  digitalWrite(5, LOW);
  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
}

// 系統運算函數
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;
  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

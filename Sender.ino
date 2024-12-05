#include <SPI.h>
#include "RF24.h"

RF24 rf24(7, 8);  // CE腳, CSN腳

// ===========設定為傳送模式===========
byte addr[] = "1Node";
void setsendMode() {
  rf24.setChannel(83);             // 設定頻道編號
  rf24.openWritingPipe(addr);      // 設定通道位址
  rf24.setPALevel(RF24_PA_MIN);    // 設定廣播功率
  rf24.setDataRate(RF24_250KBPS);  // 設定傳輸速率
  rf24.stopListening();            // 停止偵聽；設定成發射模式
}
// ===========設定為接收模式===========
byte addr0[] = "0Node";
byte pipe = 0;  // 指定通道編號
void setreceiveMode() {
  rf24.setChannel(84);  // 設定頻道編號
  rf24.setPALevel(RF24_PA_MIN);
  rf24.setDataRate(RF24_250KBPS);
  rf24.openReadingPipe(pipe, addr0);  // 讀取地址設定
  rf24.startListening();              // 開始監聽無線廣播
}



void setup() {
  rf24.begin();
  Serial.begin(9600);
  setsendMode();
  // 設定input
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
}
unsigned long previousMillis = 0;
void loop() {
  while (millis() - previousMillis < 100) {
    setreceiveMode();
    if (rf24.available(&pipe)) {
      char msg[64] = "";
      rf24.read(&msg, sizeof(msg));
      Serial.println(String(msg));
      Serial.println("monitor:");
    }
  }
  previousMillis = millis();
  while (millis() - previousMillis < 500) {
    setsendMode();
    String ram = String((millis() / 1000)) + "秒" + ";" + String(analogRead(0)) + ";" + String(analogRead(1)) + ";" + String(analogRead(3));
    //Serial.println(ram);
    char msg[64];
    ram.toCharArray(msg, ram.length() + 1);
    rf24.write(&msg, sizeof(msg));  // 傳送資料
  }
  previousMillis = millis();
}

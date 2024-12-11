#include <SPI.h>
#include "RF24.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#define OLED_RESET -1
Adafruit_SH1106 display(OLED_RESET);
#if (SH1106_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SH1106.h!");
#endif

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
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // 一般1306 OLED的位址都是0x3C
  display.clearDisplay();
  display.setTextSize(2);     // 設定文字大小
  display.setTextColor(1);        // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 0);            // 設定起始座標
  display.print("Booting...");        // 要顯示的字串
  display.display();
  delay(500);
  rf24.begin();
  Serial.begin(9600);
  display.setTextSize(1);     // 設定文字大小
  display.setTextColor(1);        // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 16);
  display.print("RF24.....OK");
  display.display();
  setreceiveMode();
  display.setCursor(0, 32);
  display.print("MODE.....REV");
  display.display();
  // 設定input
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  display.setCursor(0, 48);
  display.print("PINSET.....ok");
  display.display();
  delay(1000);
  while (!rf24.available(&pipe)) {
    waitingLogo();
  }
  delay(1000);
  setreceiveMode();
}
unsigned long previousMillis = 0;
bool start = false;
String m;
void loop() {
  while (millis() - previousMillis < 10000) {
    setreceiveMode();
    if (rf24.available(&pipe)) {
      char msg[64] = "";
      rf24.read(&msg, sizeof(msg));
      Serial.println(String(msg));
      Serial.println("monitor:");
      m = String(msg);
    }
  }
  MonitorLogo(m);
  previousMillis = millis();
  while (millis() - previousMillis < 500) {
    setsendMode();
    String ram = String((millis() / 1000)) + "秒" + ";" + String(analogRead(0)) + ";" + String(analogRead(1)) + ";" + String(analogRead(3));
    Serial.println(ram);
    char msg[64];
    ram.toCharArray(msg, ram.length() + 1);
    rf24.write(&msg, sizeof(msg));  // 傳送資料
  }
  previousMillis = millis();
}

void waitingLogo() {
  display.clearDisplay();
  display.setTextSize(2);     // 設定文字大小
  display.setTextColor(1);        // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 0);            // 設定起始座標
  display.print("Waiting");        // 要顯示的字串
  display.setTextSize(1);     // 設定文字大小
  display.setCursor(0, 20);
  display.print("Please boot the Receiving-side,When the signal is received, the process will continue");
  display.display();
  display.setTextSize(2);     // 設定文字大小
  display.setCursor(80, 0);            // 設定起始座標
  delay(100);
  display.print(".");        // 要顯示的字串
  display.display();
  delay(100);
  display.print(".");        // 要顯示的字串
  display.display();
  delay(100);
  display.print(".");        // 要顯示的字串
  display.display();
  delay(100);
  display.print(".");        // 要顯示的字串
  display.display();
  delay(100);
}
void MonitorLogo(String data) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);            // 設定起始座標
  display.print("Monitor:");        // 要顯示的字串
  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("--------------");
  display.setCursor(0, 30);
  display.print("BT:    |  FAN_RPM:");
  display.setCursor(0, 40);
  display.print(getValue(data, ';', 0) + "    " + getValue(data, ';', 1));
  display.display();
}
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

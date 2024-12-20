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

void LEDsetip() {
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);  // 一般1306 OLED的位址都是0x3C
  display.clearDisplay();
  display.setTextSize(2);       // 設定文字大小
  display.setTextColor(1);      // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 0);      // 設定起始座標
  display.print("Booting...");  // 要顯示的字串
  display.display();
}
void failedmsg(String modle, String data, int se) {
  for (int i = se; i >= 0; i--) {  // 使用 for 循環，從 10 遞減到 0
    display.clearDisplay();
    display.setTextSize(2);   // 設定文字大小
    display.setTextColor(1);  // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
    display.setCursor(0, 0);  // 設定起始座標
    display.print("ERROR!");
    display.setTextSize(1); 
    display.setCursor(0, 18);
    display.print(modle);
    display.setCursor(0, 29);
    display.print(data);
    display.setCursor(0, 55);
    display.print("Retry in ....... " + String(i) + "se");
    display.display();
    delay(1000);
  }
}
bool selftest() {
  display.setTextSize(1);   // 設定文字大小
  display.setTextColor(1);  // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 16);
  if (!rf24.begin()) {
    failedmsg("RF24.........! FALL !", "Cannot Initialization RF24,Please cheack the RF24 modle", 10);
    return false;
  }
  setreceiveMode();
  display.print("RF24.....OK");
  display.display();
  delay(250);
  display.setCursor(0, 28);
  display.print("MODE.....REV");
  display.display();
  delay(250);
  pinMode(0, INPUT);
  pinMode(1, INPUT);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  pinMode(5, INPUT);
  display.setCursor(0, 40);
  display.print("PINSET.....Done");
  display.display();
  delay(250);
  display.setCursor(0, 52);
  float voltage = analogRead(A2) * (5.0 / 1023.0);
  if (voltage < 3.5) {
    failedmsg("Vin........! FALL !", "Vin votage to low(" +String(voltage) +"v) ,Please check battery/transformer", 10);
    return false;
  }
  display.print("Vin.....OK");
  display.display();
  return true;
}

void setup() {
  LEDsetip();
  delay(500);
  if (!selftest()) {
    setup();
    return;
  }
  delay(1000);
  while (!rf24.available(&pipe)) {
    waitingLogo();
  }
}
unsigned long previousMillis = 0;
unsigned long LastRSVMillis = 0;
bool start = false;
String m;
void loop() {
  while (millis() - previousMillis < 70) {
    setreceiveMode();
    if (rf24.available(&pipe)) {
      char msg[32] = "";
      rf24.read(&msg, sizeof(msg));
      m = String(msg);
      LastRSVMillis = millis();
    }
  }
  MonitorLogo(m);
  previousMillis = millis();
  while (millis() - previousMillis < 510) {
    setsendMode();
    String ram = String(analogRead(0)) + ";" + String(analogRead(1)) + ";" + String(analogRead(3));
    char msg[32];
    ram.toCharArray(msg, ram.length() + 1);
    rf24.write(&msg, sizeof(msg));  // 傳送資料
  }
  previousMillis = millis();
}

void waitingLogo() {
  display.clearDisplay();
  display.setTextSize(2);    // 設定文字大小
  display.setTextColor(1);   // 1:OLED預設的顏色(這個會依該OLED的顏色來決定)
  display.setCursor(0, 0);   // 設定起始座標
  display.print("Waiting");  // 要顯示的字串
  display.setTextSize(1);    // 設定文字大小
  display.setCursor(0, 20);
  display.print("To ensure Half-Duplex timing synchronization, the system will wait until receive RSV-side's ready signal");
  display.display();
  display.setTextSize(2);    // 設定文字大小
  display.setCursor(80, 0);  // 設定起始座標
  for (int i = 0; i <= 3; i++) {
    delay(25);
    display.print(".");  // 要顯示的字串
    display.display();
    delay(25);
  }
}
bool b = 1;
void MonitorLogo(String data) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);    // 設定起始座標
  display.print("Monitor:");  // 要顯示的字串
  display.setTextSize(1);
  display.setCursor(0, 15);
  display.print("--------------------");
  display.setCursor(0, 25);
  display.print("BT:    | FAN_RPM:");
  display.setCursor(0, 35);
  if (getValue(data, ';', 0).toInt() < 7) {
    if (b) {
      display.print(getValue(data, ';', 0) + "V");
      b = 0;
    } else {
      display.print(getValue(data, ';', 0) + "V !!");
      b = 1;
    }
  } else {
    display.print(getValue(data, ';', 0) + "V");
  }
  display.setCursor(55, 35);
  if (getValue(data, ';', 1).toInt() < 49){
      display.print("0/14720");
  }else{
  display.print(String(map(getValue(data, ';', 1).toInt(), 49, 180, 0, 14720))+"/14720");
  }
  display.setCursor(0, 45);
  display.print("--------------------");
  display.setCursor(0, 55);
  float voltage = analogRead(A2) * (5.0 / 1023.0);
  display.print(String(voltage) + "V  | LRS:" + String((millis() - LastRSVMillis) / 1000) + "Se");
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

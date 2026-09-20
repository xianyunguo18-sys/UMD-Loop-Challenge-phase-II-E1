// 发送端：Arduino Uno
// 作用：通过一根导线，把HEX颜色文本发送给接收端ESP32
// 接线：Uno 3号脚(TX) --[1kΩ]--+--> ESP32 GPIO16(RX)
//                              |
//                            [2kΩ]
//                              |
//                             GND
//       Uno GND -> ESP32 GND
// （分压是因为Uno输出5V，而ESP32引脚只能承受3.3V）

#include <SoftwareSerial.h>   // Arduino自带，不需要额外安装

SoftwareSerial wire(2, 3);    // RX=2（本例不用）, TX=3

const char* colors[] = {"#FF0000", "#00FF00", "#0000FF", "#FF8800", "#10A0E0"};
const int NUM_COLORS = sizeof(colors) / sizeof(colors[0]);
int idx = 0;
unsigned long lastSend = 0;

void setup() {
  Serial.begin(9600);    // USB串口监视器（调试用）
  wire.begin(9600);      // 导线上的通信，波特率必须与ESP32接收端一致
  Serial.println("Uno sender ready");
}

void loop() {
  // 方式1：每2秒自动发送一个颜色
  if (millis() - lastSend > 2000) {
    lastSend = millis();
    wire.println(colors[idx]);        // println会加换行，接收端靠换行判断一条结束
    Serial.print("sent: ");
    Serial.println(colors[idx]);
    idx = (idx + 1) % NUM_COLORS;
  }

  // 方式2：把串口监视器里手动输入的内容转发出去
  if (Serial.available()) {
    String s = Serial.readStringUntil('\n');
    s.trim();
    if (s.length() > 0) {
      wire.println(s);
      Serial.print("sent: ");
      Serial.println(s);
    }
  }
}

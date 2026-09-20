// System ONE（自动识别输入引脚版）
// 上电后，ESP32会在一组候选GPIO里等待信号，哪个引脚先出现信号，就把串口接收（RX）绑定到它。
// 因此HEX信号线可以接在候选列表里的任意一个引脚上，不需要改代码。
// 换引脚后按一下RST（或重新上电），再等发送端发下一条即可。

#include <Arduino.h>

// ---------- 候选输入引脚（信号线可以接在其中任意一个）----------
const int CANDIDATES[] = {4, 5, 13, 14, 16, 17, 18, 19, 21, 22, 23, 32};
const int NUM_CANDIDATES = sizeof(CANDIDATES) / sizeof(CANDIDATES[0]);

// 串口必须指定一个发送引脚：选一个空闲的、不接任何东西的引脚
const int DUMMY_TX_PIN = 33;

const int LED_R = 25;
const int LED_G = 26;
const int LED_B = 27;

String lineUart;
String lineUsb;
int rxPin = -1;

void allOff() {
  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

bool parseHex(String s, int &r, int &g, int &b) {
  s.trim();
  if (s.startsWith("#")) s = s.substring(1);
  if (s.startsWith("0x") || s.startsWith("0X")) s = s.substring(2);
  if (s.length() != 6) return false;
  for (unsigned int i = 0; i < s.length(); i++) {
    if (!isxdigit(s[i])) return false;
  }
  long v = strtol(s.c_str(), NULL, 16);
  r = (v >> 16) & 0xFF;
  g = (v >> 8) & 0xFF;
  b = v & 0xFF;
  return true;
}

long dist2(int r, int g, int b, int tr, int tg, int tb) {
  long dr = r - tr, dg = g - tg, db = b - tb;
  return dr * dr + dg * dg + db * db;
}

void handleHex(const String &text) {
  int r, g, b;
  if (!parseHex(text, r, g, b)) {
    Serial.println("格式错误，请输入6位HEX，例如 #FF0000");
    return;
  }
  long dR = dist2(r, g, b, 255, 0, 0);
  long dG = dist2(r, g, b, 0, 255, 0);
  long dB = dist2(r, g, b, 0, 0, 255);

  allOff();
  if (dR <= dG && dR <= dB) {
    digitalWrite(LED_R, HIGH);
    Serial.println("closest color：red");
  } else if (dG <= dR && dG <= dB) {
    digitalWrite(LED_G, HIGH);
    Serial.println("closest color：green");
  } else {
    digitalWrite(LED_B, HIGH);
    Serial.println("closest color：blue");
  }
  Serial.printf("R=%d G=%d B=%d\n", r, g, b);
}

void readLine(Stream &port, String &buf) {
  while (port.available()) {
    char c = port.read();
    if (c == '\n' || c == '\r') {
      if (buf.length() > 0) {
        handleHex(buf);
        buf = "";
      }
    } else {
      buf += c;
    }
  }
}

// 等待：候选引脚里哪个先被拉低（串口的起始位是低电平），就是信号线所在的引脚
int detectRxPin() {
  for (int i = 0; i < NUM_CANDIDATES; i++) {
    pinMode(CANDIDATES[i], INPUT_PULLUP);   // 串口空闲时是高电平，上拉让悬空引脚保持高电平
  }
  Serial.println("detecting which GPIO recieve HEX signal，waiting for input...");
  while (true) {
    for (int i = 0; i < NUM_CANDIDATES; i++) {
      if (digitalRead(CANDIDATES[i]) == LOW) {
        return CANDIDATES[i];
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  allOff();

  rxPin = detectRxPin();
  Serial.printf("detected GPIO：GPIO%d\n", rxPin);

  Serial2.begin(9600, SERIAL_8N1, rxPin, DUMMY_TX_PIN);
  delay(100);                                // 等这一帧数据结束
  while (Serial2.available()) Serial2.read();  // 丢掉检测时收到的不完整数据
  lineUart = "";
  Serial.println("waiting for next HEX");
}

void loop() {
  readLine(Serial2, lineUart);   // 来自自动检测到的GPIO引脚
  readLine(Serial, lineUsb);     // 来自USB（测试用）
}

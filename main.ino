#include <LiquidCrystal_I2C_Hangul.h>
#include <Wire.h>
#include <wctype.h>
#include <WiFi.h>

//고질적인 문제: 한글 출력 불가.

byte find_address(void) {
  byte error, address;
  int nDevices = 0;
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      return address;
      nDevices++;
    } else if (error == 4) {
      continue;
    }
  }
}

byte addr = 0;
LiquidCrystal_I2C_Hangul *lcd = nullptr;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Wire.begin();
  if (!addr) {
    addr = find_address();
    if (addr) {
      Serial.println(addr);
      lcd = new LiquidCrystal_I2C_Hangul(addr, 16, 2);
      lcd->init();
      lcd->setDelayTime(300);
      lcd->backlight();
      lcd->printHangul(L"시작",0,2);
    } else {
      delay(1000);
      return;
    }
  }
}

wchar_t* utf8_to_wchar(const char* utf8Str) {
  size_t len = strlen(utf8Str);
  wchar_t* wcharStr = new wchar_t[len + 1]; // 널 문자 공간 추가
  int wcharIndex = 0;
  const unsigned char* p = (const unsigned char*)utf8Str;

  while (*p) {
    if ((*p & 0x80) == 0) { // 1바이트 문자
      wcharStr[wcharIndex++] = *p;
      p++;
    } else if ((*p & 0xE0) == 0xC0) { // 2바이트 문자
      wcharStr[wcharIndex++] = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
      p += 2;
    } else if ((*p & 0xF0) == 0xE0) { // 3바이트 문자
      wcharStr[wcharIndex++] = ((p[0] & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F);
      p += 3;
    } else {
      // 4바이트 이상의 문자는 처리하지 않음
      break;
    }
  }

  wcharStr[wcharIndex] = L'\0'; // 널 문자 추가
  return wcharStr;
}

void loop() {


  int found_wifi = WiFi.scanNetworks();
  lcd->clear();

  if (!found_wifi) {
    lcd->setCursor(0, 0);
    lcd->print("No Networks");
  } else {
    for (int i = 0; i < found_wifi && i < 4; i++) {
      lcd->clear();
      lcd->setCursor(0, 0);

      String ssid = WiFi.SSID(i);
      Serial.println("SSID: " + ssid);
      wchar_t *str = new wchar_t[17];
      str = utf8_to_wchar(ssid.c_str());
      lcd->printHangul(str,0,wcslen(str));

      char s[20];
      sprintf(s, "%.1f%%", (WiFi.RSSI(i) + 100.0F));
      lcd->setCursor(16-strlen(s), 1);
      Serial.println(s);
      lcd->print(s);

      delay(1000);  // 2초마다 갱신
    }
  }
  delay(500);
}

#include <WiFi.h>
#include "LuxpowerTCP.h"

LuxpowerTCP inverter;

void setup() {
  Serial.begin(115200);
  WiFi.begin("WIFI_SSID", "WIFI_PASS");
  while (WiFi.status() != WL_CONNECTED) delay(500);

  // Khởi tạo thư viện với IP và SN
  inverter.begin("192.168.1.150", "BA12345678", "1234567890");

  // Ví dụ: Điều khiển chỉnh dòng sạc bình lưu trữ thành 40A
  if (inverter.setChargeCurrent(40)) {
    Serial.println("Cài đặt dòng sạc 40A thành công!");
  }
}

void loop() {
  LuxRealtimeData rt;

  // Đọc thông số thời gian thực
  if (inverter.readRealtime(rt)) {
    Serial.printf("PV: %u W | Pin: %.1fV (%u%%) | Sạc: %u W | Lưới: %d W\n",
                  rt.pPvTotal, rt.vBat, rt.soc, rt.pCharge, rt.pGrid);
  } else {
    Serial.println("Đọc dữ liệu thất bại!");
  }

  delay(5000);
}

#include <WiFi.h>
#include <LuxpowerTCP.h>

const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASS     = "YOUR_WIFI_PASS";

// Điền đúng 10 ký tự
const char* DONGLE_SN     = "BA12345678";
const char* INVERTER_SN   = "1234567890";
const char* INVERTER_IP   = ""; // Để trống nếu muốn ESP32 tự quét cổng 8000

LuxpowerTCP inverter;

void setup() {
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\nWiFi connected! IP: %s\n", WiFi.localIP().toString().c_str());

  inverter.begin(INVERTER_IP, DONGLE_SN, INVERTER_SN);
  inverter.setInterval(3); // Cài đặt chu kỳ 3 giây

  // Tự quét IP nếu chưa điền
  if (strlen(INVERTER_IP) == 0) {
    Serial.println("Đang quét dải IP tìm biến tần...");
    String foundIP = inverter.discoverIP();
    if (foundIP.length() > 0) {
      Serial.printf("Tìm thấy biến tần tại: %s\n", foundIP.c_str());
    } else {
      Serial.println("Không tìm thấy biến tần trên cổng 8000!");
    }
  }
}

void loop() {
  if (inverter.shouldPoll()) {
    LuxRealtimeData rt;
    LuxEnergyData   en;

    if (inverter.readRealtime(rt)) {
      Serial.println("\n--- DỮ LIỆU THỜI GIAN THỰC ---");
      Serial.printf("PV Tổng  : %u W (PV1: %u W, PV2: %u W)\n", rt.pPvTotal, rt.pPv1, rt.pPv2);
      Serial.printf("Pin      : %.1f V | SOC: %u%% | Sạc: %u W | Xả: %u W\n", 
                    rt.vBat, rt.soc, rt.pCharge, rt.pDischarge);
      Serial.printf("Điện lưới: %.1f V | %d W (+ Mua / - Bán)\n", rt.vGrid, rt.pGrid);
      Serial.printf("Tải EPS  : %.1f V | %u W\n", rt.vEps, rt.pEps);
      Serial.printf("Nhiệt độ : %u °C\n", rt.tempRad);
    }

    if (inverter.readEnergy(en)) {
      Serial.printf("Sản lượng PV hôm nay: %.1f kWh | Tổng: %.1f kWh\n", 
                    en.epvToday, en.epvTotal);
    }
  }
}

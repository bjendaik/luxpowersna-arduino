#ifndef LUXPOWER_TCP_H
#define LUXPOWER_TCP_H

#include <Arduino.h>
#include <WiFi.h>

// Cấu trúc dữ liệu thời gian thực
struct LuxRealtimeData {
  uint16_t status;
  float    vPv1;
  float    vPv2;
  uint16_t pPv1;
  uint16_t pPv2;
  uint16_t pPvTotal;
  float    vBat;
  uint16_t soc;
  uint16_t pCharge;
  uint16_t pDischarge;
  float    vGrid;
  float    fGrid;
  int16_t  pGrid;
  float    vEps;
  uint16_t pEps;
  uint16_t tempRad;
};

// Cấu trúc dữ liệu sản lượng điện
struct LuxEnergyData {
  float epvToday;
  float epvTotal;
  float echgToday;
  float edisToday;
  float eepsToday;
  float eImpToday; // Mua lưới
  float eExpToday; // Bán lưới
};

class LuxpowerTCP {
public:
  LuxpowerTCP();

  // Khởi tạo thông tin kết nối
  void begin(const char* ip, const char* dongleSn, const char* inverterSn, uint16_t port = 8000);

  // API đọc thông số
  bool readRealtime(LuxRealtimeData &data);
  bool readEnergy(LuxEnergyData &data);

  // API ghi thông số / Chỉnh nạp xả
  bool writeRegister(uint16_t reg, uint16_t value);
  bool setChargeCurrent(uint8_t amps);    // Cài đặt dòng nạp (A)
  bool setDischargeCurrent(uint8_t amps); // Cài đặt dòng xả (A)

private:
  char     _ip[32];
  uint16_t _port;
  char     _dongleSn[11];
  char     _inverterSn[11];

  bool sendReadBatch(WiFiClient &client, uint16_t startReg, uint16_t count);
  bool readResponse(WiFiClient &client, uint8_t* buffer, size_t maxLen, size_t &bytesRead);
  
  uint16_t getU16(const uint8_t* buf, int offsetIdx);
  int16_t  getS16(const uint8_t* buf, int offsetIdx);
  uint32_t getU32(const uint8_t* buf, int lowRegIdx);
};

#endif

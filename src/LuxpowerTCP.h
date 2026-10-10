#ifndef LUXPOWER_TCP_H
#define LUXPOWER_TCP_H

#include <Arduino.h>
#include <WiFi.h>

// Dữ liệu đo thời gian thực (Registers 0 - 39)
struct LuxRealtimeData {
  uint16_t status;
  float    vPv1;
  float    vPv2;
  float    iPv1;
  float    iPv2;
  uint16_t pPv1;
  uint16_t pPv2;
  uint16_t pPvTotal;
  float    vBat;
  int16_t  iBat;
  uint16_t soc;
  uint16_t pCharge;
  uint16_t pDischarge;
  float    vGrid;
  float    fGrid;
  int16_t  pGrid;
  float    vEps;
  float    fEps;
  uint16_t pEps;
  uint16_t sEps;
  float    vBus1;
  float    vBus2;
  uint16_t tempInverter;
  uint16_t tempRad;
  uint16_t tempBat;
  uint16_t faultCode;
  uint16_t warningCode;
};

// Dữ liệu sản lượng điện tích lũy kWh (Registers 40 - 79)
struct LuxEnergyData {
  float epvToday;
  float epvTotal;
  float epv1Today;
  float epv2Today;
  float echgToday;
  float echgTotal;
  float edisToday;
  float edisTotal;
  float eepsToday;
  float eepsTotal;
  float eExpToday; // Đẩy lưới
  float eExpTotal;
  float eImpToday; // Mua lưới
  float eImpTotal;
};

class LuxpowerTCP {
public:
  LuxpowerTCP();

  // Khởi tạo thông tin kết nối
  void begin(const char* ip, const char* dongleSn, const char* inverterSn, uint16_t port = 8000);
  void setHost(const char* ip);
  void setDongleSN(const char* dongleSn);
  void setInverterSN(const char* inverterSn);
  
  // Quản lý chu kỳ đọc (giây)
  void     setInterval(uint32_t seconds);
  uint32_t getInterval() const;
  bool     shouldPoll();

  // Tự động quét dải mạng LAN tìm IP biến tần mở cổng 8000
  String discoverIP(uint16_t timeoutMs = 35);

  // Đọc dữ liệu từ biến tần
  bool readRealtime(LuxRealtimeData &data);
  bool readEnergy(LuxEnergyData &data);

  // Ghi cài đặt thông số xuống biến tần
  bool writeRegister(uint16_t reg, uint16_t value);
  bool setChargeCurrent(uint8_t amps);    // Đặt dòng sạc (A)
  bool setDischargeCurrent(uint8_t amps); // Đặt dòng xả (A)

private:
  char          _ip[32];
  uint16_t      _port;
  char          _dongleSn[11];
  char          _inverterSn[11];
  uint32_t      _intervalSec;
  unsigned long _lastPollTime;

  bool sendReadBatch(WiFiClient &client, uint16_t startReg, uint16_t count);
  bool readResponse(WiFiClient &client, uint8_t* buffer, size_t maxLen, size_t &bytesRead);

  uint16_t getU16(const uint8_t* buf, int offsetIdx);
  int16_t  getS16(const uint8_t* buf, int offsetIdx);
  uint32_t getU32(const uint8_t* buf, int lowRegIdx);
};

#endif

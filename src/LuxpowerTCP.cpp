#include "LuxpowerTCP.h"

LuxpowerTCP::LuxpowerTCP() : _port(8000) {
  memset(_ip, 0, sizeof(_ip));
  memset(_dongleSn, 0, sizeof(_dongleSn));
  memset(_inverterSn, 0, sizeof(_inverterSn));
}

void LuxpowerTCP::begin(const char* ip, const char* dongleSn, const char* inverterSn, uint16_t port) {
  strncpy(_ip, ip, sizeof(_ip) - 1);
  strncpy(_dongleSn, dongleSn, 10);
  strncpy(_inverterSn, inverterSn, 10);
  _port = port;
}

uint16_t LuxpowerTCP::getU16(const uint8_t* buf, int offsetIdx) {
  int pos = 30 + (offsetIdx * 2);
  return (uint16_t)(buf[pos] | (buf[pos + 1] << 8));
}

int16_t LuxpowerTCP::getS16(const uint8_t* buf, int offsetIdx) {
  int pos = 30 + (offsetIdx * 2);
  return (int16_t)(buf[pos] | (buf[pos + 1] << 8));
}

uint32_t LuxpowerTCP::getU32(const uint8_t* buf, int lowRegIdx) {
  uint32_t low  = getU16(buf, lowRegIdx);
  uint32_t high = getU16(buf, lowRegIdx + 1);
  return low | (high << 16);
}

bool LuxpowerTCP::sendReadBatch(WiFiClient &client, uint16_t startReg, uint16_t count) {
  uint8_t frame[31];
  frame[0] = 0xA1; frame[1] = 0x1A;
  frame[2] = 0x01; frame[3] = 0x00;
  frame[4] = 0x19; frame[5] = 0x00; // Payload = 25 bytes
  memcpy(&frame[6], _dongleSn, 10);
  memcpy(&frame[16], _inverterSn, 10);
  frame[26] = 0x03; // Function: Read
  frame[27] = (uint8_t)(startReg & 0xFF);
  frame[28] = (uint8_t)((startReg >> 8) & 0xFF);
  frame[29] = (uint8_t)(count & 0xFF);
  frame[30] = (uint8_t)((count >> 8) & 0xFF);

  return client.write(frame, sizeof(frame)) == sizeof(frame);
}

bool LuxpowerTCP::readResponse(WiFiClient &client, uint8_t* buffer, size_t maxLen, size_t &bytesRead) {
  unsigned long start = millis();
  while (client.available() == 0) {
    if (millis() - start > 1500) return false;
    delay(10);
  }
  bytesRead = client.read(buffer, maxLen);
  return (bytesRead >= 30 && buffer[0] == 0xA1 && buffer[1] == 0x1A);
}

bool LuxpowerTCP::readRealtime(LuxRealtimeData &data) {
  WiFiClient client;
  if (!client.connect(_ip, _port, 2000)) return false;

  if (!sendReadBatch(client, 0, 40)) {
    client.stop();
    return false;
  }

  uint8_t buf[256];
  size_t len = 0;
  if (!readResponse(client, buf, sizeof(buf), len)) {
    client.stop();
    return false;
  }
  client.stop();

  data.status     = getU16(buf, 0);
  data.vPv1       = getU16(buf, 1) * 0.1f;
  data.vPv2       = getU16(buf, 2) * 0.1f;
  data.pPv1       = getU16(buf, 7);
  data.pPv2       = getU16(buf, 8);
  data.pPvTotal   = data.pPv1 + data.pPv2;
  data.vBat       = getU16(buf, 4) * 0.1f;
  data.soc        = getU16(buf, 5);
  data.pCharge    = getU16(buf, 9);
  data.pDischarge = getU16(buf, 10);
  data.vGrid      = getU16(buf, 11) * 0.1f;
  data.fGrid      = getU16(buf, 12) * 0.01f;
  data.pGrid      = getS16(buf, 13);
  data.vEps       = getU16(buf, 15) * 0.1f;
  data.pEps       = getU16(buf, 17);
  data.tempRad    = getU16(buf, 21);

  return true;
}

bool LuxpowerTCP::readEnergy(LuxEnergyData &data) {
  WiFiClient client;
  if (!client.connect(_ip, _port, 2000)) return false;

  if (!sendReadBatch(client, 40, 40)) {
    client.stop();
    return false;
  }

  uint8_t buf[256];
  size_t len = 0;
  if (!readResponse(client, buf, sizeof(buf), len)) {
    client.stop();
    return false;
  }
  client.stop();

  data.epvTotal  = getU32(buf, 0) * 0.1f;
  data.epvToday  = getU16(buf, 4) * 0.1f;
  data.echgToday = getU16(buf, 7) * 0.1f;
  data.edisToday = getU16(buf, 10) * 0.1f;
  data.eepsToday = getU16(buf, 13) * 0.1f;
  data.eExpToday = getU16(buf, 16) * 0.1f;
  data.eImpToday = getU16(buf, 19) * 0.1f;

  return true;
}

// Ghi 1 thanh ghi đơn (Function code 0x06)
bool LuxpowerTCP::writeRegister(uint16_t reg, uint16_t value) {
  WiFiClient client;
  if (!client.connect(_ip, _port, 2000)) return false;

  uint8_t frame[31];
  frame[0] = 0xA1; frame[1] = 0x1A;
  frame[2] = 0x01; frame[3] = 0x00;
  frame[4] = 0x19; frame[5] = 0x00;
  memcpy(&frame[6], _dongleSn, 10);
  memcpy(&frame[16], _inverterSn, 10);
  frame[26] = 0x06; // Function: Write Single Register
  frame[27] = (uint8_t)(reg & 0xFF);
  frame[28] = (uint8_t)((reg >> 8) & 0xFF);
  frame[29] = (uint8_t)(value & 0xFF);
  frame[30] = (uint8_t)((value >> 8) & 0xFF);

  client.write(frame, sizeof(frame));

  uint8_t rx[64];
  size_t len = 0;
  bool ok = readResponse(client, rx, sizeof(rx), len);
  client.stop();
  return ok;
}

// Cài đặt dòng nạp (Thanh ghi cài đặt dòng sạc acquy: Reg 33)
bool LuxpowerTCP::setChargeCurrent(uint8_t amps) {
  return writeRegister(33, (uint16_t)amps);
}

// Cài đặt dòng xả (Thanh ghi cài đặt dòng xả acquy: Reg 34)
bool LuxpowerTCP::setDischargeCurrent(uint8_t amps) {
  return writeRegister(34, (uint16_t)amps);
}

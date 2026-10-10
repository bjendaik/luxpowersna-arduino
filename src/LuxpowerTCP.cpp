#include "LuxpowerTCP.h"

LuxpowerTCP::LuxpowerTCP() 
  : _port(8000), _intervalSec(5), _lastPollTime(0) {
  memset(_ip, 0, sizeof(_ip));
  memset(_dongleSn, 0, sizeof(_dongleSn));
  memset(_inverterSn, 0, sizeof(_inverterSn));
}

void LuxpowerTCP::begin(const char* ip, const char* dongleSn, const char* inverterSn, uint16_t port) {
  setHost(ip);
  setDongleSN(dongleSn);
  setInverterSN(inverterSn);
  _port = port;
}

void LuxpowerTCP::setHost(const char* ip) {
  if (ip) {
    strncpy(_ip, ip, sizeof(_ip) - 1);
    _ip[sizeof(_ip) - 1] = '\0';
  }
}

void LuxpowerTCP::setDongleSN(const char* dongleSn) {
  if (dongleSn) {
    snprintf(_dongleSn, sizeof(_dongleSn), "%-10s", dongleSn);
  }
}

void LuxpowerTCP::setInverterSN(const char* inverterSn) {
  if (inverterSn) {
    snprintf(_inverterSn, sizeof(_inverterSn), "%-10s", inverterSn);
  }
}

void LuxpowerTCP::setInterval(uint32_t seconds) {
  _intervalSec = (seconds < 1) ? 1 : seconds;
}

uint32_t LuxpowerTCP::getInterval() const {
  return _intervalSec;
}

bool LuxpowerTCP::shouldPoll() {
  unsigned long now = millis();
  if (now - _lastPollTime >= (_intervalSec * 1000UL)) {
    _lastPollTime = now;
    return true;
  }
  return false;
}

String LuxpowerTCP::discoverIP(uint16_t timeoutMs) {
  IPAddress local = WiFi.localIP();
  IPAddress scanIP = local;
  WiFiClient scanClient;

  for (int i = 2; i < 255; i++) {
    scanIP[3] = i;
    if (scanIP == local) continue;

    if (scanClient.connect(scanIP, _port, timeoutMs)) {
      scanClient.stop();
      strncpy(_ip, scanIP.toString().c_str(), sizeof(_ip) - 1);
      return scanIP.toString();
    }
  }
  return "";
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
  frame[26] = 0x03; // Lệnh đọc
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
  if (strlen(_ip) == 0 || strlen(_dongleSn) < 10 || strlen(_inverterSn) < 10) return false;

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

  data.status       = getU16(buf, 0);
  data.vPv1         = getU16(buf, 1) * 0.1f;
  data.vPv2         = getU16(buf, 2) * 0.1f;
  data.vBat         = getU16(buf, 4) * 0.1f;
  data.soc          = getU16(buf, 5);
  data.pPv1         = getU16(buf, 7);
  data.pPv2         = getU16(buf, 8);
  data.pPvTotal     = data.pPv1 + data.pPv2;
  data.pCharge      = getU16(buf, 9);
  data.pDischarge   = getU16(buf, 10);
  data.vGrid        = getU16(buf, 11) * 0.1f;
  data.fGrid        = getU16(buf, 12) * 0.01f;
  data.pGrid        = getS16(buf, 13);
  data.vEps         = getU16(buf, 15) * 0.1f;
  data.fEps         = getU16(buf, 16) * 0.01f;
  data.pEps         = getU16(buf, 17);
  data.sEps         = getU16(buf, 18);
  data.tempInverter = getU16(buf, 20);
  data.tempRad      = getU16(buf, 21);
  data.tempBat      = getU16(buf, 22);
  data.iPv1         = getU16(buf, 23) * 0.1f;
  data.iPv2         = getU16(buf, 24) * 0.1f;
  data.iBat         = getS16(buf, 25) * 0.1f;
  data.vBus1        = getU16(buf, 26) * 0.1f;
  data.vBus2        = getU16(buf, 27) * 0.1f;
  data.faultCode    = getU16(buf, 28);
  data.warningCode  = getU16(buf, 30);

  return true;
}

bool LuxpowerTCP::readEnergy(LuxEnergyData &data) {
  if (strlen(_ip) == 0 || strlen(_dongleSn) < 10 || strlen(_inverterSn) < 10) return false;

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
  data.epv1Today = getU16(buf, 5) * 0.1f;
  data.epv2Today = getU16(buf, 6) * 0.1f;
  data.echgToday = getU16(buf, 7) * 0.1f;
  data.echgTotal = getU32(buf, 8) * 0.1f;
  data.edisToday = getU16(buf, 10) * 0.1f;
  data.edisTotal = getU32(buf, 11) * 0.1f;
  data.eepsToday = getU16(buf, 13) * 0.1f;
  data.eepsTotal = getU32(buf, 14) * 0.1f;
  data.eExpToday = getU16(buf, 16) * 0.1f;
  data.eExpTotal = getU32(buf, 17) * 0.1f;
  data.eImpToday = getU16(buf, 19) * 0.1f;
  data.eImpTotal = getU32(buf, 20) * 0.1f;

  return true;
}

bool LuxpowerTCP::writeRegister(uint16_t reg, uint16_t value) {
  if (strlen(_ip) == 0 || strlen(_dongleSn) < 10 || strlen(_inverterSn) < 10) return false;

  WiFiClient client;
  if (!client.connect(_ip, _port, 2000)) return false;

  uint8_t frame[31];
  frame[0] = 0xA1; frame[1] = 0x1A;
  frame[2] = 0x01; frame[3] = 0x00;
  frame[4] = 0x19; frame[5] = 0x00;
  memcpy(&frame[6], _dongleSn, 10);
  memcpy(&frame[16], _inverterSn, 10);
  frame[26] = 0x06; // Lệnh ghi 1 thanh ghi (Write Single Register)
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

bool LuxpowerTCP::setChargeCurrent(uint8_t amps) {
  return writeRegister(33, (uint16_t)amps);
}

bool LuxpowerTCP::setDischargeCurrent(uint8_t amps) {
  return writeRegister(34, (uint16_t)amps);
}

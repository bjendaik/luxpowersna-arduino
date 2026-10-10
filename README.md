# LuxpowerTCP for ESP32

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-00979C.svg)](https://www.arduino.cc/)
[![Target: ESP32](https://img.shields.io/badge/MCU-ESP32-E7352C.svg)](https://espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

Thư viện Arduino C++ cho ESP32 giúp đọc và điều khiển biến tần **Luxpower SNA Pro** trực tiếp qua mạng LAN Wi-Fi cổng TCP 8000 (không phụ thuộc Cloud).

## Tính năng

- **Kết nối nội bộ TCP Port 8000:** Đọc trực tiếp từ Dongle Wi-Fi nội bộ, không tốn tài nguyên Cloud.
- **Tự động dò IP (`discoverIP`):** Tự quét toàn bộ mạng con LAN để tìm IP của biến tần.
- **Thông số thời gian thực:** Điện áp, dòng điện, công suất PV1, PV2, Pin (V, A, SOC %), Điện lưới AC (V, Hz, W mua/bán), Tải EPS Backup và nhiệt độ nhôm tản nhiệt.
- **Sản lượng tích lũy ($kWh$):** Đọc số điện phát trong ngày và tổng trọn đời.
- **Cài đặt tham số:** Hỗ trợ lệnh chỉnh dòng nạp (`setChargeCurrent`) và dòng xả (`setDischargeCurrent`).
- **Không phụ thuộc MQTT:** Hoạt động độc lập, nhẹ và tối ưu RAM cho ESP32.

## Cài đặt

1. Tải repository này dưới dạng **ZIP**.
2. Trong Arduino IDE: **Sketch** -> **Include Library** -> **Add .ZIP Library...**.

## Lệnh Git đẩy lên GitHub

```bash
git init
git add .
git commit -m "Initial release: Luxpower SNA TCP library for ESP32"
git branch -M main
git remote add origin [https://github.com/bjendaik/luxpowersna-arduino.git](https://github.com/bjendaik/luxpowersna-arduino.git)
git push -u origin main

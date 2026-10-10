# LuxpowerTCP for ESP32

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-00979C.svg)](https://www.arduino.cc/)
[![Target: ESP32](https://img.shields.io/badge/MCU-ESP32-E7352C.svg)](https://espressif.com/)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)

A lightweight, zero-cloud-dependency Arduino C++ library for querying and controlling **Luxpower SNA Pro** hybrid inverters directly via local Wi-Fi (Dongle TCP port 8000).

---

## Tính năng nổi bật

- **Kết nối nội bộ trực tiếp:** Giao tiếp qua mạng LAN cổng TCP 8000 của Dongle Wi-Fi, không phụ thuộc Cloud server hay Internet.
- **Trừu tượng hóa hoàn toàn:** Tự động đóng gói cấu trúc bản tin `0xA1 0x1A` và xử lý Little-Endian nội bộ.
- **Dữ liệu thời gian thực:** Điện áp, dòng điện, công suất PV1, PV2, Pin lưu trữ (SOC, V, W), Điện lưới AC (V, Hz, W mua/bán), Tải Backup EPS và nhiệt độ máy.
- **Sản lượng tích lũy ($kWh$):** Đọc sản lượng trong ngày và tổng trọn đời của PV, Nạp/Xả pin, Mua/Bán lưới và Tải tiêu thụ.
- **Hỗ trợ ghi tham số:** Cung cấp API trực tiếp thay đổi dòng sạc (`setChargeCurrent`) và dòng xả (`setDischargeCurrent`).

---

## Cài đặt

1. Tải repository này dưới dạng file **ZIP** từ GitHub.
2. Mở Arduino IDE -> chọn **Sketch** -> **Include Library** -> **Add .ZIP Library...**.
3. Chọn file ZIP vừa tải về.

Hoặc clone trực tiếp vào thư mục thư viện Arduino của bạn:
```bash
cd ~/Documents/Arduino/libraries/
git clone [https://github.com/bjendaik/luxpowersna-arduino.git](https://github.com/bjendaik/luxpowersna-arduino.git)

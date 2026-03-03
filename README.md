# RC Car Controller: Jumper PPM to ESP-NOW Bridge 🚗📻

[![ESP32-S3](https://img.shields.io/badge/Board-ESP32--S3%20Super%20Mini-blue.svg)](https://www.espressif.com/)
[![ESP8266](https://img.shields.io/badge/Board-ESP8266-black.svg)](https://www.espressif.com/)
[![Protocol](https://img.shields.io/badge/Protocol-PPM%20%26%20ESP--NOW-green.svg)]()

*(Türkçe açıklamalar aşağıdadır / Turkish description is below)*

## 🇬🇧 English

A highly optimized, zero-latency RC car control system that reads PPM signals from a standard RC Transmitter (e.g., Jumper, Radiomaster via Trainer/AUX port) using an **ESP32-S3 Super Mini**, and transmits the 8-channel data wirelessly via **ESP-NOW** to an **ESP8266** receiver. 

[cite_start]The advanced v3 receiver features an **Auto-Scanning I2C OLED Telemetry Dashboard** and multi-mode data visualization[cite: 1, 13].

### 🌟 Receiver Features (ESP8266 v3)
* **Auto I2C Pin Scanner:** The receiver automatically tests multiple SDA/SCL pin combinations to find the OLED display.
* **Headless Fallback:** If no OLED is detected, the system safely continues to operate and streams all telemetry data directly to the Serial Monitor[cite: 2, 37, 38].
* **Multi-Mode UI:** Use a physical button (D3) to switch between 3 live display modes[cite: 13, 34, 47]:
  1. **RC Mode:** Live 8-channel joystick progress bars and values.
  2. **HID Mode:** Raw debug packet data visualization[cite: 23, 24].
  3. **STATUS Mode:** System health, uptime, and ESP32 transmitter packet statistics[cite: 25].
* **Connection Failsafe:** Auto-detects signal loss and triggers a "NO SIG" warning if the connection drops for more than 2 seconds (2000ms)[cite: 26, 46].
* **Heartbeat LED:** Built-in LED blinks fast when the radio link is stable, and slow when disconnected[cite: 56].

### 🔌 Receiver Wiring (ESP8266 + OLED)
* **OLED SDA / SCL:** Connect to any standard I2C pins. [cite_start]The code will auto-detect them (e.g., `D5/D6`, `D2/D1`, `D3/D4`)[cite: 35].
* **OLED VCC & GND:** -> `3.3V` and `GND`
* **Mode Button:** -> Connect a push button between `D3` (GPIO0) and `GND`[cite: 34].

---

## 🇹🇷 Türkçe

Standart bir RC Kumandadan (Jumper, Radiomaster vb.) Trainer/AUX portu üzerinden PPM sinyali okuyan ve bu veriyi **ESP-NOW** protokolü ile gecikmesiz olarak **ESP8266** alıcısına aktaran yüksek optimizasyonlu RC kontrol sistemi. 

[cite_start]Gelişmiş v3 alıcı sürümü, **Otomatik I2C Tarayıcılı OLED Telemetri Ekranı** ve çoklu mod görselleştirme özelliklerine sahiptir[cite: 1, 13].

### 🌟 Alıcı Özellikleri (ESP8266 v3)
* **Otomatik I2C Pin Tarayıcı:** Alıcı, OLED ekranı bulmak için çeşitli SDA/SCL pin kombinasyonlarını otomatik olarak test eder ve yapılandırır.
* **OLED'siz Çalışma Desteği:** Ekran bağlanmamışsa veya bulunamazsa, sistem çökmek yerine güvenli moda geçer ve tüm telemetri verilerini Seri Monitör üzerinden akıtmaya devam eder[cite: 2, 37, 38].
* **Çoklu Ekran Modu:** `D3` pinine bağlı fiziksel bir buton ile 3 farklı canlı telemetri ekranı arasında geçiş yapılabilir[cite: 13, 34, 47]:
  1. **RC Modu:** 8 kanalın anlık joystick verileri ve ilerleme çubukları (Progress Bar).
  2. **HID Modu:** Ham veri paketlerinin (Debug) anlık dökümü[cite: 23, 24].
  3. **STATUS Modu:** Sistem sağlığı, çalışma süresi, gönderilen/hata veren paket istatistikleri[cite: 25].
* **Failsafe (Güvenlik Koruması):** İletişim 2 saniyeden (2000ms) uzun süre koparsa ekranda anında "NO SIG" (Sinyal Yok) uyarısı verir ve motorları güvenliğe alır[cite: 26, 46].
* **Durum LED'i:** Dahili mavi LED, telsiz bağlantısı kusursuzken hızlı, bağlantı koptuğunda yavaş yanıp sönerek görsel geri bildirim sağlar[cite: 56].

### 🔌 Alıcı Bağlantı Şeması (ESP8266 + OLED)
* **OLED SDA / SCL:** Herhangi bir standart I2C pinine bağlayın. [cite_start]Kod otomatik olarak bulacaktır (Örn: `D5/D6`, `D2/D1`, `D3/D4`)[cite: 35].
* **OLED VCC ve GND:** -> `3.3V` ve `GND`
* **Mod Butonu:** -> `D3` (GPIO0) pini ile `GND` arasına standart bir push buton bağlayın[cite: 34].

### 🚀 Kurulum & Kullanım / Installation & Usage
1. `Receiver_ESP8266v3.ino` kodunu ESP8266 kartınıza yükleyin ve Seri Monitörden kartın MAC adresini not edin. (Ekranda veya Seri Portta görünecektir).
2. ESP32-S3 verici kodundaki MAC dizisini bu adres ile güncelleyip yükleyin.
3. Alıcı ekranında "ESP32 bekleniyor.." yazısını gördüğünüzde ESP32 vericisine güç verin[cite: 42].
4. Kumanda kollarını hareket ettirerek OLED ekrandaki barların tepkisini izleyin. [cite_start]D3 butonuna basarak Sistem Sağlığı ve Ham Veri modlarına geçiş yapın[cite: 43].

### 📄 License
This project is open-source and available under the MIT License.

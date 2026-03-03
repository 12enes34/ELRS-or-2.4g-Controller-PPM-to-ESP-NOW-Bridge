/*
 * PROJECT: RC Transmitter (PPM to ESP-NOW)
 * BOARD: ESP32-S3 Super Mini
 */

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include "esp_wifi.h"
#include <Adafruit_NeoPixel.h>

// --- CONFIGURATION ---
#define PPM_PIN 13
#define LED_PIN 48
#define LED_COUNT 1

// REPLACE WITH YOUR ESP8266 MAC ADDRESS
uint8_t RECEIVER_MAC[6] = {0xA4, 0xCF, 0x12, 0xB2, 0xD9, 0x83};

Adafruit_NeoPixel rgb(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

typedef struct __attribute__((packed)) {
  uint8_t  type;
  uint16_t ch[8];
  uint8_t  rssi;
  uint8_t  flags;
} RCPacket_t;

RCPacket_t txRC;

volatile uint16_t channels[8];
volatile uint8_t channel_idx = 0;
volatile uint32_t last_micros = 0;
volatile bool new_data = false;
uint32_t last_ppm_time = 0;

void setLED(uint8_t r, uint8_t g, uint8_t b) {
  rgb.setPixelColor(0, rgb.Color(r, g, b));
  rgb.show();
}

void IRAM_ATTR readPPM() {
  uint32_t current_micros = micros();
  uint32_t time_diff = current_micros - last_micros;
  if (time_diff < 400) return; // Anti-jitter / Noise filter
  last_micros = current_micros;

  if (time_diff > 3000) {
    channel_idx = 0; // Sync pulse
  } 
  else if (channel_idx < 8) {
    channels[channel_idx] = time_diff;
    channel_idx++;
    if (channel_idx == 8) new_data = true;
  }
}

void onSent(const wifi_tx_info_t* mac, esp_now_send_status_t s) { }

void setup() {
  Serial.begin(115200);
  
  // HEAT PREVENTION: Downclock CPU to 80MHz
  setCpuFrequencyMhz(80); 
  delay(100);

  rgb.begin();
  rgb.setBrightness(15);
  setLED(100, 0, 150); // Booting (Purple)

  WiFi.mode(WIFI_STA);
  WiFi.setTxPower(WIFI_POWER_8_5dBm); // HEAT PREVENTION: Reduce TX Power
  WiFi.setChannel(1);
  WiFi.disconnect(false, true);
  delay(100);

  if (esp_now_init() != ESP_OK) {
    setLED(200, 0, 0); // Error (Red)
    while(1);
  }
  
  esp_now_register_send_cb(onSent);
  esp_now_peer_info_t peer;
  memset(&peer, 0, sizeof(peer));
  memcpy(peer.peer_addr, RECEIVER_MAC, 6);
  peer.channel = 1;
  peer.encrypt = false;
  
  if (esp_now_add_peer(&peer) != ESP_OK) {
    setLED(200, 0, 0); // Error (Red)
    while(1);
  }
  
  txRC.type = 0x01;
  txRC.rssi = 100;
  txRC.flags = 0x01;

  pinMode(PPM_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PPM_PIN), readPPM, RISING);
  
  setLED(200, 150, 0); // Ready, waiting for PPM (Yellow)
}

void loop() {
  if (new_data) {
    new_data = false;
    last_ppm_time = millis();

    for (int i = 0; i < 8; i++) {
      txRC.ch[i] = constrain(channels[i], 900, 2100); 
    }

    esp_now_send(RECEIVER_MAC, (uint8_t*)&txRC, sizeof(RCPacket_t));

    // LED Heartbeat (Green)
    static bool led_tog = false;
    led_tog = !led_tog;
    if (led_tog) setLED(0, 255, 0); else setLED(0, 30, 0);  

    static uint32_t last_print = 0;
    if (millis() - last_print > 100) {
      last_print = millis();
      Serial.printf("CH3(Thr):%4d | CH4(Str):%4d\n", txRC.ch[2], txRC.ch[3]);
    }
  }

  // Failsafe: No PPM signal for 500ms
  if (millis() - last_ppm_time > 500) {
    setLED(200, 150, 0); // Waiting (Yellow)
  }
  delay(1); // Relax Watchdog
}

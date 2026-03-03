/*
 * =====================================================================
 *  ESP8266 ESP-NOW ALICI v9 — OLED OTOMATİK TESPİT
 * =====================================================================
 *  OLED bağlantı noktası otomatik taranır.
 *  OLED bulunamazsa sadece Serial ile çalışmaya devam eder.
 *
 *  Dene sırasıyla:
 *    D1=SCL D2=SDA  (GPIO5, GPIO4)  ← standart
 *    D3=SDA D4=SCL  (GPIO0, GPIO2)
 *    D5=SDA D6=SCL  (GPIO14,GPIO12)
 *
 *  Kütüphaneler:
 *    → Adafruit SSD1306
 *    → Adafruit GFX Library
 * =====================================================================
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <espnow.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ============= OLED =================================================
#define SCREEN_W 128
#define SCREEN_H  64
Adafruit_SSD1306 oled(SCREEN_W, SCREEN_H, &Wire, -1);
bool oledOK = false;  // OLED bulundu mu?

// Güvenli OLED yazma makroları — oledOK false ise hiçbir şey yapmaz
#define OLED_CLEAR()      if(oledOK) oled.clearDisplay()
#define OLED_DISPLAY()    if(oledOK) oled.display()
#define OLED_PRINT(x)     if(oledOK) oled.print(x)
#define OLED_PRINTF(...)  if(oledOK) oled.printf(__VA_ARGS__)
#define OLED_CURSOR(x,y)  if(oledOK) oled.setCursor(x,y)
#define OLED_SIZE(s)      if(oledOK) oled.setTextSize(s)
#define OLED_COLOR(c)     if(oledOK) oled.setTextColor(c)

// ============= PAKET TİPLERİ ========================================
#define PKT_TYPE_RC     0x01
#define PKT_TYPE_HID    0x02
#define PKT_TYPE_STATUS 0x03

typedef struct __attribute__((packed)) {
  uint8_t  type;
  uint16_t ch[8];
  uint8_t  rssi;
  uint8_t  flags;
} RCPacket_t;

typedef struct __attribute__((packed)) {
  uint8_t type;
  uint8_t len;
  uint8_t data[16];
} HIDDebugPacket_t;

typedef struct __attribute__((packed)) {
  uint8_t  type;
  uint8_t  state;
  uint32_t uptime_sec;
  uint32_t pkt_sent;
  uint32_t pkt_fail;
  char     msg[20];
} StatusPacket_t;

// ============= GLOBAL ===============================================
RCPacket_t       rxRC;
HIDDebugPacket_t rxHID;
StatusPacket_t   rxStatus;

bool     newRC      = false;
bool     newHID     = false;
bool     newStatus  = false;
bool     connected  = false;
uint32_t pktCount   = 0;
uint32_t lastPktMs  = 0;
uint8_t  dispMode   = 0;  // 0=RC  1=HID  2=STATUS

// ============= ESP-NOW CALLBACK =====================================
void onReceive(uint8_t* mac, uint8_t* data, uint8_t len) {
  if (len < 1) return;
  pktCount++;
  lastPktMs = millis();
  connected = true;

  uint8_t t = data[0];
  if (t == PKT_TYPE_RC && len == sizeof(RCPacket_t)) {
    memcpy(&rxRC, data, sizeof(RCPacket_t));
    newRC = true;
    if (dispMode != 1) dispMode = 0;
  }
  else if (t == PKT_TYPE_HID && len == sizeof(HIDDebugPacket_t)) {
    memcpy(&rxHID, data, sizeof(HIDDebugPacket_t));
    newHID   = true;
    dispMode = 1;
  }
  else if (t == PKT_TYPE_STATUS && len == sizeof(StatusPacket_t)) {
    memcpy(&rxStatus, data, sizeof(StatusPacket_t));
    newStatus = true;
  }
}

// ============= OLED ÇIZIM FONKSİYONLARI ============================
void drawRC() {
  OLED_CLEAR();
  OLED_SIZE(1); OLED_COLOR(SSD1306_WHITE);

  OLED_CURSOR(0,0);
  OLED_PRINTF("OK #%lu Q:%d", pktCount%100000, rxRC.rssi);

  const char* nm[] = {"A","E","T","R","5","6","7","8"};
  int xs[]={0,64}, ys[]={10,18,26,34};
  for (int i=0;i<8;i++) {
    int x=xs[i/4], y=ys[i%4];
    OLED_CURSOR(x,y);
    OLED_PRINTF("%s%4d",nm[i],rxRC.ch[i]);
    if(oledOK){
      int bx=x+40, bw=22;
      int fill=constrain((int)map(rxRC.ch[i],1000,2000,0,bw),0,bw);
      oled.drawRect(bx,y,bw,6,SSD1306_WHITE);
      if(fill>0) oled.fillRect(bx,y,fill,6,SSD1306_WHITE);
    }
  }
  OLED_CURSOR(0,56);
  OLED_PRINTF("U:%d S:%d pk:%lu",
    (rxRC.flags&1)?1:0,(rxRC.flags&2)?1:0,pktCount%10000);
  OLED_DISPLAY();
}

void drawHID() {
  OLED_CLEAR();
  OLED_SIZE(1); OLED_COLOR(SSD1306_WHITE);
  OLED_CURSOR(0,0);
  OLED_PRINTF("HID len=%d #%lu", rxHID.len, pktCount%10000);
  int y=10;
  for(int row=0;row<4;row++){
    int i0=row*2, i1=row*2+1;
    OLED_CURSOR(0,y);
    if(i0<rxHID.len) OLED_PRINTF("[%2d]=%3d",i0,rxHID.data[i0]);
    OLED_CURSOR(66,y);
    if(i1<rxHID.len) OLED_PRINTF("[%2d]=%3d",i1,rxHID.data[i1]);
    y+=9;
  }
  OLED_CURSOR(0,48);
  for(int i=8;i<16&&i<rxHID.len;i++) OLED_PRINTF("%d:%d ",i,rxHID.data[i]);
  OLED_CURSOR(0,57); OLED_PRINT("D3=mod degistir");
  OLED_DISPLAY();
}

void drawStatus() {
  OLED_CLEAR();
  OLED_SIZE(1); OLED_COLOR(SSD1306_WHITE);
  const char* sn[]={"INIT","ESPNOW","SWEEP","USB_BKL","USB_OK"};
  OLED_CURSOR(0,0);  OLED_PRINT("=ESP32 DURUM=");
  OLED_CURSOR(0,12); OLED_PRINTF("State:%s",rxStatus.state<5?sn[rxStatus.state]:"?");
  OLED_CURSOR(0,22); OLED_PRINTF("Up:%lus",rxStatus.uptime_sec);
  OLED_CURSOR(0,32); OLED_PRINTF("OK:%lu FL:%lu",rxStatus.pkt_sent,rxStatus.pkt_fail);
  OLED_CURSOR(0,42); OLED_PRINTF("%s",rxStatus.msg);
  OLED_CURSOR(0,52); OLED_PRINTF("Bu:#%lu",pktCount);
  OLED_DISPLAY();
}

void drawNoSignal() {
  OLED_CLEAR();
  OLED_SIZE(2); OLED_COLOR(SSD1306_WHITE);
  OLED_CURSOR(8,10); OLED_PRINT("NO SIG");
  OLED_SIZE(1);
  OLED_CURSOR(0,36); OLED_PRINT("ESP32 calisiyor mu?");
  OLED_CURSOR(0,50); OLED_PRINTF("PKT:%lu",pktCount);
  OLED_DISPLAY();
}

// ============= I2C TARAMA ===========================================
bool tryOLED(int sda, int scl, const char* label) {
  Serial.printf("[I2C] Deneniyor: SDA=GPIO%d SCL=GPIO%d (%s)\n", sda, scl, label);
  Wire.begin(sda, scl);
  delay(50);
  for (uint8_t addr = 0x3C; addr <= 0x3D; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.printf("[I2C] 0x%02X adresinde cihaz BULUNDU!\n", addr);
      if (oled.begin(SSD1306_SWITCHCAPVCC, addr)) {
        Serial.printf("[OLED] Baslatildi! SDA=GPIO%d SCL=GPIO%d addr=0x%02X\n",
                      sda, scl, addr);
        oled.clearDisplay();
        oled.setTextSize(1);
        oled.setTextColor(SSD1306_WHITE);
        oled.setCursor(10, 20);
        oled.printf("SDA=%d SCL=%d", sda, scl);
        oled.setCursor(10, 35);
        oled.print("OLED CALIS!");
        oled.display();
        delay(1500);
        return true;
      }
    }
  }
  // Wire.end() yok ESP8266da
  delay(30);
  return false;
}

// ============= SETUP ================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== ESP8266 ALICI v9 ===");
  Serial.printf("MAC: %s\n", WiFi.macAddress().c_str());

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(0, INPUT_PULLUP);  // D3 = mod butonu

  // OLED pin kombinasyonları — sırayla dene
  struct { int sda; int scl; const char* lbl; } pins[] = {
    {4,  5,  "D2/D1 standart"},
    {0,  2,  "D3/D4"},
    {14, 12, "D5/D6"},
    {13, 15, "D7/D8"},
    {2,  14, "D4/D5"},
    {5,  4,  "D1/D2 ters"},
  };

  Serial.println("[I2C] OLED araniyor...");
  for (auto& p : pins) {
    if (tryOLED(p.sda, p.scl, p.lbl)) {
      oledOK = true;
      break;
    }
  }

  if (!oledOK) {
    Serial.println("[OLED] Hicbir pinde bulunamadi!");
    Serial.println("[OLED] OLED olmadan devam ediliyor.");
    Serial.println("[OLED] Tum veriler Serial Monitor'de gorunecek.");
    Wire.begin(4, 5);  // Varsayilan
  }

  // WiFi + ESP-NOW
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  wifi_set_channel(1);

  if (esp_now_init() != 0) {
    Serial.println("[ESP-NOW] HATA!");
    while(1) { digitalWrite(LED_BUILTIN,LOW); delay(200);
                digitalWrite(LED_BUILTIN,HIGH); delay(200); }
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(onReceive);

  // Başlangıç RC değerleri
  rxRC.type = PKT_TYPE_RC;
  for (int i=0;i<8;i++) rxRC.ch[i]=1500;
  rxRC.ch[2]=1000; rxRC.rssi=0; rxRC.flags=0;

  // Hazır ekranı
  if (oledOK) {
    oled.clearDisplay(); oled.setTextSize(1); oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(10,10); oled.print("ESP-NOW ALICI v9");
    oled.setCursor(0, 30); oled.print(WiFi.macAddress());
    oled.setCursor(0, 45); oled.print("ESP32 bekleniyor..");
    oled.display();
  }
  Serial.println("[OK] Hazir. ESP32 bekleniyor...");
  Serial.println("D3 butonu ile mod: 0=RC  1=HID  2=STATUS");
}

// ============= LOOP =================================================
void loop() {
  static uint32_t lastDispMs  = 0;
  static uint32_t lastBlinkMs = 0;
  static bool     ledSt       = false;
  static uint8_t  lastBtn     = HIGH;

  uint32_t now = millis();

  // Bağlantı timeout
  if (lastPktMs>0 && now-lastPktMs>2000) connected=false;

  // D3 butonu → mod geçişi
  uint8_t btn = digitalRead(0);
  if (btn==LOW && lastBtn==HIGH) {
    delay(50);
    if (digitalRead(0)==LOW) {
      dispMode=(dispMode+1)%3;
      Serial.printf("[BTN] Mod: %d (%s)\n",
        dispMode, dispMode==0?"RC":dispMode==1?"HID":"STATUS");
      while(digitalRead(0)==LOW) delay(10);
    }
  }
  lastBtn = btn;

  // 100ms ekran güncelle
  if (now-lastDispMs>=100) {
    lastDispMs=now;
    if (!connected)         drawNoSignal();
    else if (dispMode==1)   drawHID();
    else if (dispMode==2)   drawStatus();
    else                    drawRC();
  }

  // Serial çıktı
  if (newRC) {
    newRC=false;
    if (pktCount%50==0) {
      Serial.printf("[RC] #%lu A:%d E:%d T:%d R:%d | USB:%d\n",
        pktCount, rxRC.ch[0],rxRC.ch[1],rxRC.ch[2],rxRC.ch[3],
        rxRC.flags&1);
    }
  }
  if (newHID) {
    newHID=false;
    Serial.printf("[HID] len=%d: ", rxHID.len);
    for(int i=0;i<rxHID.len;i++) Serial.printf("%3d ",rxHID.data[i]);
    Serial.println();
  }
  if (newStatus) {
    newStatus=false;
    Serial.printf("[STATUS] state=%d up=%lus sent=%lu fail=%lu | %s\n",
      rxStatus.state, rxStatus.uptime_sec,
      rxStatus.pkt_sent, rxStatus.pkt_fail, rxStatus.msg);
  }

  // LED blink
  if (now-lastBlinkMs>=(connected?150:700)) {
    lastBlinkMs=now; ledSt=!ledSt;
    digitalWrite(LED_BUILTIN, ledSt?LOW:HIGH);
  }
}

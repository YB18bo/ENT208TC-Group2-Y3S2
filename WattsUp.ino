/*
 * Watt's Up - Dorm Energy Saving Monitor
 * ENT208TC Industry Readiness | Session 2 - Group 2
 *
 * Communication: MQTT (cross-network, no IP needed)
 *   - Publish sensor data to:  wattsup/status  (JSON, every 2s)
 *   - Subscribe commands from: wattsup/command  ("on" / "off" / "reset")
 *   - Broker: broker.emqx.io:1883 (free public MQTT)
 *
 * Dashboard: open dashboard.html in any browser, anywhere
 */

#include <M5Unified.h>
#include <WiFi.h>
#include <Wire.h>
#include <PubSubClient.h>

// ── WiFi Configuration ──────────────────────────────────────────
const char* WIFI_SSID     = "iPhone";
const char* WIFI_PASSWORD = "11223344";

// ── MQTT Broker ──────────────────────────────────────────────────
const char* MQTT_BROKER   = "broker.emqx.io";
const int   MQTT_PORT     = 1883;
const char* MQTT_USER     = "";        // EMQX public: no auth needed
const char* MQTT_PASS     = "";
const char* TOPIC_STATUS  = "wattsup/status";
const char* TOPIC_COMMAND = "wattsup/command";

// ── Pin Assignments ─────────────────────────────────────────────
const int PIN_PIR     = 33;   // PIR on Hub (verified working)
const int PIN_RELAY   = 32;   // Relay on Hub
const int PIN_CURRENT = 34;

// Dlight (BH1750) over I2C via Grove Hub → GPIO 21/22
const uint8_t BH1750_ADDR = 0x23;

// Relay: HIGH=energized(ON), LOW=released(OFF)
void relayOn()  { pinMode(PIN_RELAY, OUTPUT); digitalWrite(PIN_RELAY, HIGH); }
void relayOff() { pinMode(PIN_RELAY, OUTPUT); digitalWrite(PIN_RELAY, LOW); }

// ── Timing Constants (ms) ───────────────────────────────────────
const unsigned long IDLE_TIMEOUT         = 3000;   // 3s for testing (was 900000)
const unsigned long PENDING_GRACE        = 2000;   // 2s for testing (was 30000)
const unsigned long AUTO_RESTORE_WINDOW  =  30000;
const unsigned long DISPLAY_INTERVAL     =   1000;
const unsigned long MQTT_PUB_INTERVAL    =   2000;

// ── Power Constants ─────────────────────────────────────────────
const float MAINS_VOLTAGE    = 230.0;
const float ACS712_MV_PER_A  = 100.0;
const float ADC_REF          = 3.3;
const float ADC_MAX          = 4095.0;

// ── Display Colors ──────────────────────────────────────────────
const uint32_t COLOR_BG       = 0x111111;
const uint32_t COLOR_OK       = 0x33CCFF;
const uint32_t COLOR_ALERT    = 0xFF0000;
const uint32_t COLOR_TEXT     = 0xFFFFFF;
const uint32_t COLOR_DIM      = 0x888888;
const uint32_t COLOR_GREEN    = 0x00CC00;
const uint32_t COLOR_YELLOW   = 0xFFCC00;

// ── Global State ────────────────────────────────────────────────
unsigned long lastMotionTime      = 0;
unsigned long pendingCutoffStart  = 0;
unsigned long cutoffTime          = 0;
unsigned long lastDisplayUpdate   = 0;
unsigned long lastMqttPub         = 0;
bool isCutoff                     = false;
bool manualOverride               = false;
bool mqttConnected                = false;

float dailySaveWh   = 0.0;
float powerWatt     = 0.0;
float lux           = 0.0;
int   pirState      = 0;

// ── MQTT Client ─────────────────────────────────────────────────
WiFiClient   wifiClient;
PubSubClient mqtt(wifiClient);

// ── Setup ───────────────────────────────────────────────────────
void setup() {
  M5.begin();
  M5.Speaker.end();  // release GPIO from speaker (DAC on Basic, I2S on Core2)
  M5.Display.setRotation(0);
  M5.Display.fillScreen(COLOR_BG);

  Serial.begin(115200);
  pinMode(PIN_PIR, INPUT_PULLUP);
  pinMode(PIN_RELAY, OUTPUT);

  // 诊断: 开机3秒内切换继电器3次，听嗒嗒声
  Serial.println("=== RELAY TEST: toggling 3 times ===");
  for (int i = 0; i < 3; i++) {
    relayOn();  delay(500);
    relayOff(); delay(500);
  }
  relayOn();
  Serial.println("=== RELAY TEST done (should've heard 3 clicks) ===");

  pinMode(PIN_CURRENT, INPUT);

  // I2C + Dlight
  Wire.begin(21, 22);  // I2C on HAT header
  Wire.beginTransmission(BH1750_ADDR);
  Wire.write(0x01);   // power on
  Wire.endTransmission();
  Wire.beginTransmission(BH1750_ADDR);
  Wire.write(0x10);   // continuous high-res mode, 1lx resolution
  Wire.endTransmission();

  // Splash
  M5.Display.fillScreen(COLOR_BG);
  M5.Display.setTextColor(TFT_WHITE, COLOR_BG);
  M5.Display.setTextSize(1);
  drawCentered(100, "Watt's Up");

  // WiFi connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  M5.Display.setTextColor(COLOR_YELLOW, COLOR_BG);
  drawCentered(130, "Connecting WiFi...");

  int dots = 0;
  while (WiFi.status() != WL_CONNECTED && dots < 60) {
    delay(500);
    dots++;
    M5.Display.fillRect(0, 128, 135, 10, COLOR_BG);
    String dotStr = "";
    for (int i = 0; i < (dots % 4); i++) dotStr += ".";
    M5.Display.setCursor(60, 128);
    M5.Display.print(dotStr);
  }

  if (WiFi.status() == WL_CONNECTED) {
    M5.Display.setTextColor(COLOR_GREEN, COLOR_BG);
    drawCentered(150, WiFi.localIP().toString().c_str());
  } else {
    M5.Display.setTextColor(COLOR_ALERT, COLOR_BG);
    drawCentered(150, "WiFi Failed!");
  }

  // MQTT setup
  String clientId = "wattsup-" + WiFi.macAddress();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);
  connectMqtt();

  // Start screen
  delay(1500);
  M5.Display.fillScreen(COLOR_BG);
  lastMotionTime = millis();
}

// ── Main Loop ───────────────────────────────────────────────────
void loop() {
  M5.update();
  unsigned long now = millis();

  // ── WiFi auto-reconnect ────────────────────────────────────
  static unsigned long lastReconnectAttempt = 0;
  static int reconnectFailCount = 0;
  static bool wasConnected = true;

  bool wifiOk = (WiFi.status() == WL_CONNECTED);

  if (!wifiOk && wasConnected) {
    lastReconnectAttempt = now;
    reconnectFailCount = 0;
    M5.Display.fillScreen(COLOR_BG);
  }

  if (!wifiOk && now - lastReconnectAttempt > 10000UL * (1 + reconnectFailCount)) {
    lastReconnectAttempt = now;
    reconnectFailCount++;
    WiFi.disconnect(true);
    delay(500);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    char buf[32];
    snprintf(buf, sizeof(buf), "WiFi retry %d...", reconnectFailCount);
    M5.Display.setTextColor(COLOR_YELLOW, COLOR_BG);
    drawCentered(130, buf);

    if (reconnectFailCount > 30) reconnectFailCount = 30;
  }

  if (wifiOk && !wasConnected) {
    M5.Display.fillScreen(COLOR_BG);
    reconnectFailCount = 0;
    connectMqtt();
  }

  wasConnected = wifiOk;

  // ── MQTT keep-alive ────────────────────────────────────────
  if (wifiOk && !mqtt.connected()) {
    connectMqtt();
  }
  if (mqtt.connected()) {
    mqtt.loop();
  }
  mqttConnected = mqtt.connected();

  // ── Read sensors ──────────────────────────────────────────
  pirState  = digitalRead(PIN_PIR);
  lux       = readLight();
  powerWatt = readPower();

  // Monitor relay pin every second
  static int lastRelay = -1;
  static unsigned long lastMonitor = 0;
  if (now - lastMonitor > 1000) {
    lastMonitor = now;
    int r = digitalRead(PIN_RELAY);
    if (r != lastRelay) {
      Serial.printf("GPIO%d: %d→%d manual=%d cutoff=%d\n",
                    PIN_RELAY, lastRelay, r, manualOverride, isCutoff);
      lastRelay = r;
    }
  }

  // ── Core logic (skip if manual override is active) ────────
  if (!manualOverride) {
    static int lastPirState = -1;
    if (pirState != lastPirState) {
      Serial.printf("PIR: %d→%d idle=%lus cutoff=%d\n",
                    lastPirState, pirState, (now - lastMotionTime) / 1000, isCutoff);
      lastPirState = pirState;
    }

    if (pirState == HIGH) {
      lastMotionTime = now;

      if (isCutoff && (now - cutoffTime <= AUTO_RESTORE_WINDOW)) {
        Serial.println("AUTO: motion → relayOn()");
        relayOn();
        isCutoff = false;
        M5.Display.fillScreen(COLOR_BG);
      }
    }

    if (!isCutoff && (now - lastMotionTime >= IDLE_TIMEOUT)) {
      if (pendingCutoffStart == 0) {
        Serial.println("AUTO: idle timeout, starting grace period...");
        pendingCutoffStart = now;
      } else if (now - pendingCutoffStart >= PENDING_GRACE) {
        Serial.println("AUTO: grace over → relayOff()");
        relayOff();
        isCutoff = true;
        cutoffTime = now;
        pendingCutoffStart = 0;
        M5.Display.fillScreen(COLOR_BG);
      }
    }

    if (now - lastMotionTime < IDLE_TIMEOUT) {
      pendingCutoffStart = 0;
    }

    if (isCutoff) {
      dailySaveWh += 0.01;
    }
  }

  // ── Manual mode: enforce relay state every loop ───────────
  if (manualOverride) {
    if (isCutoff) relayOff();
    else relayOn();
  }

  // ── MQTT publish status ───────────────────────────────────
  if (mqtt.connected() && now - lastMqttPub >= MQTT_PUB_INTERVAL) {
    lastMqttPub = now;
    publishStatus();
  }

  // ── Update LCD ────────────────────────────────────────────
  if (now - lastDisplayUpdate >= DISPLAY_INTERVAL) {
    lastDisplayUpdate = now;
    drawScreen(now - lastMotionTime);
  }

  // ── Button A: exit manual mode ────────────────────────────
  if (manualOverride && M5.BtnA.wasPressed()) {
    manualOverride = false;
    lastMotionTime = millis();
    pendingCutoffStart = 0;
    if (isCutoff) {
      relayOn();
      isCutoff = false;
    }
    M5.Display.fillScreen(COLOR_BG);
  }

  delay(10);
}

// ══════════════════════════════════════════════════════════════════
//  MQTT
// ══════════════════════════════════════════════════════════════════

void connectMqtt() {
  if (WiFi.status() != WL_CONNECTED) return;

  String clientId = "wattsup-" + WiFi.macAddress();
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);

  M5.Display.setTextColor(COLOR_YELLOW, COLOR_BG);
  drawCentered(165, "MQTT connecting...");

  if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
    mqtt.subscribe(TOPIC_COMMAND);
    M5.Display.fillRect(0, 160, 135, 20, COLOR_BG);
  } else {
    M5.Display.fillRect(0, 160, 135, 20, COLOR_BG);
    M5.Display.setTextColor(COLOR_ALERT, COLOR_BG);
    drawCentered(165, "MQTT failed");
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char cmd[16] = {0};
  unsigned int len = length < 15 ? length : 15;
  memcpy(cmd, payload, len);

  // Debug: print received command
  Serial.printf("MQTT cmd received: [%s] len=%d\n", cmd, length);

  if (strcmp(cmd, "on") == 0) {
    Serial.println("→ matched 'on'");
    manualOverride = true;
    isCutoff = false;
    pendingCutoffStart = 0;
    lastMotionTime = millis();
    M5.Display.fillScreen(COLOR_BG);
  } else if (strcmp(cmd, "off") == 0) {
    Serial.println("→ matched 'off'");
    manualOverride = true;
    isCutoff = true;
    cutoffTime = millis();
    pendingCutoffStart = 0;
    M5.Display.fillScreen(COLOR_BG);
  } else if (strcmp(cmd, "reset") == 0) {
    Serial.println("→ matched 'reset'");
    manualOverride = false;
    lastMotionTime = millis();
    pendingCutoffStart = 0;
    if (isCutoff) {
      relayOn();
      isCutoff = false;
    }
    M5.Display.fillScreen(COLOR_BG);
  }
}

void publishStatus() {
  unsigned long idleSec = (millis() - lastMotionTime) / 1000;
  const char* statusStr = isCutoff ? "cutoff"
                        : (pendingCutoffStart > 0 ? "pending" : "normal");

  String json = "{";
  json += "\"occupied\":"   + String(pirState == HIGH ? "true" : "false") + ",";
  json += "\"motion\":"     + String(pirState == HIGH ? "true" : "false") + ",";
  json += "\"power\":\""    + String(isCutoff ? "off" : "on") + "\",";
  json += "\"idle_time\":"  + String(idleSec) + ",";
  json += "\"saved_wh\":"   + String(dailySaveWh, 2) + ",";
  json += "\"lux\":"        + String(lux, 1) + ",";
  json += "\"load_w\":"     + String(powerWatt, 1) + ",";
  json += "\"relay\":\""    + String(isCutoff ? "off" : "on") + "\",";
  json += "\"status\":\""   + String(statusStr) + "\",";
  json += "\"device\":\"Watt's Up\",";
  json += "\"manual\":"     + String(manualOverride ? "true" : "false") + ",";
  json += "\"uptime_ms\":"  + String(millis());
  json += "}";

  mqtt.publish(TOPIC_STATUS, json.c_str());
}

// ══════════════════════════════════════════════════════════════════
//  Sensor Reading
// ══════════════════════════════════════════════════════════════════

float readLight() {
  Wire.beginTransmission(BH1750_ADDR);
  Wire.requestFrom(BH1750_ADDR, 2);
  if (Wire.available() >= 2) {
    uint16_t raw = (Wire.read() << 8) | Wire.read();
    return raw / 1.2;
  }
  return -1;
}

float readPower() {
  long sum = 0;
  const int samples = 200;
  for (int i = 0; i < samples; i++) {
    sum += analogRead(PIN_CURRENT);
    delayMicroseconds(50);
  }
  float avgADC = (float)sum / samples;

  float voltage  = avgADC * ADC_REF / ADC_MAX;
  float currentA = (voltage - (ADC_REF / 2.0)) / (ACS712_MV_PER_A / 1000.0);

  if (currentA < 0.05) currentA = 0.0;

  return currentA * MAINS_VOLTAGE;
}

// ══════════════════════════════════════════════════════════════════
//  LCD Display
// ══════════════════════════════════════════════════════════════════

void drawScreen(unsigned long idleMs) {
  unsigned long idleSec  = idleMs / 1000;
  unsigned long graceLeft = 0;

  if (!isCutoff && pendingCutoffStart > 0) {
    unsigned long elapsed = millis() - pendingCutoffStart;
    graceLeft = (PENDING_GRACE > elapsed) ? (PENDING_GRACE - elapsed) / 1000 : 0;
  }

  // Top status bar
  uint32_t barColor = isCutoff ? COLOR_ALERT : (pirState ? COLOR_OK : COLOR_YELLOW);
  const char* barText = isCutoff ? "POWER OFF" : (pirState ? "Occupied" : "Room Empty");
  if (manualOverride) barText = "MANUAL MODE";

  M5.Display.fillRect(0, 0, 135, 28, barColor);
  M5.Display.setTextColor(TFT_WHITE, barColor);
  M5.Display.setTextSize(1);
  drawCentered(20, barText);

  M5.Display.setTextSize(1);
  char buf[16];

  drawRow(0,  40, "Relay",    isCutoff ? "OFF" : "ON",   isCutoff ? COLOR_ALERT : COLOR_GREEN);

  snprintf(buf, sizeof(buf), "%.0f W", powerWatt);
  drawRow(1,  60, "Load",     buf,                       (powerWatt > 10.0) ? COLOR_YELLOW : COLOR_DIM);

  snprintf(buf, sizeof(buf), "%.0f lx", lux);
  bool lightWaste = (pirState == LOW && lux > 100);
  drawRow(2,  80, "Light",    buf,                       lightWaste ? COLOR_YELLOW : COLOR_DIM);

  snprintf(buf, sizeof(buf), "%lus", idleSec);
  drawRow(3, 100, "Idle",     buf,                       COLOR_TEXT);

  if (graceLeft > 0) {
    snprintf(buf, sizeof(buf), "%lus", graceLeft);
    drawRow(4, 120, "Cutoff in", buf,                    COLOR_ALERT);
  } else {
    drawRow(4, 120, "Status",  isCutoff ? "CUTOFF" : "Normal", isCutoff ? COLOR_ALERT : COLOR_GREEN);
  }

  snprintf(buf, sizeof(buf), "%.2f Wh", dailySaveWh);
  drawRow(5, 140, "Saved",    buf,                       COLOR_GREEN);

  // MQTT + WiFi status (bottom line)
  M5.Display.setTextColor(COLOR_DIM, COLOR_BG);
  if (WiFi.status() == WL_CONNECTED) {
    if (mqttConnected) {
      drawCentered(180, "MQTT Online");
    } else {
      drawCentered(180, "MQTT Offline");
    }
  } else {
    drawCentered(180, "WiFi Offline");
  }

  // Light waste warning
  if (lightWaste) {
    M5.Display.setTextColor(COLOR_YELLOW, COLOR_BG);
    drawCentered(198, "! Light on in empty room");
  } else {
    M5.Display.fillRect(0, 193, 135, 16, COLOR_BG);
  }

  if (manualOverride) {
    M5.Display.setTextColor(COLOR_YELLOW, COLOR_BG);
    drawCentered(215, "[Manual] Tap to restore auto");
  }
}

// ── Display helpers ─────────────────────────────────────────────

void drawRow(int index, int y, const char* label, const char* value, uint32_t color) {
  M5.Display.fillRect(0, y, 135, 18, COLOR_BG);
  M5.Display.setTextColor(COLOR_DIM, COLOR_BG);
  M5.Display.setCursor(6, y + 3);
  M5.Display.print(label);
  M5.Display.setTextColor(color, COLOR_BG);
  int16_t tw = M5.Display.textWidth(value);
  M5.Display.setCursor(130 - tw, y + 3);
  M5.Display.print(value);
}

void drawCentered(int y, const char* text) {
  int16_t tw = M5.Display.textWidth(text);
  M5.Display.setCursor((135 - tw) / 2, y);
  M5.Display.print(text);
}

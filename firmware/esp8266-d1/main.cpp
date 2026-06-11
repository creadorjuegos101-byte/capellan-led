// Main.ino - ESP8266 D1 Firmware
// Capellán LED v1.0.0 - Complete Implementation

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <SPIFFS.h>
#include <ArduinoJSON.h>

// Include custom libraries
#include "ws2812_driver.h"
#include "timeline_executor.h"
#include "serial_handler.h"
#include "master_slave_sync.h"

// ============= CONFIGURACIÓN =============

#define LED_PIN D1                  // GPIO5
#define NUM_LEDS 30
#define DEVICE_NAME "Capellan-LED"
#define FIRMWARE_VERSION "1.0.0"

// WiFi AP
#define AP_SSID "Capellan-LED"
#define AP_PASSWORD "12345678"
#define AP_IP IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

// ============= GLOBAL OBJECTS =============

WS2812B leds(LED_PIN, NUM_LEDS);
TimelineExecutor timeline_exec(&leds);
SerialHandler serial_handler;
MasterSlaveSync sync_manager(MODE_MASTER);
SlavePool slave_pool;
WebSocketsServer webSocket(8080);

// Device state
struct {
  String mac_address;
  uint32_t uptime_ms = 0;
  bool playing = false;
  uint32_t last_update_ms = 0;
} device_state;

// ============= SETUP =============

void setup() {
  // Serial
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n\n╔════════════════════════════════════════╗");
  Serial.println("║  🎭 Capellán LED v1.0.0              ║");
  Serial.println("║  ESP8266 D1 Firmware                 ║");
  Serial.println("╚════════════════════════════════════════╝\n");

  // LED Setup
  pinMode(LED_PIN, OUTPUT);
  leds.clear();
  leds.show();
  Serial.println("✓ LED Driver initialized");

  // SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("❌ SPIFFS failed - formatting...");
    SPIFFS.format();
  } else {
    Serial.println("✓ SPIFFS initialized");
  }

  // WiFi AP
  setupWiFi();

  // WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("✓ WebSocket server started on port 8080");

  // Serial Handler
  serial_handler.on_command = handleSerialCommand;
  Serial.println("✓ Serial handler ready");

  // Device info
  device_state.mac_address = WiFi.macAddress();
  Serial.printf("✓ MAC: %s\n", device_state.mac_address.c_str());
  Serial.println("\n📡 System ready!\n");
}

// ============= MAIN LOOP =============

void loop() {
  // Update timing
  device_state.uptime_ms = millis();

  // Handle Serial
  serial_handler.update();

  // Handle WebSocket
  webSocket.loop();

  // Update Timeline
  if (device_state.playing) {
    timeline_exec.update();
    
    // Broadcast sync to slaves (every 100ms ~10 FPS for sync)
    static uint32_t last_sync = 0;
    if (millis() - last_sync > 100) {
      broadcastSync();
      last_sync = millis();
    }
  }

  // Check slave health
  sync_manager.checkSyncHealth();

  // Yield to prevent watchdog reset
  yield();
  delay(1);
}

// ============= WIFI SETUP =============

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);

  String ssid = AP_SSID;
  ssid += "_";
  ssid += device_state.mac_address.substring(12);

  if (WiFi.softAP(ssid.c_str(), AP_PASSWORD)) {
    Serial.println("✓ WiFi AP active");
    Serial.printf("  SSID: %s\n", ssid.c_str());
    Serial.printf("  Password: %s\n", AP_PASSWORD);
    Serial.printf("  IP: %s\n", AP_IP.toString().c_str());
  } else {
    Serial.println("❌ WiFi AP failed");
  }
}

// ============= WEBSOCKET EVENTS =============

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.printf("[WS %u] Connected\n", num);
      sendWebSocketMessage(num, "{\"type\":\"ready\",\"status\":\"online\"}");
      break;

    case WStype_DISCONNECTED:
      Serial.printf("[WS %u] Disconnected\n", num);
      break;

    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;
  }
}

void handleWebSocketMessage(uint8_t num, uint8_t *payload, size_t length) {
  char json_buf[512];
  strncpy(json_buf, (char*)payload, min(length, 511));
  json_buf[min(length, 511)] = '\0';

  StaticJsonDocument<256> doc;
  deserializeJson(doc, json_buf);

  String msg_type = doc["type"];

  if (msg_type == "handshake") {
    Serial.printf("[WS %u] Handshake received\n", num);
  }
  else if (msg_type == "timeline_upload") {
    Serial.println("[WS] Timeline upload started");
    // TODO: Handle large timeline upload
  }
  else if (msg_type == "playback") {
    String action = doc["action"];
    if (action == "play") {
      device_state.playing = true;
      timeline_exec.play();
      Serial.println("▶ Playback started (WebSocket)");
    }
    else if (action == "stop") {
      device_state.playing = false;
      timeline_exec.stop();
      Serial.println("⏹ Playback stopped (WebSocket)");
    }
  }
  else if (msg_type == "sync_ack") {
    uint8_t slave_id = doc["device_id"];
    slave_pool.processSlaveAck(slave_id);
  }
}

void sendWebSocketMessage(uint8_t num, const char* message) {
  webSocket.sendTXT(num, (uint8_t*)message, strlen(message));
}

// ============= SERIAL COMMAND HANDLER =============

void handleSerialCommand(uint8_t cmd, uint8_t *payload, uint16_t length) {
  switch(cmd) {
    case 0x01: // DEVICE_INFO
      handleSerialDeviceInfo();
      break;

    case 0x02: // UPLOAD_TIMELINE
      handleSerialUploadTimeline(payload, length);
      break;

    case 0x03: // PLAY
      device_state.playing = true;
      timeline_exec.play();
      serial_handler.sendAck(cmd);
      Serial.println("▶ Playback started (Serial)");
      break;

    case 0x04: // STOP
      device_state.playing = false;
      timeline_exec.stop();
      serial_handler.sendAck(cmd);
      Serial.println("⏹ Playback stopped (Serial)");
      break;

    case 0x07: // STATUS_REQUEST
      handleSerialStatus();
      break;
  }
}

void handleSerialDeviceInfo() {
  StaticJsonDocument<256> doc;
  doc["type"] = "esp8266";
  doc["version"] = FIRMWARE_VERSION;
  doc["mac"] = device_state.mac_address;
  doc["mode"] = "master";
  doc["uptime_ms"] = device_state.uptime_ms;
  doc["heap_free"] = ESP.getFreeHeap();
  doc["leds_configured"] = NUM_LEDS;

  String output;
  serializeJson(doc, output);
  serial_handler.sendJSON(output.c_str());
}

void handleSerialUploadTimeline(uint8_t *payload, uint16_t length) {
  char json_buf[1024];
  strncpy(json_buf, (char*)payload, min(length, 1023));
  json_buf[min(length, 1023)] = '\0';

  if (timeline_exec.loadFromJSON(json_buf)) {
    serial_handler.sendJSON("{\"status\":\"timeline_loaded\"}");
  } else {
    serial_handler.sendError(0x04); // Invalid format
  }
}

void handleSerialStatus() {
  StaticJsonDocument<256> doc;
  doc["status"] = device_state.playing ? "playing" : "stopped";
  doc["progress"] = timeline_exec.getProgress();
  doc["current_time_ms"] = timeline_exec.getCurrentTime();
  doc["heap_free"] = ESP.getFreeHeap();

  String output;
  serializeJson(doc, output);
  serial_handler.sendJSON(output.c_str());
}

// ============= SYNCHRONIZATION =============

void broadcastSync() {
  if (!device_state.playing) return;

  uint32_t current_frame = timeline_exec.getCurrentTime() / 16; // Aproximado
  String sync_msg = sync_manager.createSyncMessage(current_frame, 0);

  // Enviar a todos los WebSocket conectados
  for (uint8_t i = 0; i < webSocket.connectedClients(true); i++) {
    webSocket.sendTXT(i, (uint8_t*)sync_msg.c_str(), sync_msg.length());
  }

  Serial.printf("Broadcast sync: frame=%d\n", current_frame);
}

// ============= UTILITY FUNCTIONS =============

void blinkLED(uint16_t count = 3, uint16_t delay_ms = 200) {
  for (uint16_t i = 0; i < count; i++) {
    leds.fill(Colors::RED);
    leds.show();
    delay(delay_ms);
    leds.clear();
    leds.show();
    delay(delay_ms);
  }
}

void testLEDs() {
  Serial.println("\n🧪 Testing LEDs...");

  // Rojo
  leds.fill(Colors::RED);
  leds.show();
  delay(500);

  // Verde
  leds.fill(Colors::GREEN);
  leds.show();
  delay(500);

  // Azul
  leds.fill(Colors::BLUE);
  leds.show();
  delay(500);

  leds.clear();
  leds.show();
  Serial.println("✓ LED test complete\n");
}

// ============= DEBUG INFO =============

void printSystemInfo() {
  Serial.println("\n╔════════════════════════════════════════╗");
  Serial.println("║         System Information             ║");
  Serial.println("╚════════════════════════════════════════╝");
  Serial.printf("Uptime: %d ms\n", device_state.uptime_ms);
  Serial.printf("Heap Free: %d bytes\n", ESP.getFreeHeap());
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Playing: %s\n", device_state.playing ? "YES" : "NO");
  Serial.printf("Progress: %.1f%%\n", timeline_exec.getProgress() * 100);
  Serial.println();
}

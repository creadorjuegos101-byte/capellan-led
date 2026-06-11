# Firmware Base - ESP8266 D1
# Capellán LED v1.0.0

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJSON.h>
#include <SPIFFS.h>
#include <WebSocketsServer.h>

// ============= CONFIGURACIÓN =============

#define LED_PIN D1          // GPIO5
#define NUM_LEDS 30
#define BAUD_RATE 115200
#define SERIAL_TIMEOUT 5000

// WiFi
#define AP_SSID "Capellan-LED-XXXXX"
#define AP_PASSWORD "12345678"
#define AP_IP IPAddress(192, 168, 4, 1)
#define AP_SUBNET IPAddress(255, 255, 255, 0)

// ============= VARIABLES GLOBALES =============

uint8_t led_buffer[NUM_LEDS * 3];  // RGB buffer
struct {
  bool connected = false;
  bool playing = false;
  uint32_t current_frame = 0;
  uint32_t current_time_ms = 0;
} playback_state;

struct {
  String name = "ESP8266-D1";
  String mac_address;
  uint32_t uptime_ms = 0;
  uint16_t heap_free = 0;
} device_info;

WebSocketsServer webSocket = WebSocketsServer(8080);

// ============= SETUP =============

void setup() {
  Serial.begin(BAUD_RATE);
  delay(100);
  
  Serial.println("\n\n=== Capellán LED v1.0.0 ===");
  Serial.println("Iniciando ESP8266 D1...");
  
  // LED Pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("❌ Error: SPIFFS no inicializado");
  } else {
    Serial.println("✓ SPIFFS iniciado");
  }
  
  // WiFi AP
  setupWiFi();
  
  // WebSocket
  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("✓ WebSocket iniciado en ws://192.168.4.1:8080");
  
  // Info del dispositivo
  device_info.mac_address = WiFi.macAddress();
  Serial.println("✓ Setup completado!");
}

// ============= LOOP =============

void loop() {
  // Manejo de serial
  handleSerialInput();
  
  // WebSocket
  webSocket.loop();
  
  // Actualizar estado
  device_info.uptime_ms = millis();
  device_info.heap_free = ESP.getFreeHeap();
  
  // Reproducción
  if (playback_state.playing) {
    updatePlayback();
  }
  
  delay(16); // ~60 FPS
}

// ============= WIFI =============

void setupWiFi() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, AP_SUBNET);
  
  String ap_name = AP_SSID;
  ap_name.replace("XXXXX", device_info.mac_address.substring(12).c_str());
  
  if (WiFi.softAP(ap_name.c_str(), AP_PASSWORD)) {
    Serial.print("✓ WiFi AP activo: ");
    Serial.println(ap_name);
    Serial.print("  IP: ");
    Serial.println(AP_IP);
  } else {
    Serial.println("❌ Error: No se pudo crear AP");
  }
}

// ============= WEBSOCKET =============

void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_CONNECTED:
      Serial.printf("[%u] Client conectado\n", num);
      sendDeviceInfo(num);
      break;
      
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Client desconectado\n", num);
      break;
      
    case WStype_TEXT:
      handleWebSocketMessage(num, payload, length);
      break;
  }
}

void handleWebSocketMessage(uint8_t num, uint8_t * payload, size_t length) {
  // Parse JSON
  StaticJsonDocument<512> doc;
  deserializeJson(doc, payload);
  
  String type = doc["type"];
  
  if (type == "handshake") {
    sendWebSocketMessage(num, "{\"type\":\"ready\",\"status\":\"online\"}");
  }
  else if (type == "play") {
    playback_state.playing = true;
    playback_state.current_frame = 0;
    Serial.println("▶ Reproducción iniciada (WebSocket)");
  }
  else if (type == "stop") {
    playback_state.playing = false;
    Serial.println("⏹ Reproducción detenida");
  }
}

void sendWebSocketMessage(uint8_t num, const char* message) {
  webSocket.sendTXT(num, message);
}

// ============= SERIAL =============

void handleSerialInput() {
  if (Serial.available() > 0) {
    uint8_t byte = Serial.read();
    
    // Start of Frame
    if (byte == 0xAA) {
      uint8_t cmd = Serial.read();
      
      switch(cmd) {
        case 0x01: // DEVICE_INFO
          handleSerialDeviceInfo();
          break;
        case 0x03: // PLAY
          playback_state.playing = true;
          playback_state.current_frame = 0;
          Serial.println("▶ Reproducción iniciada (Serial)");
          sendSerialAck(0x03);
          break;
        case 0x04: // STOP
          playback_state.playing = false;
          Serial.println("⏹ Reproducción detenida");
          sendSerialAck(0x04);
          break;
        case 0x07: // STATUS
          handleSerialStatus();
          break;
      }
    }
  }
}

void handleSerialDeviceInfo() {
  StaticJsonDocument<256> doc;
  doc["type"] = "esp8266";
  doc["version"] = "1.0.0";
  doc["mac"] = device_info.mac_address;
  doc["mode"] = "master";
  doc["uptime_ms"] = device_info.uptime_ms;
  doc["heap_free"] = device_info.heap_free;
  
  String output;
  serializeJson(doc, output);
  Serial.println(output);
}

void handleSerialStatus() {
  StaticJsonDocument<256> doc;
  doc["status"] = playback_state.playing ? "playing" : "idle";
  doc["current_frame"] = playback_state.current_frame;
  doc["heap_free"] = device_info.heap_free;
  
  String output;
  serializeJson(doc, output);
  Serial.println(output);
}

void sendSerialAck(uint8_t cmd) {
  Serial.printf("ACK: 0x%02X\n", cmd);
}

// ============= REPRODUCCIÓN =============

void updatePlayback() {
  playback_state.current_time_ms += 16; // ~60 FPS
  
  // Simulación: Cambiar color cada 500ms
  static uint32_t last_color_change = 0;
  if (millis() - last_color_change > 500) {
    updateLEDColor();
    last_color_change = millis();
  }
}

void updateLEDColor() {
  // Colores de ejemplo: Rojo → Verde → Azul
  static uint8_t color_index = 0;
  
  uint8_t r = 0, g = 0, b = 0;
  
  switch(color_index % 3) {
    case 0: r = 255; break; // Rojo
    case 1: g = 255; break; // Verde
    case 2: b = 255; break; // Azul
  }
  
  // Llenar buffer
  for (int i = 0; i < NUM_LEDS; i++) {
    led_buffer[i * 3 + 0] = r;
    led_buffer[i * 3 + 1] = g;
    led_buffer[i * 3 + 2] = b;
  }
  
  // Enviar a LEDs
  writeLEDs();
  
  color_index++;
}

void writeLEDs() {
  // TODO: Implementar driver WS2812B
  // Esto es un placeholder
  Serial.printf("LED Update: LED_PIN=%d, Buffer=%d bytes\n", LED_PIN, NUM_LEDS * 3);
}

// ============= INFO =============

void printSystemInfo() {
  Serial.println("\n=== System Info ===");
  Serial.printf("Uptime: %d ms\n", device_info.uptime_ms);
  Serial.printf("Heap Free: %d bytes\n", device_info.heap_free);
  Serial.printf("Flash Size: %d bytes\n", ESP.getFlashChipSize());
  Serial.printf("CPU Freq: %d MHz\n", ESP.getCpuFreqMHz());
}

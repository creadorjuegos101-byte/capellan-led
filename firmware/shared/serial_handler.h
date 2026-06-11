// USB Serial Handler - Comunicación con web app
// Capellán LED v1.0.0

#pragma once

#include <Arduino.h>

// ============= PROTOCOLO SERIAL =============

#define SERIAL_SOF 0xAA              // Start of Frame
#define SERIAL_BAUD 115200
#define SERIAL_TIMEOUT 5000

#define CMD_DEVICE_INFO 0x01
#define CMD_UPLOAD_TIMELINE 0x02
#define CMD_PLAY 0x03
#define CMD_STOP 0x04
#define CMD_SYNC_DATA 0x05
#define CMD_CONFIG 0x06
#define CMD_STATUS_REQUEST 0x07

// ============= CLASE SERIAL HANDLER =============

class SerialHandler {
private:
  uint8_t rx_buffer[1024];
  uint16_t rx_index;
  
  // CRC8 Implementation
  uint8_t calculateCRC8(uint8_t* data, uint16_t length) {
    uint8_t crc = 0;
    for (uint16_t i = 0; i < length; i++) {
      crc ^= data[i];
      for (uint8_t j = 0; j < 8; j++) {
        if (crc & 0x80) {
          crc = (crc << 1) ^ 0x07;
        } else {
          crc = crc << 1;
        }
      }
    }
    return crc;
  }

public:
  typedef void (*CommandCallback)(uint8_t cmd, uint8_t* payload, uint16_t length);
  CommandCallback on_command;
  
  SerialHandler() : rx_index(0), on_command(nullptr) {
    Serial.begin(SERIAL_BAUD);
  }
  
  // Procesar datos recibidos
  void update() {
    while (Serial.available() > 0) {
      uint8_t byte = Serial.read();
      
      // Buscar Start of Frame
      if (byte == SERIAL_SOF && rx_index == 0) {
        rx_buffer[rx_index++] = byte;
      }
      else if (rx_index > 0) {
        rx_buffer[rx_index++] = byte;
        
        // Validar frame completo
        if (rx_index >= 4) {
          uint8_t cmd = rx_buffer[1];
          uint16_t length = (rx_buffer[2] << 8) | rx_buffer[3];
          
          uint16_t frame_size = 5 + length; // SOF + CMD + LEN + PAYLOAD + CRC
          
          if (rx_index >= frame_size) {
            handleFrame();
            rx_index = 0;
          }
        }
        
        // Overflow protection
        if (rx_index >= 1024) {
          rx_index = 0;
        }
      }
    }
  }
  
private:
  void handleFrame() {
    uint8_t cmd = rx_buffer[1];
    uint16_t length = (rx_buffer[2] << 8) | rx_buffer[3];
    uint8_t* payload = rx_buffer + 4;
    uint8_t crc = rx_buffer[4 + length];
    
    // Verificar CRC
    uint8_t calculated_crc = calculateCRC8(rx_buffer, 4 + length);
    if (calculated_crc != crc) {
      sendError(0x01); // CRC Error
      return;
    }
    
    // Llamar callback
    if (on_command) {
      on_command(cmd, payload, length);
    }
  }

public:
  // Enviar frame
  void sendFrame(uint8_t cmd, uint8_t* payload, uint16_t length) {
    Serial.write(SERIAL_SOF);
    Serial.write(cmd);
    Serial.write((length >> 8) & 0xFF);
    Serial.write(length & 0xFF);
    
    if (payload && length > 0) {
      Serial.write(payload, length);
    }
    
    uint8_t frame_data[4 + length];
    frame_data[0] = cmd;
    frame_data[1] = (length >> 8) & 0xFF;
    frame_data[2] = length & 0xFF;
    if (payload && length > 0) {
      memcpy(frame_data + 3, payload, length);
    }
    
    uint8_t crc = calculateCRC8(frame_data, 3 + length);
    Serial.write(crc);
  }
  
  // Enviar JSON como string
  void sendJSON(const char* json_str) {
    uint16_t length = strlen(json_str);
    sendFrame(0xFF, (uint8_t*)json_str, length);
  }
  
  // Enviar ACK
  void sendAck(uint8_t cmd) {
    uint8_t ack_data[2] = {cmd, 0x00};
    sendFrame(0xF0, ack_data, 2);
  }
  
  // Enviar error
  void sendError(uint8_t error_code) {
    sendFrame(0xF1, &error_code, 1);
  }
  
  // Helpers para enviar datos comunes
  void sendDeviceInfo(const char* device_type, const char* mac, uint32_t uptime) {
    StaticJsonDocument<256> doc;
    doc["type"] = device_type;
    doc["mac"] = mac;
    doc["uptime_ms"] = uptime;
    doc["heap_free"] = ESP.getFreeHeap();
    
    String output;
    serializeJson(doc, output);
    sendJSON(output.c_str());
  }
  
  void sendStatus(bool playing, uint32_t current_frame, uint32_t total_frames) {
    StaticJsonDocument<256> doc;
    doc["status"] = playing ? "playing" : "stopped";
    doc["current_frame"] = current_frame;
    doc["total_frames"] = total_frames;
    doc["heap_free"] = ESP.getFreeHeap();
    
    String output;
    serializeJson(doc, output);
    sendJSON(output.c_str());
  }
};

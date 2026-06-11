// Master/Slave Synchronization Protocol
// Capellán LED v1.0.0

#pragma once

#include <Arduino.h>
#include <ArduinoJSON.h>

// ============= MODOS DE DISPOSITIVO =============

enum DeviceMode {
  MODE_STANDALONE,  // Sin sincronización
  MODE_MASTER,      // Principal - envía sync
  MODE_SLAVE,       // Esclavo - recibe sync
};

// ============= ESTRUCTURA DE SYNC =============

struct SyncMessage {
  uint32_t frame_id;           // ID del frame actual
  uint32_t timestamp_ms;       // Timestamp en ms
  uint16_t device_id;          // ID del dispositivo
  uint8_t sync_count;          // Contador de sincronizaciones
  int8_t latency_ms;           // Latencia medida
};

// ============= CLASE MASTER/SLAVE =============

class MasterSlaveSync {
private:
  DeviceMode mode;
  uint32_t master_uptime;
  uint32_t slave_start_time;
  int32_t time_offset;
  
  // Slave state
  uint32_t last_sync_time;
  uint32_t last_frame_id;
  bool is_synced;
  uint8_t sync_failures;
  const uint8_t MAX_SYNC_FAILURES = 3;
  
  // Master state
  uint32_t next_slave_check;
  const uint32_t SLAVE_CHECK_INTERVAL = 5000; // 5 segundos

public:
  MasterSlaveSync(DeviceMode _mode = MODE_STANDALONE)
    : mode(_mode), master_uptime(0), slave_start_time(0),
      time_offset(0), last_sync_time(0), last_frame_id(0),
      is_synced(false), sync_failures(0), next_slave_check(0) {}

  // ============= MASTER FUNCTIONS =============

  // Crear mensaje de sincronización
  String createSyncMessage(uint32_t frame_id, uint16_t device_id) {
    StaticJsonDocument<128> doc;
    doc["type"] = "sync";
    doc["frame_id"] = frame_id;
    doc["timestamp_ms"] = millis();
    doc["device_id"] = device_id;
    doc["sync_count"] = frame_id % 256;

    String output;
    serializeJson(doc, output);
    return output;
  }

  // Procesar ACK de slave
  bool processSyncAck(const char* json_data) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, json_data);

    uint32_t received_frame_id = doc["frame_id"];
    int8_t latency = doc["latency_ms"];

    Serial.printf("Sync ACK: Frame=%d, Latency=%dms\n", received_frame_id, latency);

    // Validar latencia
    if (latency > 100) {
      Serial.println("⚠ High latency detected!");
      return false;
    }

    return true;
  }

  // ============= SLAVE FUNCTIONS =============

  // Procesar mensaje de sincronización del master
  bool processMasterSync(const char* json_data) {
    StaticJsonDocument<128> doc;
    deserializeJson(doc, json_data);

    uint32_t master_frame_id = doc["frame_id"];
    uint32_t master_timestamp = doc["timestamp_ms"];

    // Calcular latencia
    uint32_t current_time = millis();
    int32_t latency = current_time - master_timestamp;

    Serial.printf("Sync RX: Frame=%d, Latency=%ldms\n", master_frame_id, latency);

    // Si es el primer sync
    if (!is_synced) {
      slave_start_time = current_time;
      time_offset = master_timestamp - current_time;
      is_synced = true;
      sync_failures = 0;
      Serial.println("✓ Slave sincronizado!");
    } else {
      // Verificar drift
      int32_t time_drift = (current_time + time_offset) - master_timestamp;
      if (abs(time_drift) > 100) {
        Serial.printf("⚠ Time drift: %ldms, ajustando...\n", time_drift);
        time_offset += time_drift / 2; // Corrección gradual
      }
      sync_failures = 0;
    }

    last_sync_time = current_time;
    last_frame_id = master_frame_id;

    return true;
  }

  // Crear ACK para master
  String createSyncAck(uint32_t frame_id, int8_t latency) {
    StaticJsonDocument<128> doc;
    doc["type"] = "sync_ack";
    doc["device_id"] = 1; // TODO: obtener ID real
    doc["frame_id"] = frame_id;
    doc["latency_ms"] = latency;
    doc["status"] = "synced";

    String output;
    serializeJson(doc, output);
    return output;
  }

  // Detectar pérdida de sincronización
  bool checkSyncHealth() {
    if (mode != MODE_SLAVE) return true;

    uint32_t time_since_sync = millis() - last_sync_time;

    if (time_since_sync > 1000) { // Sin sync por más de 1s
      sync_failures++;
      
      if (sync_failures >= MAX_SYNC_FAILURES) {
        is_synced = false;
        Serial.println("✗ Sync lost! Waiting for master...");
        return false;
      }
    }

    return is_synced;
  }

  // ============= GETTERS =============

  DeviceMode getMode() const { return mode; }
  void setMode(DeviceMode _mode) { mode = _mode; }

  bool isSynced() const { return is_synced; }
  uint32_t getLastFrameId() const { return last_frame_id; }

  int32_t getTimeOffset() const { return time_offset; }

  // Obtener tiempo sincronizado (para slave)
  uint32_t getSyncedTime() const {
    if (is_synced && mode == MODE_SLAVE) {
      return millis() + time_offset;
    }
    return millis();
  }

  // ============= DEBUG =============

  void printStatus() {
    Serial.println("\n=== Master/Slave Status ===");
    Serial.printf("Mode: %s\n", 
      mode == MODE_MASTER ? "MASTER" : 
      mode == MODE_SLAVE ? "SLAVE" : "STANDALONE");
    
    if (mode == MODE_SLAVE) {
      Serial.printf("Synced: %s\n", is_synced ? "YES" : "NO");
      Serial.printf("Last Frame: %d\n", last_frame_id);
      Serial.printf("Time Offset: %ldms\n", time_offset);
      Serial.printf("Sync Failures: %d\n", sync_failures);
    }
  }
};

// ============= POOL DE SLAVES (para Master) =============

struct SlaveDevice {
  uint8_t id;
  String ip;
  uint32_t last_ack_time;
  bool connected;
  uint8_t consecutive_failures;
};

class SlavePool {
private:
  SlaveDevice slaves[10];
  uint8_t slave_count;
  const uint8_t MAX_SLAVES = 10;
  const uint8_t MAX_FAILURES = 3;

public:
  SlavePool() : slave_count(0) {}

  // Agregar slave
  bool addSlave(uint8_t id, const String& ip) {
    if (slave_count >= MAX_SLAVES) {
      return false;
    }

    slaves[slave_count] = {
      id,
      ip,
      millis(),
      false,
      0
    };
    slave_count++;
    Serial.printf("Slave added: ID=%d, IP=%s\n", id, ip.c_str());
    return true;
  }

  // Remover slave
  void removeSlave(uint8_t id) {
    for (int i = 0; i < slave_count; i++) {
      if (slaves[i].id == id) {
        slaves[i] = slaves[slave_count - 1];
        slave_count--;
        Serial.printf("Slave removed: ID=%d\n", id);
        break;
      }
    }
  }

  // Procesar ACK
  void processSlaveAck(uint8_t id) {
    SlaveDevice* slave = findSlave(id);
    if (slave) {
      slave->last_ack_time = millis();
      slave->consecutive_failures = 0;
      if (!slave->connected) {
        slave->connected = true;
        Serial.printf("✓ Slave %d connected\n", id);
      }
    }
  }

  // Registrar fallo
  void recordFailure(uint8_t id) {
    SlaveDevice* slave = findSlave(id);
    if (slave) {
      slave->consecutive_failures++;
      if (slave->consecutive_failures >= MAX_FAILURES) {
        slave->connected = false;
        Serial.printf("✗ Slave %d disconnected (failures=%d)\n", id, slave->consecutive_failures);
      }
    }
  }

  // Obtener info
  uint8_t getSlaveCount() const { return slave_count; }
  SlaveDevice* getSlave(uint8_t index) {
    return index < slave_count ? &slaves[index] : nullptr;
  }

private:
  SlaveDevice* findSlave(uint8_t id) {
    for (int i = 0; i < slave_count; i++) {
      if (slaves[i].id == id) return &slaves[i];
    }
    return nullptr;
  }
};

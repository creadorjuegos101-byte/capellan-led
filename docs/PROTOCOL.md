# 🔌 Especificación de Protocolos - Capellán LED

## 1. Protocolo Serial USB

### Características
- **Baud Rate:** 115200 bps
- **Data Bits:** 8
- **Stop Bits:** 1
- **Parity:** None
- **Flow Control:** None
- **Timeout:** 5 segundos

### Frame Format

```
┌─────┬────────┬────────┬──────────────┬─────────┐
│ SOF │  CMD   │ LENGTH │   PAYLOAD    │  CRC8   │
└─────┴────────┴────────┴──────────────┴─────────┘
  1B    1B       2B        0-1024 B       1B

SOF: 0xAA (Start of Frame)
CRC8: Fletcher checksum
```

### Comandos Implementados

#### 0x01: DEVICE_INFO (Request)
**Request:**
```
AA 01 00 00 CRC
```

**Response:**
```json
{
  "type": "esp8266",
  "version": "1.0.0",
  "mac": "AA:BB:CC:DD:EE:FF",
  "rssi": -45,
  "ip": "192.168.4.1",
  "mode": "master",
  "uptime_ms": 3600000,
  "flash_size": 4194304,
  "free_heap": 32768
}
```

#### 0x02: UPLOAD_TIMELINE (Request)
**Request:**
```
AA 02 [LENGTH-2B] [JSON-TIMELINE] CRC

Payload: Compressed JSON timeline (gzip)
```

**Response (durante transferencia):**
```json
{
  "status": "uploading",
  "progress": 45,
  "bytes_received": 45600
}
```

**Response (final):**
```json
{
  "status": "complete",
  "timeline_id": "route_001",
  "duration_ms": 120000
}
```

#### 0x03: PLAY (Request)
**Request:**
```json
{
  "timeline_id": "route_001",
  "loop": true,
  "speed": 100
}
```

**Response:**
```json
{
  "status": "playing",
  "current_frame": 0,
  "fps": 60,
  "duration_ms": 120000
}
```

#### 0x04: STOP (Request)
**Request:**
```
AA 04 00 00 CRC
```

**Response:**
```json
{
  "status": "stopped",
  "current_frame": 1250
}
```

#### 0x05: SYNC_DATA (Bidirectional)
**Master → Slave:**
```json
{
  "frame_id": 1250,
  "timestamp_ms": 20833,
  "effect_id": "pulse_001",
  "colors": ["#FF0000", "#00FF00"],
  "intensity": 255
}
```

**Slave → Master (ACK):**
```json
{
  "status": "synced",
  "device_id": "slave_1",
  "frame_id": 1250
}
```

#### 0x06: CONFIG (Request)
**Request:**
```json
{
  "led_type": "ws2812b",
  "num_leds": 30,
  "pin": 5,
  "brightness": 255,
  "prop_type": "poi",
  "mode": "master"
}
```

**Response:**
```json
{
  "status": "configured",
  "config_saved": true
}
```

#### 0x07: STATUS_REQUEST (Request)
**Request:**
```
AA 07 00 00 CRC
```

**Response:**
```json
{
  "status": "ready",
  "current_timeline": "route_001",
  "playback_state": "playing",
  "current_frame": 1250,
  "fps": 60,
  "memory_used": 786432,
  "memory_total": 1048576,
  "temperature": 42,
  "last_update_ms": 16
}
```

### CRC8 Implementation
```cpp
uint8_t crc8(uint8_t* data, uint16_t length) {
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
```

---

## 2. Protocolo WebSocket (WiFi)

### Conexión
```
ws://192.168.4.1:8080/api/devices
wss://192.168.4.1:8080/api/devices (con certificado)
```

### Handshake

**Client → Server:**
```json
{
  "type": "handshake",
  "client_id": "web_app_instance_001",
  "version": "1.0.0",
  "mode": "controller"
}
```

**Server → Client:**
```json
{
  "type": "ready",
  "server_id": "capellan_master_001",
  "version": "1.0.0",
  "timestamp_ms": 1623456789
}
```

### Message Types

#### Timeline Upload
**Request:**
```json
{
  "type": "timeline_upload",
  "id": "route_001",
  "name": "Mi Rutina Épica",
  "data": {...},
  "compression": "gzip"
}
```

**Response:**
```json
{
  "type": "ack",
  "id": "route_001",
  "status": "received",
  "storage_used": 45600
}
```

#### Playback Control
**Request:**
```json
{
  "type": "playback",
  "action": "play",
  "timeline_id": "route_001",
  "loop": true,
  "speed": 100
}
```

**Response (Continuous):**
```json
{
  "type": "playback_status",
  "state": "playing",
  "current_frame": 1250,
  "fps": 60,
  "elapsed_ms": 20833
}
```

#### Master/Slave Sync
**Master → All Slaves (Broadcast):**
```json
{
  "type": "sync",
  "frame_id": 1250,
  "timestamp_ms": 20833,
  "expected_latency_ms": 50
}
```

**Slave → Master (ACK):**
```json
{
  "type": "sync_ack",
  "device_id": "slave_1",
  "frame_id": 1250,
  "latency_ms": 12
}
```

#### Device Discovery
**Request:**
```json
{
  "type": "discover",
  "timeout_ms": 5000
}
```

**Response (Multicast):**
```json
{
  "type": "device_found",
  "device_id": "slave_2",
  "device_name": "POI Left",
  "mode": "slave",
  "ip": "192.168.4.50",
  "signal_strength": -45
}
```

---

## 3. Protocolo Master/Slave

### Arquitectura

```
MASTER (1)
├─ Lee timeline + música
├─ Calcula posición frame
├─ Procesa efectos
├─ Transmite sync cada 16.67ms (60 FPS)
└─ Controla LEDs locales

        ↓ WiFi WebSocket (UDP para baja latencia)
        
SLAVE 1-10
├─ Recibe frame_id + timestamp
├─ Busca frame en timeline local
├─ Aplica mismo efecto
├─ Actualiza LEDs
└─ Reporta latencia
```

### Tolerancias
- **Latencia máxima:** 50ms
- **Drift máximo:** 100ms antes de resync
- **Timeout de conexión:** 5 segundos
- **Reintentos:** 3 intentos, exponential backoff

### Estado de Sincronización
```
IDLE → SYNCING → SYNCED → PLAYING → ERROR
```

---

## 4. Formato de Timeline JSON

Ver **[docs/TIMELINE-FORMAT.md](TIMELINE-FORMAT.md)**

---

## 5. Velocidades de Transferencia

| Tipo | Conexión | Velocidad | Latencia |
|------|----------|-----------|----------|
| USB | Serial 115200 | ~14 KB/s | <5ms |
| WiFi | 802.11n | ~500 KB/s | <100ms |
| Sync | WebSocket | Real-time | <50ms |

---

## 6. Seguridad

### Validación
- **CRC8** en todos los frames USB
- **HMAC-SHA256** en comunicación WiFi (opcional)
- **Timeout** en todas las transacciones
- **Límites de tamaño** en payloads

### Rate Limiting
- **Máximo 100 timelines** por dispositivo
- **Máximo 1000 frames** por timeline
- **Máximo 50 MB** de almacenamiento total

---

## 7. Error Codes

| Código | Significado | Acción |
|--------|------------|--------|
| 0x00 | OK | Continuar |
| 0x01 | CRC Error | Reintentar |
| 0x02 | Timeout | Reconectar |
| 0x03 | Memory Error | Liberar espacio |
| 0x04 | Invalid Format | Validar datos |
| 0x05 | Device Busy | Reintentar |

---

**Última actualización:** Junio 2026 | v1.0.0

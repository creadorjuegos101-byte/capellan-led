# 🏗️ Arquitectura de Capellán LED

## Diagrama General del Sistema

```
┌─────────────────────────────────────────────────────────────────┐
│                        CAPA PRESENTACIÓN                         │
├─────────────────────────────────────────────────────────────────┤
│  ┌──────────────────────┐        ┌──────────────────────┐       │
│  │  Web App             │        │  Mobile App (APK)    │       │
│  │  React + Vite        │        │  Flutter             │       │
│  │  - Timeline Editor   │        │  - Control UI        │       │
│  │  - Visualización 3D  │        │  - Live Preview      │       │
│  │  - Dashboard         │        │  - Sync Status       │       │
│  └──────┬───────────────┘        └──────────┬───────────┘       │
│         │                                   │                    │
└─────────┼───────────────────────────────────┼────────────────────┘
          │                                   │
          ├─────────────────┬─────────────────┤
          │                 │                 │
┌─────────▼──────┐  ┌──────▼────────┐  ┌────▼──────────┐
│  USB Serial    │  │  WiFi WebSocket   WiFi Direct   │
│  (CH340)       │  │  ws://ip:8080     192.168.4.1   │
└─────────┬──────┘  └──────┬────────┘  └────┬──────────┘
          │                │             │
          ├────────────────┼─────────────┤
          │                │             │
┌─────────▼────────────────▼─────────────▼──────────────┐
│           CAPA COMUNICACIÓN (Protocolos)              │
└─────────┬──────────────────────────────────────────┬─┘
          │                          MASTER           │
          │                            │              │
┌─────────▼──────────────────┐  ┌──────▼──────────┐ │
│   FIRMWARE PRINCIPAL       │  │  FIRMWARE SLAVE │ │
├─────────────────────────────┤  ├─────────────────┤ │
│  - Driver WS2812           │  │ - Sync Thread   │ │
│  - USB Handler             │  │ - Local playback│ │
│  - WiFi Manager            │  │ - Report status │ │
│  - Timeline Executor       │  └─────────────────┘ │
│  - Effect Engine           │                       │
│  - Music Synchronizer      │      (hasta 10)       │
│  - Storage Manager         │                       │
│  - OTA Update Handler      │                        │
└─────────┬──────────────────┘                        │
          │                                           │
┌─────────▼────────────────────────────────────────────┤
│             CAPA HARDWARE                             │
├─────────────────────────────────────────────────────┤
│  ┌─────────────────┐  ┌──────────────────────┐   │
│  │  Microcontroller│  │  LED Strip           │   │
│  │  - ESP8266 D1   │  │  - WS2812B/NeoPixel  │   │
│  │  - ESP32 C3     │  │  - APA102            │   │
│  │  - ESP32 WROOM  │  │  - Custom drivers    │   │
│  └─────────────────┘  └──────────────────────┘   │
│  ┌─────────────────────────────────────────────┐  │
│  │  Almacenamiento                             │  │
│  │  - SPIFFS (Flash Storage)                   │  │
│  │  - EEPROM (Configuración)                   │  │
│  │  - SD Card opcional                         │  │
│  └─────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘
```

## Stack Tecnológico

### Frontend
- **Web**: React 18 + Vite + TypeScript
- **Mobile**: Flutter 3.10+
- **UI**: TailwindCSS + Material Design

### Backend/Firmware
- **Lenguaje**: C++ (Arduino Framework)
- **Firmware Base**: ESP-IDF / Arduino Core
- **Build**: PlatformIO + Arduino IDE

### Comunicación
- **USB**: Serial Protocol (FTDI/CH340)
- **WiFi**: WebSocket + REST API
- **Sincronización**: Custom Protocol

### Storage
- **Cliente**: IndexedDB (Web), Local Storage (Mobile)
- **Dispositivo**: SPIFFS, EEPROM, SD Card

## Motor de Efectos

```javascript
Efectos Disponibles:
├── solid       - Color sólido
├── pulse       - Pulso rítmico
├── chase       - Persecución de LEDs
├── rainbow     - Gradiente arcoíris
├── strobe      - Destello
├── wave        - Onda
├── gradient    - Gradiente lineal
├── fade        - Desvanecimiento
├── sparkle     - Chispa aleatoria
├── matrix      - Efecto matrix
├── fire        - Simulación de fuego
└── breathing   - Respiración
```

## Flujo de Datos

```
Timeline Editor
    ↓
JSON Export
    ↓
Compresión + Validación
    ↓
USB/WiFi Transfer
    ↓
Firmware Parse
    ↓
SPIFFS Storage
    ↓
Playback Engine
    ↓
Effect Processor
    ↓
LED Driver
    ↓
WS2812 Output
```

## Protocolo Master/Slave

```
MASTER
├─ Carga timeline
├─ Inicia reproducción
├─ Detecta BPM de música
├─ Calcula frame actual
└─ Broadcast sync @ 60 FPS

        ↓ WebSocket
        
SLAVES (hasta 10)
├─ Reciben frame_id + timestamp
├─ Aplican mismo timeline localmente
├─ Actualizan LEDs sincronizados
└─ Reportan status
```

---

**Última actualización:** Junio 2026

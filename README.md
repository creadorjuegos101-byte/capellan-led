# 🎭 Capellán LED - Sistema de Control Inteligente para Props de Flow Arts

**Versión:** 1.0.0  
**Estado:** En Desarrollo 🚀

## 📋 Descripción General

Capellán LED es una plataforma completa y profesional para controlar propiedades LED de Flow Arts (poi, staff, trajes LED, levid wand) mediante timeline sincronizado con música MP3.

### Características Principales

✨ **Timeline Editor Avanzado**
- Interfaz visual con numeración de tiempos
- Sincronización automática con música MP3
- Visualización en tiempo real de animaciones
- Edición de colores y efectos por fotograma

🔌 **Conectividad Múltiple**
- USB directo (sin necesidad de IP)
- WiFi integrado
- Sistema Master & Slave para múltiples dispositivos
- Sincronización en tiempo real

📱 **Multiplataforma**
- Aplicación Web (React/Vite)
- App APK Android nativa
- Firmware para microcontroladores

🎛️ **Dispositivos Compatibles**
- ESP8266 D1
- ESP32 C3
- ESP32 WROOM-32
- Instalación vía Arduino IDE y PlatformIO

🌈 **Soporte LED**
- WS2812B (NeoPixel)
- APA102
- Configuración flexible de pines

## 📁 Estructura del Proyecto

```
capellan-led/
├── firmware/                    # Código para microcontroladores
├── web-app/                     # Aplicación Web (React + Vite)
├── mobile-app/                  # App Android (Flutter)
├── docs/                        # Documentación
├── shared/                      # Código compartido
└── tests/                       # Tests e integración
```

## 🚀 Quick Start

Ver [docs/SETUP.md](docs/SETUP.md) para guía de instalación completa.

## 📚 Documentación

- [Arquitectura del Sistema](docs/ARCHITECTURE.md)
- [Protocolo de Comunicación](docs/PROTOCOL.md)
- [Formato de Timeline](docs/TIMELINE-FORMAT.md)
- [Guía de Desarrollo](docs/DEVELOPMENT.md)

## 📈 Roadmap

Ver [ROADMAP.md](ROADMAP.md)

## 🤝 Contribución

Las contribuciones son bienvenidas. Ver [CONTRIBUTING.md](CONTRIBUTING.md)

## 📄 Licencia

MIT License

---

**Hecho con ❤️ para la comunidad de Flow Arts** 🎭✨

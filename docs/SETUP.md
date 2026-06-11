# 📋 Guía de Instalación - Capellán LED

## Requisitos Previos

### General
- Git instalado
- 500MB de espacio en disco

### Para Firmware
- Python 3.8+ 
- Arduino IDE 1.8.19+ O PlatformIO CLI
- Drivers USB: CH340G (ESP8266 D1) o CP2102 (ESP32)
- Cable USB Mini/Micro o USB-C según tu placa

### Para Web App
- Node.js 18+
- npm 9+

### Para Mobile App
- Flutter 3.10+
- Android SDK (para APK)
- Emulador o dispositivo Android 8+

---

## 1️⃣ Instalación del Firmware

### Opción A: PlatformIO (Recomendado ⭐)

#### 1.1 Instalar PlatformIO CLI
```bash
pip install platformio
```

#### 1.2 Clonar y preparar
```bash
git clone https://github.com/creadorjuegos101-byte/capellan-led.git
cd capellan-led/firmware
```

#### 1.3 Compilar y subir (elige tu placa)

**ESP8266 D1:**
```bash
pio run -e esp8266-d1 -t upload
```

**ESP32 C3:**
```bash
pio run -e esp32-c3 -t upload
```

**ESP32 WROOM-32:**
```bash
pio run -e esp32-wroom -t upload
```

#### 1.4 Verificar
```bash
pio device monitor -b 115200
```

Deberías ver logs del sistema en la consola.

---

### Opción B: Arduino IDE

#### 1.1 Descargar Arduino IDE
https://www.arduino.cc/en/software

#### 1.2 Agregar URLs de placas

**Archivo → Preferencias**

En "URLs adicionales de gestor de tarjetas" agrega:
```
https://arduino.esp8266.com/stable/package_esp8266com_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

#### 1.3 Instalar placas

**Herramientas → Placa → Gestor de tarjetas**

Busca e instala:
- `ESP8266` por ESP8266 Community
- `esp32` por Espressif Systems

#### 1.4 Instalar drivers

**Windows:**
```
# Descargar driver CH340 desde:
https://www.wch.cn/downloads/CH341SER_ZIP.html
# Seguir instrucciones de instalación
```

**Mac/Linux:**
```bash
# Generalmente detecta automáticamente
# Si no, instalar: brew install ch340g-driver
```

#### 1.5 Cargar firmware

1. Conecta tu ESP vía USB
2. **Herramientas → Puerto** - Selecciona COM port (ej: COM3)
3. **Herramientas → Placa** - Selecciona tu ESP
4. **Herramientas → Velocidad** - 921600
5. **Archivo → Abrir** - `firmware/esp8266-d1/main.ino` (según tu placa)
6. Haz clic en **Subir** (botón de flecha)

Espera "Upload complete" ✓

---

## 2️⃣ Configuración de Conexión USB

### Windows
1. Conecta ESP vía USB
2. Abre Device Manager
3. Busca "USB-SERIAL" o "CH340"
4. Si hay ⚠️ amarillo, instala driver

### Mac
```bash
# Detecta automáticamente en /dev/cu.usbserial-*
ls /dev/cu.usbserial-*
```

### Linux
```bash
# Detecta automáticamente
ls /dev/ttyUSB*
# Puede necesitar: sudo usermod -a -G dialout $USER
```

---

## 3️⃣ Instalación de Web App

### 3.1 Clonar (si no lo hiciste)
```bash
git clone https://github.com/creadorjuegos101-byte/capellan-led.git
cd capellan-led/web-app
```

### 3.2 Instalar dependencias
```bash
npm install
```

### 3.3 Ejecutar en desarrollo
```bash
npm run dev
```

Abre: `http://localhost:5173`

### 3.4 Build para producción
```bash
npm run build
```

Output en `dist/` - listo para desplegar

---

## 4️⃣ Compilación de APK Android

### 4.1 Instalar Flutter
```bash
# Descargar desde:
https://docs.flutter.dev/get-started/install

# Verificar instalación:
flutter --version
flutter doctor
```

### 4.2 Clonar proyecto
```bash
git clone https://github.com/creadorjuegos101-byte/capellan-led.git
cd capellan-led/mobile-app
```

### 4.3 Obtener dependencias
```bash
flutter pub get
```

### 4.4 Compilar APK
```bash
# Debug APK (para testing):
flutter build apk

# Release APK (para distribuir):
flutter build apk --release
```

APK generado en: `build/app/release/app-release.apk`

### 4.5 Instalar en dispositivo
```bash
flutter install
```

O manualmente:
```bash
adb install build/app/release/app-release.apk
```

---

## 5️⃣ Conexión Inicial

### Método 1: USB (Recomendado primero)

1. Conecta ESP a PC vía USB
2. Abre http://localhost:5173 (web app)
3. App detecta automáticamente puerto COM
4. Haz clic en "Conectar"
5. ¡Listo! Deberías ver el status

### Método 2: WiFi

1. ESP crea red WiFi: `Capellan-LED-XXXXX`
2. Conecta a esa red
3. Abre navegador: `http://192.168.4.1`
4. Web app carga automáticamente
5. ¡Conectado!

### Método 3: Mobile App

1. Abre app en tu Android
2. Selecciona conexión (USB o WiFi)
3. Busca dispositivos
4. Toca para conectar
5. ¡Sincronizado!

---

## 6️⃣ Verificación de Instalación

### Checklist
- ✓ Firmware cargado (web app muestra "Connected")
- ✓ USB Serial funcionando
- ✓ WiFi AP visible
- ✓ Web app accesible
- ✓ Mobile app detecta dispositivos

### Troubleshooting

**"No se detecta puerto COM"**
- Reinstala drivers USB
- Prueba otro cable USB
- Reinicia PC y ESP

**"Connection timeout"**
- Verifica velocidad baud (115200)
- Comprueba cables
- Reinicia web app

**"WiFi no visible"**
- Reinicia ESP (botón reset)
- Espera 10 segundos
- Busca red: "Capellan-LED-*"

**"APK no instala"**
- Habilita "Instalar desde fuentes desconocidas"
- Usa: `adb install -r app.apk`
- Verifica Android 8+

---

## 7️⃣ Próximos Pasos

1. **Carga un Timeline de ejemplo**
   - Ve a: [docs/examples/](../docs/examples/)
   - Importa JSON de ejemplo

2. **Configura tus LEDs**
   - En Settings → LED Config
   - Selecciona tipo: WS2812B o APA102
   - Número de LEDs
   - Pin GPIO

3. **Selecciona tipo de prop**
   - POI / STAFF / TRAJE / LEVID WAND
   - Visualización cambia según tipo

4. **Reproduce tu primer timeline**
   - Play en web app
   - Observa animación en LEDs

---

## 📞 Soporte

- 📖 [Documentación completa](.)
- 🐛 [Reportar problemas](https://github.com/creadorjuegos101-byte/capellan-led/issues)
- 💬 [Preguntas en Discusiones](https://github.com/creadorjuegos101-byte/capellan-led/discussions)

---

**Última actualización:** Junio 2026 | v1.0.0-alpha

# 🔧 Guía de Desarrollo - Capellán LED

## Configuración del Entorno

### Herramientas Requeridas
```bash
# Node.js + npm (para web app)
node --version  # 18+
npm --version   # 9+

# Python (para firmware)
python --version  # 3.8+

# PlatformIO
pip install platformio
pio --version

# Flutter (para mobile)
flutter --version  # 3.10+
```

---

## Estructura de Desarrollo

### Web App (React + Vite)
```
web-app/
├── src/
│   ├── components/
│   │   ├── TimelineEditor/
│   │   │   ├── TimelineGrid.jsx
│   │   │   ├── ColorPicker.jsx
│   │   │   ├── EffectLibrary.jsx
│   │   │   └── Timeline3DPreview.jsx
│   │   ├── DeviceManager/
│   │   │   ├── DeviceList.jsx
│   │   │   ├── ConnectionStatus.jsx
│   │   │   └── DeviceMonitor.jsx
│   │   ├── MusicPlayer/
│   │   │   ├── Player.jsx
│   │   │   ├── Waveform.jsx
│   │   │   └── BeatDetector.jsx
│   │   └── PropVisualizer/
│   │       ├── POIPreview.jsx
│   │       ├── STAFFPreview.jsx
│   │       ├── TRAJEPreview.jsx
│   │       └── LEVIDWANDPreview.jsx
│   ├── pages/
│   │   ├── Dashboard.jsx
│   │   ├── Editor.jsx
│   │   ├── Devices.jsx
│   │   └── Settings.jsx
│   ├── services/
│   │   ├── usb-serial.js
│   │   ├── wifi-connection.js
│   │   ├── timeline-api.js
│   │   └── music-sync.js
│   ├── stores/
│   │   ├── device-store.js (Zustand)
│   │   ├── timeline-store.js
│   │   ├── settings-store.js
│   │   └── playback-store.js
│   ├── utils/
│   │   ├── timeline-parser.js
│   │   ├── color-utils.js
│   │   ├── effect-engine.js
│   │   └── sync-utils.js
│   ├── App.jsx
│   └── main.jsx
├── public/
├── index.html
├── package.json
├── vite.config.js
└── tailwind.config.js
```

### Firmware (C++ / Arduino)
```
firmware/
├── esp8266-d1/
│   └── main.ino / main.cpp
├── esp32-c3/
│   └── main.ino / main.cpp
├── esp32-wroom/
│   └── main.ino / main.cpp
├── shared/
│   ├── protocols.h
│   ├── timeline_executor.cpp
│   ├── effect_engine.cpp
│   ├── color_converter.cpp
│   ├── led_driver.cpp
│   └── ws2812_driver.cpp
├── platformio.ini
└── libraries/
    ├── FastLED/
    ├── ArduinoJSON/
    └── custom_libs/
```

---

## Flujo de Desarrollo

### 1. Clonar y Configurar
```bash
git clone https://github.com/creadorjuegos101-byte/capellan-led.git
cd capellan-led
git checkout development
git pull origin development
```

### 2. Crear Rama de Feature
```bash
git checkout -b feature/tu-feature-aqui
```

### 3. Desarrollo Web App
```bash
cd web-app
npm install
npm run dev

# En otra terminal - tests
npm run test

# Build
npm run build
```

### 4. Desarrollo Firmware
```bash
cd firmware

# Con PlatformIO
pio run -e esp8266-d1          # Compilar
pio run -e esp8266-d1 -t upload # Subir
pio device monitor             # Monitor serial

# O con Arduino IDE
# File → Open → firmware/esp8266-d1/main.ino
```

### 5. Desarrollo Mobile
```bash
cd mobile-app

# Flutter hot reload
flutter run

# Build APK
flutter build apk --release
```

---

## Estándares de Código

### JavaScript/React
```javascript
// Naming
const deviceStore = useDeviceStore();
const [isConnected, setIsConnected] = useState(false);
function handleTimelineUpload() {}

// Archivos
- PascalCase para componentes: TimelineEditor.jsx
- camelCase para utils: timeline-parser.js
- camelCase para stores: device-store.js
```

### C++/Firmware
```cpp
// Naming
class TimelineExecutor { };
void handleSerialCommand() { }
uint8_t calculateCRC8() { }

// Constantes
#define LED_PIN 5
const int MAX_FRAMES = 1000;
```

### Commits
```
feat: Add timeline upload over USB
fix: Correct CRC8 calculation in serial protocol
docs: Update installation guide
refactor: Extract effect engine to separate module
test: Add unit tests for color conversion
```

---

## Testing

### Web App
```bash
cd web-app

# Unit tests
npm run test

# Coverage
npm run test:coverage

# E2E tests
npm run test:e2e
```

### Firmware
```bash
cd firmware

# Compilar tests
pio run -e test

# Serial monitor (para debug)
pio device monitor -b 115200
```

---

## Build & Deploy

### Web App (Vercel)
```bash
# Deploy automático desde desarrollo
git push origin development

# O manual
npm run build
vercel --prod
```

### Firmware OTA
```bash
# Actualización over-the-air
curl -X POST http://192.168.4.1/api/ota -F "file=@firmware.bin"
```

---

## Debugging

### Serial Monitor
```bash
pio device monitor -b 115200 --filter=send

# Ver todos los datos
pio device monitor -b 115200 --raw
```

### Web App Developer Tools
```javascript
// En consola del navegador
const store = useDeviceStore.getState();
console.log(store);

// Storage
localStorage.getItem('device_config')
```

### Flutter DevTools
```bash
flutter pub global activate devtools
devtools
```

---

## Contribución

### Pull Request Checklist
- [ ] Código sigue estándares del proyecto
- [ ] Tests pasan
- [ ] Documentation actualizada
- [ ] Sin conflictos con `development`
- [ ] Descripción clara del cambio

### Process
1. Fork del repo
2. Crear rama feature
3. Commit + Push
4. Crear Pull Request hacia `development`
5. Review de mantenedor
6. Merge

---

## Performance Tips

### Web App
```javascript
// Usar React.memo para componentes pesados
const TimelineGrid = React.memo(function({ timeline }) {
  return <div>{timeline.frames.length}</div>;
});

// Usar useCallback para event handlers
const handleUpload = useCallback((file) => {
  // ...
}, [dependency]);

// Lazy load componentes
const Editor = React.lazy(() => import('./Editor'));
```

### Firmware
```cpp
// Usar FreeRTOS tasks para operaciones no-bloqueantes
xTaskCreate(playbackTask, "Playback", 4096, NULL, 1, NULL);

// Minimizar cálculos en ISR
void ledUpdateISR() {
  // Solo actualizar puntero
  current_frame_index++;
}

// Pre-asignar buffers
uint8_t led_buffer[300 * 3]; // 300 LEDs RGB
```

---

## Recursos

- [React Docs](https://react.dev)
- [Arduino Reference](https://www.arduino.cc/reference/en/)
- [PlatformIO Docs](https://docs.platformio.org/)
- [Flutter Docs](https://flutter.dev/docs)
- [WebSocket MDN](https://developer.mozilla.org/en-US/docs/Web/API/WebSocket)

---

**Última actualización:** Junio 2026 | v1.0.0

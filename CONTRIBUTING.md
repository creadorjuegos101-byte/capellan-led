# Contributing to Capellán LED

Thank you for your interest in contributing! This document provides guidelines for contributing to the Capellán LED project.

## Getting Started

1. **Fork the repository**
   ```bash
   git clone https://github.com/YOUR_USERNAME/capellan-led.git
   cd capellan-led
   ```

2. **Create a development branch**
   ```bash
   git checkout -b feature/your-feature-name
   ```

3. **Set up development environment**
   - See [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)

## Code Style

### JavaScript/React
```javascript
// Use camelCase for variables and functions
const handleTimelineUpload = () => { };

// PascalCase for components
export function TimelineEditor() { }

// Use clear, descriptive names
const isDeviceConnected = true;
```

### C++/Firmware
```cpp
// snake_case for functions
void handle_serial_command() { }

// UPPERCASE for constants
#define MAX_LEDS 500
const int BAUD_RATE = 115200;

// Classes in PascalCase
class TimelineExecutor { };
```

### Dart/Flutter
```dart
// camelCase for variables
final String deviceName = "ESP8266";

// PascalCase for classes
class DeviceInfo { }

// Use type annotations
Future<bool> connectDevice(String deviceId) async { }
```

## Commit Messages

Follow conventional commits:
```
feat: Add new feature
fix: Fix a bug
docs: Update documentation
refactor: Refactor code
test: Add tests
style: Format code
chore: Maintenance tasks
```

Example:
```
feat: Add master/slave synchronization protocol

- Implement sync message structure
- Add slave pool management
- Handle reconnection logic
- Add sync health monitoring
```

## Pull Request Process

1. **Ensure code quality**
   - Run linters: `npm run lint` (web) or `pio check` (firmware)
   - Run tests: `npm run test` (web)
   - No console warnings or errors

2. **Update documentation**
   - Update relevant `.md` files
   - Add code comments for complex logic
   - Update API docs if applicable

3. **Create descriptive PR**
   - Clear title: `feat: Add feature X`
   - Description of changes
   - List any breaking changes
   - Reference related issues

4. **Respond to reviews**
   - Address reviewer feedback
   - Push fixes to same branch
   - Re-request review when ready

## Testing

### Firmware Tests
```bash
cd firmware
pio run -e esp8266-d1 -t test
```

### Web App Tests
```bash
cd web-app
npm run test
npm run test:coverage
```

### Manual Testing Checklist
- [ ] USB connection works
- [ ] WiFi connection works
- [ ] Timeline uploads successfully
- [ ] Playback starts/stops correctly
- [ ] LEDs respond to commands
- [ ] Master/Slave sync works
- [ ] No memory leaks
- [ ] Performance acceptable

## Issue Reporting

Use GitHub Issues with:
- Clear title
- Description of problem
- Steps to reproduce
- Expected vs actual behavior
- System info (OS, device, etc)

## Areas for Contribution

High priority:
- [ ] LED driver optimization
- [ ] Master/Slave stability
- [ ] Mobile app completion
- [ ] Visualization improvements
- [ ] Documentation

Medium priority:
- [ ] Effect library expansion
- [ ] Audio sync improvements
- [ ] Cloud sync
- [ ] Plugin system

Low priority:
- [ ] UI themes
- [ ] Localization
- [ ] Advanced analytics

## Questions?

- 💬 Open a Discussion
- 📧 Contact maintainers
- 📖 Check existing documentation

## License

By contributing, you agree your work is licensed under MIT License.

---

**Thank you for contributing to Capellán LED! 🎭✨**

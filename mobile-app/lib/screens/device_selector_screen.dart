import 'package:flutter/material.dart';
import 'package:provider/provider.dart';

// ============= DEVICE MODEL =============

class DeviceInfo {
  final String id;
  final String name;
  final String type; // 'ESP8266', 'ESP32', etc
  final String? ip;
  final int numLeds;
  final String ledType;
  bool connected;

  DeviceInfo({
    required this.id,
    required this.name,
    required this.type,
    this.ip,
    required this.numLeds,
    required this.ledType,
    this.connected = false,
  });
}

// ============= DEVICE PROVIDER =============

class DeviceProvider extends ChangeNotifier {
  List<DeviceInfo> devices = [];
  DeviceInfo? activeDevice;

  void addDevice(DeviceInfo device) {
    devices.add(device);
    notifyListeners();
  }

  void setActiveDevice(DeviceInfo device) {
    activeDevice = device;
    notifyListeners();
  }

  void removeDevice(String deviceId) {
    devices.removeWhere((d) => d.id == deviceId);
    if (activeDevice?.id == deviceId) {
      activeDevice = null;
    }
    notifyListeners();
  }

  void updateDeviceStatus(String deviceId, bool connected) {
    final index = devices.indexWhere((d) => d.id == deviceId);
    if (index >= 0) {
      devices[index].connected = connected;
      notifyListeners();
    }
  }
}

// ============= DEVICE DISCOVERY =============

class DeviceDiscovery {
  static final List<String> commonIPs = [
    '192.168.4.1',
    '192.168.1.100',
    '10.0.0.1',
  ];

  static Future<List<DeviceInfo>> discoverDevices() async {
    final List<DeviceInfo> devices = [];

    for (String ip in commonIPs) {
      try {
        // Simular descubrimiento (reemplazar con HTTP request real)
        final device = DeviceInfo(
          id: 'wifi_$ip',
          name: 'Device at $ip',
          type: 'ESP8266',
          ip: ip,
          numLeds: 30,
          ledType: 'ws2812b',
          connected: false,
        );
        devices.add(device);
      } catch (e) {
        // Device not found
      }
    }

    return devices;
  }

  static Future<bool> connectUSB() async {
    // Implementación USB Serial
    // usando: flutter_libserialport
    return false;
  }
}

// ============= DEVICE SELECTOR SCREEN =============

class DeviceSelectorScreen extends StatefulWidget {
  const DeviceSelectorScreen({Key? key}) : super(key: key);

  @override
  State<DeviceSelectorScreen> createState() => _DeviceSelectorScreenState();
}

class _DeviceSelectorScreenState extends State<DeviceSelectorScreen> {
  bool isDiscovering = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('🎭 Capellán LED'),
        backgroundColor: Colors.grey[900],
        elevation: 0,
      ),
      backgroundColor: Colors.grey[950],
      body: Consumer<DeviceProvider>(
        builder: (context, deviceProvider, _) {
          return SingleChildScrollView(
            padding: const EdgeInsets.all(16),
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.start,
              children: [
                // Connection Methods
                Card(
                  color: Colors.grey[850],
                  child: Padding(
                    padding: const EdgeInsets.all(16),
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.start,
                      children: [
                        const Text(
                          'Connection Methods',
                          style: TextStyle(
                            fontSize: 18,
                            fontWeight: FontWeight.bold,
                            color: Colors.white,
                          ),
                        ),
                        const SizedBox(height: 16),
                        ElevatedButton.icon(
                          onPressed: () async {
                            final success = await DeviceDiscovery.connectUSB();
                            if (success && mounted) {
                              ScaffoldMessenger.of(context).showSnackBar(
                                const SnackBar(content: Text('✓ USB Connected')),
                              );
                            }
                          },
                          icon: const Icon(Icons.cable),
                          label: const Text('Connect via USB'),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.blue,
                            minimumSize: const Size(double.infinity, 50),
                          ),
                        ),
                        const SizedBox(height: 12),
                        ElevatedButton.icon(
                          onPressed: isDiscovering
                              ? null
                              : () async {
                                  setState(() => isDiscovering = true);

                                  final devices =
                                      await DeviceDiscovery.discoverDevices();

                                  if (mounted) {
                                    for (var device in devices) {
                                      deviceProvider.addDevice(device);
                                    }
                                    setState(() => isDiscovering = false);
                                  }
                                },
                          icon: const Icon(Icons.wifi),
                          label: Text(isDiscovering
                              ? 'Discovering...'
                              : 'Discover WiFi Devices'),
                          style: ElevatedButton.styleFrom(
                            backgroundColor: Colors.green,
                            minimumSize: const Size(double.infinity, 50),
                          ),
                        ),
                      ],
                    ),
                  ),
                ),
                const SizedBox(height: 24),

                // Available Devices
                if (deviceProvider.devices.isNotEmpty)
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const Text(
                        'Available Devices',
                        style: TextStyle(
                          fontSize: 18,
                          fontWeight: FontWeight.bold,
                          color: Colors.white,
                        ),
                      ),
                      const SizedBox(height: 12),
                      ...deviceProvider.devices.map((device) {
                        return Card(
                          color: Colors.grey[800],
                          margin: const EdgeInsets.only(bottom: 8),
                          child: ListTile(
                            leading: Icon(
                              device.type.contains('USB')
                                  ? Icons.cable
                                  : Icons.router,
                              color: device.connected
                                  ? Colors.green
                                  : Colors.orange,
                            ),
                            title: Text(
                              device.name,
                              style: const TextStyle(color: Colors.white),
                            ),
                            subtitle: Text(
                              '${device.type} • ${device.numLeds} LEDs',
                              style: TextStyle(color: Colors.grey[400]),
                            ),
                            trailing: device.connected
                                ? const Icon(Icons.check_circle,
                                    color: Colors.green)
                                : const Icon(Icons.radio_button_unchecked,
                                    color: Colors.grey),
                            onTap: () {
                              deviceProvider.setActiveDevice(device);
                              Navigator.of(context).pop(device);
                            },
                          ),
                        );
                      }).toList(),
                    ],
                  ),

                // Active Device
                if (deviceProvider.activeDevice != null)
                  Column(
                    crossAxisAlignment: CrossAxisAlignment.start,
                    children: [
                      const SizedBox(height: 24),
                      Card(
                        color: Colors.green[900],
                        child: Padding(
                          padding: const EdgeInsets.all(16),
                          child: Row(
                            children: [
                              const Icon(Icons.check_circle,
                                  color: Colors.green, size: 32),
                              const SizedBox(width: 12),
                              Expanded(
                                child: Column(
                                  crossAxisAlignment: CrossAxisAlignment.start,
                                  children: [
                                    const Text(
                                      'Connected Device',
                                      style: TextStyle(
                                        color: Colors.green,
                                        fontSize: 12,
                                      ),
                                    ),
                                    Text(
                                      deviceProvider.activeDevice!.name,
                                      style: const TextStyle(
                                        color: Colors.white,
                                        fontSize: 16,
                                        fontWeight: FontWeight.bold,
                                      ),
                                    ),
                                  ],
                                ),
                              ),
                              ElevatedButton(
                                onPressed: () {
                                  Navigator.of(context).pushNamed('/editor',
                                      arguments:
                                          deviceProvider.activeDevice);
                                },
                                style: ElevatedButton.styleFrom(
                                  backgroundColor: Colors.blue,
                                ),
                                child: const Text('Editor'),
                              ),
                            ],
                          ),
                        ),
                      ),
                    ],
                  ),
              ],
            ),
          );
        },
      ),
    );
  }
}

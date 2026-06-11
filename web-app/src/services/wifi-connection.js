// WiFi Connection Service
// For web app (WebSocket communication)

class WiFiConnectionService {
  constructor() {
    this.socket = null;
    this.isConnected = false;
    this.url = null;
    this.callbacks = {};
    this.reconnectAttempts = 0;
    this.maxReconnectAttempts = 5;
    this.reconnectInterval = 3000;
  }

  on(event, callback) {
    if (!this.callbacks[event]) {
      this.callbacks[event] = [];
    }
    this.callbacks[event].push(callback);
  }

  emit(event, data) {
    if (this.callbacks[event]) {
      this.callbacks[event].forEach(cb => cb(data));
    }
  }

  // Descubrir dispositivos ESP en la red local
  async discoverDevices() {
    const devices = [];
    const commonIPs = [
      '192.168.4.1',
      '192.168.1.100',
      '10.0.0.1',
      '172.20.10.1',
    ];

    for (const ip of commonIPs) {
      try {
        const response = await fetch(`http://${ip}:8080/api/info`, {
          timeout: 1000,
        });
        if (response.ok) {
          const data = await response.json();
          devices.push({
            ip,
            name: data.name || `Device at ${ip}`,
            ...data,
          });
        }
      } catch (error) {
        // Device not found or unreachable
      }
    }

    return devices;
  }

  // Conectar a dispositivo WiFi
  async connect(ip, port = 8080) {
    return new Promise((resolve, reject) => {
      try {
        this.url = `ws://${ip}:${port}/api/devices`;
        
        this.socket = new WebSocket(this.url);

        this.socket.onopen = () => {
          console.log('✓ WebSocket connected:', this.url);
          this.isConnected = true;
          this.reconnectAttempts = 0;

          // Handshake
          this.sendMessage({
            type: 'handshake',
            client_id: 'web_app_' + Date.now(),
            version: '1.0.0',
          });

          this.emit('connected', { ip, port });
          resolve(true);
        };

        this.socket.onmessage = (event) => {
          try {
            const data = JSON.parse(event.data);
            this.handleMessage(data);
          } catch (error) {
            console.error('Parse error:', error);
          }
        };

        this.socket.onerror = (error) => {
          console.error('WebSocket error:', error);
          this.emit('error', error);
          reject(error);
        };

        this.socket.onclose = () => {
          console.log('WebSocket disconnected');
          this.isConnected = false;
          this.attemptReconnect();
        };

        setTimeout(() => {
          if (!this.isConnected) {
            reject(new Error('Connection timeout'));
          }
        }, 5000);
      } catch (error) {
        reject(error);
      }
    });
  }

  // Intentar reconectar
  attemptReconnect() {
    if (this.reconnectAttempts < this.maxReconnectAttempts) {
      this.reconnectAttempts++;
      console.log(`Attempting reconnect ${this.reconnectAttempts}/${this.maxReconnectAttempts}...`);
      
      setTimeout(() => {
        if (this.url) {
          this.connect(this.url.split('/')[2].split(':')[0]);
        }
      }, this.reconnectInterval);
    } else {
      this.emit('connection_failed');
    }
  }

  // Desconectar
  disconnect() {
    if (this.socket) {
      this.socket.close();
      this.isConnected = false;
      this.socket = null;
    }
  }

  // Manejar mensaje recibido
  handleMessage(data) {
    const { type } = data;

    switch (type) {
      case 'ready':
        this.emit('ready', data);
        break;
      case 'playback_status':
        this.emit('playback_status', data);
        break;
      case 'sync_ack':
        this.emit('sync_ack', data);
        break;
      case 'device_found':
        this.emit('device_found', data);
        break;
      default:
        this.emit('message', data);
    }
  }

  // Enviar mensaje
  sendMessage(data) {
    if (!this.isConnected || !this.socket) {
      console.error('WebSocket not connected');
      return false;
    }

    try {
      this.socket.send(JSON.stringify(data));
      return true;
    } catch (error) {
      console.error('Send error:', error);
      return false;
    }
  }

  // Comandos específicos
  uploadTimeline(timeline) {
    return this.sendMessage({
      type: 'timeline_upload',
      id: timeline.id,
      name: timeline.name,
      data: timeline,
      compression: 'gzip',
    });
  }

  play(timelineId, loop = false) {
    return this.sendMessage({
      type: 'playback',
      action: 'play',
      timeline_id: timelineId,
      loop,
      speed: 100,
    });
  }

  stop() {
    return this.sendMessage({
      type: 'playback',
      action: 'stop',
    });
  }

  pause() {
    return this.sendMessage({
      type: 'playback',
      action: 'pause',
    });
  }

  sync(frameId, timestamp) {
    return this.sendMessage({
      type: 'sync',
      frame_id: frameId,
      timestamp_ms: timestamp,
    });
  }

  discoverDevicesWS() {
    return this.sendMessage({
      type: 'discover',
      timeout_ms: 5000,
    });
  }
}

export default new WiFiConnectionService();

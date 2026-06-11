// Timeline Executor - Motor de reproducción de timelines
// Capellán LED v1.0.0

#pragma once

#include <Arduino.h>
#include <ArduinoJSON.h>
#include "ws2812_driver.h"

// ============= ESTRUCTURA DE FRAME =============

struct TimelineFrame {
  uint32_t time_ms;           // Tiempo de inicio
  uint32_t duration_ms;       // Duración
  String effect;              // Tipo de efecto
  uint32_t color_primary;     // Color primario (hex)
  uint32_t color_secondary;   // Color secundario (hex)
  uint8_t intensity;          // 0-255
  uint8_t speed;              // 0-100
};

// ============= CLASE TIMELINE =============

class TimelineExecutor {
private:
  TimelineFrame* frames;
  uint16_t num_frames;
  uint32_t timeline_duration_ms;
  uint32_t current_time_ms;
  uint16_t current_frame_index;
  bool playing;
  bool loop_enabled;
  WS2812B* led_strip;
  
  // Helpers
  TimelineFrame* findFrameByTime(uint32_t time_ms) {
    for (uint16_t i = 0; i < num_frames; i++) {
      if (frames[i].time_ms <= time_ms && 
          time_ms < frames[i].time_ms + frames[i].duration_ms) {
        return &frames[i];
      }
    }
    return nullptr;
  }
  
  void applyEffect(TimelineFrame* frame) {
    if (!frame || !led_strip) return;
    
    CRGB color_p = WS2812B::hexToRgb(frame->color_primary);
    CRGB color_s = WS2812B::hexToRgb(frame->color_secondary);
    
    if (frame->effect == "solid") {
      applySolid(color_p, frame->intensity);
    }
    else if (frame->effect == "pulse") {
      applyPulse(color_p, frame->speed, frame->intensity);
    }
    else if (frame->effect == "chase") {
      applyChase(color_p, color_s, frame->speed, frame->intensity);
    }
    else if (frame->effect == "rainbow") {
      applyRainbow(frame->speed, frame->intensity);
    }
    else if (frame->effect == "gradient") {
      applyGradient(color_p, color_s, frame->intensity);
    }
    else if (frame->effect == "strobe") {
      applyStrobe(color_p, frame->speed, frame->intensity);
    }
    else if (frame->effect == "wave") {
      applyWave(color_p, frame->speed, frame->intensity);
    }
    else if (frame->effect == "fade") {
      applyFade(color_p, color_s, frame->speed, frame->intensity);
    }
    else if (frame->effect == "sparkle") {
      applySparkle(color_p, frame->speed, frame->intensity);
    }
  }
  
  // Efectos
  void applySolid(CRGB color, uint8_t intensity) {
    color *= (intensity / 255.0f);
    led_strip->fill(color);
    led_strip->show();
  }
  
  void applyPulse(CRGB color, uint8_t speed, uint8_t intensity) {
    static uint32_t pulse_time = 0;
    uint8_t brightness = 128 + 127 * sin(pulse_time * speed / 100.0f);
    
    color *= (brightness / 255.0f);
    color *= (intensity / 255.0f);
    led_strip->fill(color);
    led_strip->show();
    
    pulse_time++;
  }
  
  void applyChase(CRGB color_a, CRGB color_b, uint8_t speed, uint8_t intensity) {
    static uint16_t chase_pos = 0;
    
    led_strip->clear();
    
    for (uint16_t i = 0; i < led_strip->size(); i++) {
      uint16_t pos = (chase_pos + i) % led_strip->size();
      CRGB color = (i % 2 == 0) ? color_a : color_b;
      color *= (intensity / 255.0f);
      (*led_strip)[pos] = color;
    }
    
    led_strip->show();
    chase_pos++;
  }
  
  void applyRainbow(uint8_t speed, uint8_t intensity) {
    static uint16_t rainbow_hue = 0;
    
    for (uint16_t i = 0; i < led_strip->size(); i++) {
      uint16_t hue = (rainbow_hue + i * 256 / led_strip->size()) & 0xFF;
      CRGB color = WS2812B::hsvToRgb(hue, 255, 255);
      color *= (intensity / 255.0f);
      (*led_strip)[i] = color;
    }
    
    led_strip->show();
    rainbow_hue += speed / 10;
  }
  
  void applyGradient(CRGB color_start, CRGB color_end, uint8_t intensity) {
    for (uint16_t i = 0; i < led_strip->size(); i++) {
      float ratio = (float)i / led_strip->size();
      CRGB color(
        color_start.r + (color_end.r - color_start.r) * ratio,
        color_start.g + (color_end.g - color_start.g) * ratio,
        color_start.b + (color_end.b - color_start.b) * ratio
      );
      color *= (intensity / 255.0f);
      (*led_strip)[i] = color;
    }
    
    led_strip->show();
  }
  
  void applyStrobe(CRGB color, uint8_t speed, uint8_t intensity) {
    static uint32_t strobe_time = 0;
    bool on = (strobe_time / (101 - speed)) % 2 == 0;
    
    if (on) {
      color *= (intensity / 255.0f);
      led_strip->fill(color);
    } else {
      led_strip->clear();
    }
    
    led_strip->show();
    strobe_time++;
  }
  
  void applyWave(CRGB color, uint8_t speed, uint8_t intensity) {
    static uint32_t wave_time = 0;
    
    for (uint16_t i = 0; i < led_strip->size(); i++) {
      uint8_t brightness = 128 + 127 * sin((wave_time + i * speed / 10.0f) / 10.0f);
      CRGB wave_color = color;
      wave_color *= (brightness / 255.0f);
      wave_color *= (intensity / 255.0f);
      (*led_strip)[i] = wave_color;
    }
    
    led_strip->show();
    wave_time++;
  }
  
  void applyFade(CRGB color_from, CRGB color_to, uint8_t speed, uint8_t intensity) {
    static uint32_t fade_time = 0;
    float ratio = (fade_time % (101 - speed)) / (101.0f - speed);
    
    CRGB color(
      color_from.r + (color_to.r - color_from.r) * ratio,
      color_from.g + (color_to.g - color_from.g) * ratio,
      color_from.b + (color_to.b - color_from.b) * ratio
    );
    color *= (intensity / 255.0f);
    led_strip->fill(color);
    led_strip->show();
    
    fade_time++;
  }
  
  void applySparkle(CRGB color, uint8_t speed, uint8_t intensity) {
    led_strip->fill(Colors::BLACK);
    
    for (uint16_t i = 0; i < 5; i++) {
      uint16_t pos = random(led_strip->size());
      CRGB sparkle = color;
      sparkle *= (intensity / 255.0f);
      (*led_strip)[pos] = sparkle;
    }
    
    led_strip->show();
  }

public:
  TimelineExecutor(WS2812B* _led_strip) 
    : led_strip(_led_strip), num_frames(0), frames(nullptr),
      current_time_ms(0), current_frame_index(0), 
      playing(false), loop_enabled(false) {}
  
  ~TimelineExecutor() {
    if (frames) delete[] frames;
  }
  
  // Cargar timeline desde JSON
  bool loadFromJSON(const char* json_str) {
    StaticJsonDocument<4096> doc;
    DeserializationError error = deserializeJson(doc, json_str);
    
    if (error) {
      Serial.print("JSON Parse Error: ");
      Serial.println(error.c_str());
      return false;
    }
    
    timeline_duration_ms = doc["metadata"]["duration_ms"];
    uint16_t frame_count = doc["frames"].size();
    
    // Asignar memoria
    if (frames) delete[] frames;
    frames = new TimelineFrame[frame_count];
    num_frames = frame_count;
    
    // Cargar frames
    JsonArray frames_array = doc["frames"];
    for (uint16_t i = 0; i < frame_count; i++) {
      frames[i].time_ms = frames_array[i]["time_ms"];
      frames[i].duration_ms = frames_array[i]["duration_ms"];
      frames[i].effect = frames_array[i]["effect"].as<String>();
      
      // Parse hex colors
      String color_str = frames_array[i]["colors"]["primary"].as<String>();
      frames[i].color_primary = (uint32_t)strtol(color_str.c_str() + 1, nullptr, 16);
      
      frames[i].intensity = frames_array[i]["intensity"];
      frames[i].speed = frames_array[i]["speed"];
    }
    
    Serial.printf("Timeline loaded: %d frames, %d ms\n", num_frames, timeline_duration_ms);
    return true;
  }
  
  // Control de reproducción
  void play() {
    playing = true;
    current_time_ms = 0;
    Serial.println("▶ Timeline playing");
  }
  
  void stop() {
    playing = false;
    led_strip->clear();
    led_strip->show();
    Serial.println("⏹ Timeline stopped");
  }
  
  void pause() {
    playing = false;
    Serial.println("⏸ Timeline paused");
  }
  
  void setLoop(bool loop) {
    loop_enabled = loop;
  }
  
  // Actualizar (llamar cada frame)
  void update() {
    if (!playing || !frames) return;
    
    // Buscar frame actual
    TimelineFrame* current_frame = findFrameByTime(current_time_ms);
    if (current_frame) {
      applyEffect(current_frame);
    }
    
    current_time_ms += 16; // ~60 FPS
    
    // Manejo de fin
    if (current_time_ms >= timeline_duration_ms) {
      if (loop_enabled) {
        current_time_ms = 0;
      } else {
        stop();
      }
    }
  }
  
  // Getters
  bool isPlaying() const { return playing; }
  uint32_t getCurrentTime() const { return current_time_ms; }
  uint32_t getDuration() const { return timeline_duration_ms; }
  float getProgress() const {
    if (timeline_duration_ms == 0) return 0.0f;
    return (float)current_time_ms / timeline_duration_ms;
  }
};

#pragma once

// clang-format off
/* === MODULE MANIFEST V2 ===
module_description: 数字灰度（反射式循迹）传感器阵列模块 / Module for digital grey (reflective line-tracking) sensor arrays
depends: []
=== END MANIFEST === */
// clang-format on

#include <array>
#include <cstddef>
#include <cstdint>
#include <initializer_list>

#include "gpio.hpp"
#include "message.hpp"

class GreySensor
{
 public:
  static constexpr size_t MAX_CHANNEL_COUNT = 8;
  static constexpr int16_t POSITION_SCALE = 1000;
  static constexpr uint8_t LOST_SIDE_UNKNOWN = 0;
  static constexpr uint8_t LOST_SIDE_LEFT = 1;
  static constexpr uint8_t LOST_SIDE_RIGHT = 2;

  struct Sample
  {
    uint8_t raw_mask = 0;
    uint8_t active_mask = 0;
    uint8_t changed_mask = 0;
    uint8_t channel_count = 0;
    uint8_t active_count = 0;
    uint8_t line_detected = 0;
    uint8_t line_lost = 0;
    uint8_t lost_side = LOST_SIDE_UNKNOWN;
    int16_t weighted_position = 0;
    int16_t position = 0;
    int16_t remembered_position = 0;
    uint32_t lost_count = 0;
    uint32_t sequence = 0;
    std::array<uint8_t, MAX_CHANNEL_COUNT> raw = {};
    std::array<uint8_t, MAX_CHANNEL_COUNT> active = {};
  };

  GreySensor(std::initializer_list<LibXR::GPIO*> channels, bool active_low = false,
             const char* topic_name = "grey_sensor", uint32_t publish_period_ms = 10);

  Sample Read();

  uint8_t ReadRawMask() const;

  uint8_t ReadActiveMask() const;

  int16_t ReadPosition();

  size_t ChannelCount() const;

  void OnMonitor();

 private:
  static uint8_t BuildBit(size_t channel);
  static int16_t BuildPosition(size_t channel, size_t channel_count);
  static uint8_t GetLostSide(int16_t position);

  Sample ReadDigital() const;
  void UpdatePositionState(Sample& sample);

  std::array<LibXR::GPIO*, MAX_CHANNEL_COUNT> channels_ = {};
  size_t channel_count_ = 0;
  LibXR::Topic topic_;
  bool active_low_ = false;
  uint32_t publish_period_ms_ = 10;
  uint32_t last_publish_ms_ = 0;
  uint32_t sequence_ = 0;
  uint8_t last_active_mask_ = 0;
  int16_t remembered_position_ = 0;
  uint32_t lost_count_ = 0;
  bool has_position_memory_ = false;
  bool has_published_ = false;
};

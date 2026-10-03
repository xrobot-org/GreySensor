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

/**
 * @brief 数字灰度传感器阵列模块，读取 1 到 8 路 GPIO 通道并发布循迹样本。
 *        Digital grey sensor array Module that reads 1 to 8 GPIO channels and publishes
 *        line-tracking samples.
 */
class GreySensor
{
 public:
  static constexpr size_t MAX_CHANNEL_COUNT = 8;   ///< 最大通道数
                                                   ///< Maximum number of channels
  static constexpr int16_t POSITION_SCALE = 1000;  ///< 相邻通道的位置间隔
                                                   ///< Position spacing of adjacent
                                                   ///< channels
  static constexpr uint8_t LOST_SIDE_UNKNOWN = 0;  ///< 丢线侧未知
                                                   ///< Lost side unknown
  static constexpr uint8_t LOST_SIDE_LEFT = 1;     ///< 线最后出现在左侧
                                                   ///< Line last seen on the left
  static constexpr uint8_t LOST_SIDE_RIGHT = 2;    ///< 线最后出现在右侧
                                                   ///< Line last seen on the right

  /**
   * @brief 发布的循迹样本。
   *        Published line-tracking sample.
   */
  struct Sample
  {
    uint8_t raw_mask = 0;  ///< 第 i 位为 1 表示第 i 路读到高电平
    ///< Bit i is 1 when channel i reads high
    uint8_t active_mask = 0;  ///< 第 i 位为 1 表示第 i 路有效
    ///< Bit i is 1 when channel i is active
    uint8_t changed_mask = 0;  ///< active_mask 与上一次发布值的异或
    ///< active_mask XOR the previously published one
    uint8_t channel_count = 0;  ///< 通道数
    ///< Number of channels
    uint8_t active_count = 0;  ///< 有效通道数
    ///< Number of active channels
    uint8_t line_detected = 0;  ///< 有任一通道有效时为 1
    ///< 1 when any channel is active
    uint8_t line_lost = 0;  ///< 没有通道有效时为 1
    ///< 1 when no channel is active
    uint8_t lost_side = LOST_SIDE_UNKNOWN;  ///< 丢线侧：0 未知，1 左侧，2 右侧
    ///< Lost side: 0 unknown, 1 left, 2 right
    int16_t weighted_position = 0;  ///< 有效通道位置的平均值
    ///< Mean position of the active channels
    int16_t position = 0;  ///< weighted_position，丢线时为记忆位置
    ///< weighted_position, or the remembered position while the line is lost
    int16_t remembered_position = 0;  ///< 最近一次检测到线时的位置
    ///< Position at the last detection of the line
    uint32_t lost_count = 0;  ///< 连续丢线读取次数，仅丢线时有意义
    ///< Consecutive lost reads, meaningful only while the line is lost
    uint32_t sequence = 0;  ///< 每次发布加 1
    ///< Increments on every publish
    std::array<uint8_t, MAX_CHANNEL_COUNT> raw = {};  ///< 每路读到的电平，1 为高
    ///< Level read on each channel, 1 for high
    std::array<uint8_t, MAX_CHANNEL_COUNT> active = {};  ///< 每路是否有效，1 为有效
    ///< Whether each channel is active, 1 for active
  };

  /**
   * @brief 构造 GreySensor，把每路 GPIO 配置为无上下拉输入。
   *        Construct GreySensor and configure each GPIO as an input without pull.
   *
   * @param channels 通道 GPIO 指针，按从左到右排列，数量为 1 到 8。
   *                 Channel GPIO pointers ordered from left to right, 1 to 8 of them.
   * @param active_low 为 true 时低电平表示该通道有效。
   *                   When true, a low level means the channel is active.
   * @param topic_name 发布样本的 Topic 名称。
   *                   Name of the Topic that publishes the samples.
   * @param publish_period_ms 最小发布间隔，单位 ms；为 0 时每次 OnMonitor 都发布。
   *                          Minimum publish interval in ms; 0 publishes on every
   *                          OnMonitor call.
   */
  GreySensor(std::initializer_list<LibXR::GPIO*> channels, bool active_low = false,
             const char* topic_name = "grey_sensor", uint32_t publish_period_ms = 10);

  /**
   * @brief 读取所有通道并更新位置状态，不发布。
   *        Read all channels and update the position state, without publishing.
   *
   * @return 样本；changed_mask 与 sequence 为 0。
   *         Sample with changed_mask and sequence equal to 0.
   */
  Sample Read();

  /**
   * @brief 读取各通道的原始电平掩码。
   *        Read the raw level mask of the channels.
   *
   * @return 第 i 位为 1 表示第 i 路读到高电平。
   *         Bit i is 1 when channel i reads high.
   */
  uint8_t ReadRawMask() const;

  /**
   * @brief 读取各通道的有效掩码。
   *        Read the active mask of the channels.
   *
   * @return 第 i 位为 1 表示第 i 路有效。
   *         Bit i is 1 when channel i is active.
   */
  uint8_t ReadActiveMask() const;

  /**
   * @brief 读取并返回当前位置，丢线时为记忆位置。
   *        Read and return the current position, or the remembered position while the
   *        line is lost.
   *
   * @return 位置，左侧为负，右侧为正。
   *         Position, negative on the left and positive on the right.
   */
  int16_t ReadPosition();

  /**
   * @brief 获取通道数。
   *        Get the number of channels.
   *
   * @return 通道数。
   *         Number of channels.
   */
  size_t ChannelCount() const;

  /**
   * @brief 监控回调：按 publish_period_ms 读取并发布一次样本。
   *        Monitor callback: read and publish one sample according to
   *        publish_period_ms.
   */
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

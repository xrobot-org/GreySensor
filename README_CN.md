# GreySensor

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Language](https://img.shields.io/badge/language-C++20-orange.svg)](https://en.cppreference.com/)
[![Framework](https://img.shields.io/badge/Framework-LibXR-green)](https://github.com/xrobot-org/libxr)
[![GitHub stars](https://img.shields.io/github/stars/verdancy-org/GreySensor?style=social)](https://github.com/verdancy-org/GreySensor/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/verdancy-org/GreySensor?style=social)](https://github.com/verdancy-org/GreySensor/network/members)
[![GitHub issues](https://img.shields.io/github/issues/verdancy-org/GreySensor)](https://github.com/verdancy-org/GreySensor/issues)

**GreySensor** 是一个面向循迹和反射式光电阵列的可运行时配置数字灰度传感器模块。

## 模块介绍

GreySensor 按应用层传入的顺序读取数字 GPIO 通道，并通过 LibXR `Topic` 发布紧凑的循迹状态。模块不绑定具体板级引脚命名：板级或应用层负责注册 GPIO 硬件别名，再把这些别名字符串传给模块构造函数。

模块会根据有效通道计算加权线位置，并在短暂丢线时保留最近一次有效位置，方便上层控制环获得稳定的方向反馈。

## 功能特性

- 基于 `LibXR::GPIO` 硬件抽象接口，模块与具体平台 GPIO 实现解耦。

- 遵循 `Application` 框架规范，支持依赖注入，并通过 MANIFEST 接入自动化生命周期管理。

- 支持从 `channel_names` initializer list 自动推导通道数量。当前位图载荷支持 1-8 路通道。

- 发布原始通道状态、有效通道状态、加权位置、丢线状态、记忆位置和连续丢线计数。

- 使用有符号定点位置刻度：左侧为负，右侧为正，中心为 0。

## 硬件需求

- 需要提供与 `channel_names` 配置中每个字符串匹配的 GPIO 设备节点。

## 构造参数

- `channel_names`
  - 从左到右排列的 GPIO 别名列表。列表长度决定实际启用的通道数量。

- `active_low`
  - 是否使用低电平表示对应通道检测到线。

- `topic_name`
  - 用于发布 `GreySensor::Sample` 的 Topic 名称。

- `publish_period_ms`
  - 最小发布间隔，单位为毫秒。设置为 `0` 时在每次 monitor 周期都发布。

### API 参考

```cpp
struct Sample {
    uint8_t raw_mask;
    uint8_t active_mask;
    uint8_t changed_mask;
    uint8_t channel_count;
    uint8_t active_count;
    uint8_t line_detected;
    uint8_t line_lost;
    uint8_t lost_side;
    int16_t weighted_position;
    int16_t position;
    int16_t remembered_position;
    uint32_t lost_count;
    uint32_t sequence;
    std::array<uint8_t, MAX_CHANNEL_COUNT> raw;
    std::array<uint8_t, MAX_CHANNEL_COUNT> active;
};
```

对于 8 路阵列，默认位置刻度映射为：

```text
-3500, -2500, -1500, -500, 500, 1500, 2500, 3500
```

当没有通道有效时，`line_lost` 置位，`position` 保持最近一次有效 `weighted_position`。`lost_side` 为 `0` 表示未知，`1` 表示左侧，`2` 表示右侧。

## 依赖

- 除 LibXR 基础框架外无额外依赖。

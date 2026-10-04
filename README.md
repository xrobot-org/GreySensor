# GreySensor

数字灰度（反射式循迹）传感器阵列模块 / Module for digital grey (reflective line-tracking) sensor arrays

## 1. 模块作用 / Purpose

构造时，GreySensor 要求通道数为 1 到 8（不满足时进入 LibXR 的致命错误处理，debug 与 release 构建相同），断言指针非空，并把每个 GPIO 配置为无上下拉的输入。模块按传入顺序（从左到右）读取这些数字 GPIO 通道，把结果整理成紧凑的循迹样本，并通过 LibXR Topic 发布。模块使用的 GPIO 对象由 BSP 注册，实例配置中按顺序列出。

通道读到高电平即为有效；`active_low` 为 `true` 时读到低电平为有效。`OnMonitor()` 最多每 `publish_period_ms` 毫秒发布一次样本，为 0 时每次 monitor 调用都发布，因此实际发布频率同时受 monitor 循环周期限制。

模块提供以下公共方法：

- `Sample Read()`：读取所有通道并更新位置状态，不发布，返回样本的 `changed_mask` 和 `sequence` 为 0。
- `uint8_t ReadRawMask() const`、`uint8_t ReadActiveMask() const`：只读取掩码。
- `int16_t ReadPosition()`：返回 `Read().position`。
- `size_t ChannelCount() const`：返回通道数。

`Read()` 和 `ReadPosition()` 与发布的样本共用位置记忆和丢线计数。

Upon construction, GreySensor requires 1 to 8 channels (otherwise it enters the LibXR fatal error handler, the same in debug and release builds), asserts that they are non-null and configures each GPIO as an input without pull. The Module reads these digital GPIO channels in the order they are passed (left to right), turns the result into a compact line-tracking sample and publishes it through a LibXR Topic. The GPIO objects used by the Module are registered by the BSP and listed in order in the instance configuration.

A channel is active when it reads high, or when it reads low if `active_low` is `true`. `OnMonitor()` publishes a sample at most every `publish_period_ms` ms, and on every monitor call when it is 0, so the actual publish rate is also bounded by the monitor loop period.

The Module provides the following public methods:

- `Sample Read()`: read all channels and update the position state, without publishing; `changed_mask` and `sequence` of the returned sample are 0.
- `uint8_t ReadRawMask() const`, `uint8_t ReadActiveMask() const`: read the masks only.
- `int16_t ReadPosition()`: return `Read().position`.
- `size_t ChannelCount() const`: return the channel count.

`Read()` and `ReadPosition()` share the position memory and the lost counter with the published samples.

## 2. 位置与样本约定 / Position and Sample Convention

`n` 路中第 `i` 路的位置为 `(2i - (n - 1)) * 1000 / 2`：左侧为负，右侧为正，中心为 0。8 路阵列的位置依次为 `-3500, -2500, -1500, -500, 500, 1500, 2500, 3500`。

`weighted_position` 是有效通道位置的平均值。只要有通道有效，`position` 等于 `weighted_position`，并被记忆。没有通道有效时 `line_lost` 置位，`position` 保持最近一次记忆的位置（没有记忆时为 0），`lost_side` 表示线最后出现在哪一侧，`lost_count` 统计连续丢线的读取次数。

`GreySensor::Sample` 的字段：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| `raw_mask` | `uint8_t` | 第 i 位为 1 表示第 i 路读到高电平 |
| `active_mask` | `uint8_t` | 第 i 位为 1 表示第 i 路有效 |
| `changed_mask` | `uint8_t` | `active_mask` 与上一次发布值的异或 |
| `channel_count` | `uint8_t` | 通道数 |
| `active_count` | `uint8_t` | 有效通道数 |
| `line_detected` | `uint8_t` | 有任一通道有效时为 1 |
| `line_lost` | `uint8_t` | 没有通道有效时为 1 |
| `lost_side` | `uint8_t` | 0 未知，1 左侧，2 右侧，仅丢线时有意义 |
| `weighted_position` | `int16_t` | 有效通道位置的平均值 |
| `position` | `int16_t` | `weighted_position`，丢线时为记忆位置 |
| `remembered_position` | `int16_t` | 最近一次检测到线时的位置 |
| `lost_count` | `uint32_t` | 连续丢线读取次数，仅丢线时有意义 |
| `sequence` | `uint32_t` | 每次发布加 1 |
| `raw` | `std::array<uint8_t, 8>` | 每路读到的电平，1 为高 |
| `active` | `std::array<uint8_t, 8>` | 每路是否有效，1 为有效 |

The position of channel `i` out of `n` is `(2i - (n - 1)) * 1000 / 2`: negative on the left, positive on the right, and 0 at the centre. For an 8-channel array the positions are `-3500, -2500, -1500, -500, 500, 1500, 2500, 3500`.

`weighted_position` is the mean position of the active channels. While at least one channel is active, `position` equals `weighted_position` and is remembered. When no channel is active, `line_lost` is set, `position` keeps the last remembered position (0 if there is none), `lost_side` tells on which side the line was last seen, and `lost_count` counts consecutive lost reads.

The fields of `GreySensor::Sample`:

| Field | Type | Meaning |
| --- | --- | --- |
| `raw_mask` | `uint8_t` | Bit i is 1 when channel i reads high |
| `active_mask` | `uint8_t` | Bit i is 1 when channel i is active |
| `changed_mask` | `uint8_t` | `active_mask` XOR the previously published one |
| `channel_count` | `uint8_t` | Number of channels |
| `active_count` | `uint8_t` | Number of active channels |
| `line_detected` | `uint8_t` | 1 when any channel is active |
| `line_lost` | `uint8_t` | 1 when no channel is active |
| `lost_side` | `uint8_t` | 0 unknown, 1 left, 2 right; meaningful only while the line is lost |
| `weighted_position` | `int16_t` | Mean of the active channel positions |
| `position` | `int16_t` | `weighted_position`, or the remembered position while the line is lost |
| `remembered_position` | `int16_t` | Position at the last detection of the line |
| `lost_count` | `uint32_t` | Consecutive lost reads; meaningful only while the line is lost |
| `sequence` | `uint32_t` | Increments on every publish |
| `raw` | `std::array<uint8_t, 8>` | Level read on each channel, 1 for high |
| `active` | `std::array<uint8_t, 8>` | Whether each channel is active, 1 for active |

## 3. 构造接口 / Constructor

```cpp
GreySensor(std::initializer_list<LibXR::GPIO*> channels, bool active_low = false,
           const char* topic_name = "grey_sensor", uint32_t publish_period_ms = 10);
```

依赖：

- `channels`：`LibXR::GPIO` 输入的指针，按从左到右排列，取自 BSP 的硬件注册（`XR_REGISTER`）；列表长度（1 到 8）决定通道数。

配置参数：

- `active_low`：为 `true` 时低电平表示该通道检测到线，默认 `false`。
- `topic_name`：发布的 Topic 名称，默认 `"grey_sensor"`。
- `publish_period_ms`：最小发布间隔，单位 ms，默认 10；为 0 时每次 monitor 调用都发布。

Dependencies:

- `channels`: pointers to the `LibXR::GPIO` inputs, ordered from left to right and taken from the BSP's Registration (`XR_REGISTER`); the list length (1 to 8) sets the channel count.

Configuration parameters:

- `active_low`: when `true`, a low level means the channel sees the line, default `false`.
- `topic_name`: name of the published Topic, default `"grey_sensor"`.
- `publish_period_ms`: minimum publish interval in ms, default 10; 0 publishes on every monitor call.

## 4. Topic

| Topic | 方向 | 类型 | 说明 |
| --- | --- | --- | --- |
| `topic_name`（默认 `grey_sensor`） | 发布 | `GreySensor::Sample` | 循迹样本，字段见第 2 节 |

| Topic | Direction | Type | Meaning |
| --- | --- | --- | --- |
| `topic_name` (default `grey_sensor`) | Publish | `GreySensor::Sample` | Line-tracking sample, fields in section 2 |

## 5. 配置示例 / Configuration Example

`xrobot instance add xrobot-org/GreySensor` 写入的实例，`channels` 的每一项填写 BSP 通过 `XR_REGISTER`（硬件注册）注册的 GPIO 对象地址，按从左到右排列；该参数的类型是 `LibXR::GPIO*`，因此写作带引号的 `&名称`：

An instance written by `xrobot instance add xrobot-org/GreySensor`, with each item of `channels` set to the address of a GPIO object registered by the BSP with `XR_REGISTER` (Registration), ordered from left to right; the parameter has the type `LibXR::GPIO*`, so each item is written as a quoted `&name`:

```yaml
modules:
  - module: xrobot-org/GreySensor
    id: greysensor_0
    args:
      - channels:
          - '&grey_0'
          - '&grey_1'
          - '&grey_2'
          - '&grey_3'
          - '&grey_4'
          - '&grey_5'
          - '&grey_6'
          - '&grey_7'
      - active_low: false
      - topic_name: "grey_sensor"
      - publish_period_ms: 10
```

## 6. 依赖与硬件 / Dependencies and Hardware

依赖：LibXR。

硬件：1 到 8 路数字输出的灰度（反射式光电）传感器，每路连接一个 GPIO，由 BSP 配置并通过 `XR_REGISTER` 注册。

Dependencies: LibXR.

Hardware: a grey (reflective photoelectric) sensor array with 1 to 8 digital outputs, each wired to one GPIO that the BSP registers with `XR_REGISTER`.

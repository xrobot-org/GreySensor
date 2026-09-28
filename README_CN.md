# GreySensor

[English](README.md)

面向循迹和反射式光电阵列的数字灰度传感器模块。

模块按传入顺序（从左到右）读取 1-8 路数字 GPIO 通道，把结果整理成紧凑的循迹样本，并通过
LibXR Topic 发布。模块不绑定板级引脚命名：BSP 注册 GPIO 对象，实例配置中列出它们。

## 行为

- 构造函数断言通道数为 1-8 且指针非空，并把每个 GPIO 配置为无上下拉输入。
- 通道读到高电平即为有效；`active_low` 为 true 时低电平有效。
- `n` 路中第 `i` 路的位置刻度为 `(2i - (n - 1)) * 1000 / 2`：左侧为负，右侧为正，中心为 0。
  8 路阵列为 `-3500, -2500, -1500, -500, 500, 1500, 2500, 3500`。
- `weighted_position` 是有效通道位置的平均值。只要有通道有效，`position` 就等于它，并被记忆。
- 没有通道有效时 `line_lost` 置位，`position` 保持最近一次记忆的位置（没有记忆时为 0），
  `lost_side` 表示线最后出现在哪一侧，`lost_count` 统计连续丢线的读取次数。
- `OnMonitor()` 最多每 `publish_period_ms` 毫秒发布一次样本（为 0 时每次 monitor 调用都发布），
  因此实际发布频率同时受 monitor 循环周期限制。

## Topic

| Topic | 类型 |
| --- | --- |
| `topic_name`（默认 `grey_sensor`） | `GreySensor::Sample` |

```cpp
struct Sample {
  uint8_t raw_mask;             // 第 i 位 = 第 i 路读到高电平
  uint8_t active_mask;          // 第 i 位 = 第 i 路有效
  uint8_t changed_mask;         // active_mask 与上一次发布值的异或
  uint8_t channel_count;
  uint8_t active_count;
  uint8_t line_detected;        // 有任一通道有效时为 1
  uint8_t line_lost;            // 没有通道有效时为 1
  uint8_t lost_side;            // 0 未知，1 左侧，2 右侧（仅丢线时）
  int16_t weighted_position;    // 有效通道位置的平均值
  int16_t position;             // weighted_position，丢线时为记忆位置
  int16_t remembered_position;  // 最近一次检测到线时的位置
  uint32_t lost_count;          // 连续丢线读取次数（仅丢线时）
  uint32_t sequence;            // 每次发布加 1
  std::array<uint8_t, MAX_CHANNEL_COUNT> raw;
  std::array<uint8_t, MAX_CHANNEL_COUNT> active;
};
```

## 公共接口

- `Sample Read()`：读取所有通道并更新位置状态（不发布；`changed_mask` 和 `sequence` 为 0）。
- `uint8_t ReadRawMask() const`、`uint8_t ReadActiveMask() const`：只读取掩码。
- `int16_t ReadPosition()`：`Read().position`。
- `size_t ChannelCount() const`。

`Read()` 和 `ReadPosition()` 与发布的样本共用位置记忆和丢线计数。

## 依赖

无其他模块依赖，仅使用 LibXR。

## 构造接口

```cpp
GreySensor(std::initializer_list<LibXR::GPIO*> channels, bool active_low = false,
           const char* topic_name = "grey_sensor", uint32_t publish_period_ms = 10);
```

依赖：

- `channels`：从左到右排列的 `LibXR::GPIO` 输入指针；列表长度（1-8）决定通道数。

配置：

- `active_low`：低电平表示检测到线，默认 `false`。
- `topic_name`：发布的 Topic 名称，默认 `"grey_sensor"`。
- `publish_period_ms`：最小发布间隔，单位 ms，默认 10；为 0 时每次 monitor 调用都发布。

## 使用

```sh
xrobot module add xrobot-org/GreySensor
xrobot setup
xrobot instance add xrobot-org/GreySensor
```

`xrobot instance add` 在 `User/xrobot.yaml` 中写入一个实例，依赖项留空，默认值按源码写出；
把 `channels` 填为 BSP 中用 `XR_REGISTER` 注册的 GPIO 对象的地址，每路一项、从左到右
（参数类型为 `LibXR::GPIO*`，所以要写带引号的 `&`）：

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
      - active_low: 'false'
      - topic_name: '"grey_sensor"'
      - publish_period_ms: '10'
```

BSP 侧：

```cpp
XR_REGISTER(grey_0, LibXR::GPIO);
XR_REGISTER(grey_1, LibXR::GPIO);
XR_REGISTER(grey_2, LibXR::GPIO);
XR_REGISTER(grey_3, LibXR::GPIO);
XR_REGISTER(grey_4, LibXR::GPIO);
XR_REGISTER(grey_5, LibXR::GPIO);
XR_REGISTER(grey_6, LibXR::GPIO);
XR_REGISTER(grey_7, LibXR::GPIO);
```

填好后再次运行 `xrobot setup`，生成 `User/xrobot_main.hpp`。

`xrobot module show .`（在本仓库中）或 `xrobot module show Modules/xrobot-org/GreySensor`
（在 BSP 中）打印 manifest 和当前的构造函数。

# GreySensor

[中文](README_CN.md)

XRobot Module for digital grey (reflective line-tracking) sensor arrays.

The Module reads 1 to 8 digital GPIO channels in the order they are passed (left to
right), turns them into a compact line-tracking sample and publishes it through a
LibXR topic. It does not depend on board pin names: the BSP registers the GPIO
objects and the instance lists them.

## Behaviour

- The constructor asserts that 1 to 8 non-null channels are given and configures
  each GPIO as input without pull.
- A channel is active when it reads high, or low when `active_low` is true.
- Channel `i` of `n` has the position `(2i - (n - 1)) * 1000 / 2`: negative on the
  left, positive on the right, 0 at the centre. For 8 channels this is
  `-3500, -2500, -1500, -500, 500, 1500, 2500, 3500`.
- `weighted_position` is the mean position of the active channels. While at least
  one channel is active, `position` equals it and is remembered.
- When no channel is active, `line_lost` is set, `position` keeps the last remembered
  position (0 if there is none), `lost_side` tells on which side the line was last
  seen and `lost_count` counts consecutive lost reads.
- `OnMonitor()` publishes a sample at most every `publish_period_ms` ms (every
  monitor call when 0), so the actual rate is also bounded by the monitor loop.

## Topic

| Topic | Type |
| --- | --- |
| `topic_name` (default `grey_sensor`) | `GreySensor::Sample` |

```cpp
struct Sample {
  uint8_t raw_mask;             // bit i = channel i reads high
  uint8_t active_mask;          // bit i = channel i active
  uint8_t changed_mask;         // active_mask XOR the previously published one
  uint8_t channel_count;
  uint8_t active_count;
  uint8_t line_detected;        // 1 when any channel is active
  uint8_t line_lost;            // 1 when no channel is active
  uint8_t lost_side;            // 0 unknown, 1 left, 2 right (only while lost)
  int16_t weighted_position;    // mean of active channel positions
  int16_t position;             // weighted_position, or remembered one while lost
  int16_t remembered_position;  // last position with the line detected
  uint32_t lost_count;          // consecutive lost reads (only while lost)
  uint32_t sequence;            // increments on every publish
  std::array<uint8_t, MAX_CHANNEL_COUNT> raw;
  std::array<uint8_t, MAX_CHANNEL_COUNT> active;
};
```

## Public API

- `Sample Read()`: read all channels and update the position state (without
  publishing; `changed_mask` and `sequence` stay 0).
- `uint8_t ReadRawMask() const`, `uint8_t ReadActiveMask() const`: read the masks
  only.
- `int16_t ReadPosition()`: `Read().position`.
- `size_t ChannelCount() const`.

`Read()` and `ReadPosition()` share the position memory and lost counter with the
published samples.

## Dependencies

No other Modules; LibXR only.

## Constructor

```cpp
GreySensor(std::initializer_list<LibXR::GPIO*> channels, bool active_low = false,
           const char* topic_name = "grey_sensor", uint32_t publish_period_ms = 10);
```

Dependencies:

- `channels`: pointers to the `LibXR::GPIO` inputs, ordered from left to right; the
  list length (1 to 8) sets the channel count.

Configuration:

- `active_low`: a low level means the channel sees the line, default `false`.
- `topic_name`: name of the published topic, default `"grey_sensor"`.
- `publish_period_ms`: minimum publish interval in ms, default 10; 0 publishes on
  every monitor call.

## Use

```sh
xrobot module add xrobot-org/GreySensor
xrobot setup
xrobot instance add xrobot-org/GreySensor
```

`xrobot instance add` writes an instance to `User/xrobot.yaml` with empty
dependencies and the source defaults; fill `channels` with the addresses of GPIO
objects the BSP registers with `XR_REGISTER`, one list item per channel from left to
right (the parameter takes `LibXR::GPIO*`, hence the quoted `&`):

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

BSP side:

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

Run `xrobot setup` again to generate `User/xrobot_main.hpp`.

`xrobot module show .` in this repository, or
`xrobot module show Modules/xrobot-org/GreySensor` in a BSP, prints the manifest and
the current constructor.

# GreySensor

## Static assembly source line

This source line uses explicit C++ constructor dependencies and ordered instance
arguments. Inspect the current primary header with `xrobot_mod_parser --path .`;
its declarations, not old manifest/config examples, define the interface.
Historical HardwareContainer/ApplicationManager examples below apply only to the
older dynamic source tags. Device/protocol descriptions remain relevant.
See the XRobot [migration guide](https://github.com/xrobot-org/XRobot/blob/dev/MIGRATION.md).
Compilation is not hardware validation; retain version-specific board evidence.


Digital grey sensor array module for XRobot.

This module reads runtime-configurable digital GPIO channels in the order
provided by the application layer, converts them into a compact line-tracking
sample, and publishes raw state, active state, weighted position, line-loss
state, and position memory through a LibXR topic.

The module stays independent from board-level pin names. The board or User layer
registers GPIO aliases, then passes those alias strings in `channel_names`.

## Required Hardware

- `grey_sensor_gpio_0`
- `grey_sensor_gpio_1`
- `grey_sensor_gpio_2`
- `grey_sensor_gpio_3`
- `grey_sensor_gpio_4`
- `grey_sensor_gpio_5`
- `grey_sensor_gpio_6`
- `grey_sensor_gpio_7`

The GPIO names are constructor arguments, so projects may use other hardware
aliases and channel counts up to `GreySensor::MAX_CHANNEL_COUNT`.

## Constructor Arguments

- `channel_names`: GPIO alias list ordered from left to right
- `active_low`: default `false`
- `topic_name`: default `"grey_sensor"`
- `publish_period_ms`: default `10`

## Published Topics

- `topic_name`: `GreySensor::Sample`, including raw / active masks, changed mask, active count, weighted position, remembered position, lost-line side, lost-line count, and sequence number

## XRobot Configuration Example

```yaml
- id: grey_sensor
  name: GreySensor
  constructor_args:
    channel_names:
      - "grey_sensor_gpio_0"
      - "grey_sensor_gpio_1"
      - "grey_sensor_gpio_2"
      - "grey_sensor_gpio_3"
      - "grey_sensor_gpio_4"
      - "grey_sensor_gpio_5"
      - "grey_sensor_gpio_6"
      - "grey_sensor_gpio_7"
    active_low: false
    topic_name: "grey_sensor"
    publish_period_ms: 10
```

#pragma once

#include <array>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/binary_sensor/binary_sensor.h"

namespace esphome {
namespace fchao_inverter {

class FchaoInverterComponent : public PollingComponent, public uart::UARTDevice {
 public:
  void setup() override;
  void loop() override;
  void update() override;
  void dump_config() override;
  
  void publish_nan() { this->publish_nan_(); }

  void set_flow_control_pin(GPIOPin *pin) { this->flow_control_pin_ = pin; }
  void set_rx_timeout(uint32_t rx_timeout_ms) { this->rx_timeout_ms_ = rx_timeout_ms; }
  void set_data_timeout(uint32_t data_timeout_ms) { this->data_timeout_ms_ = data_timeout_ms; }
  void set_send_request(bool send_request) { this->send_request_enabled_ = send_request; }

  void set_ac_voltage_sensor(sensor::Sensor *s) { this->ac_voltage_sensor_ = s; }
  void set_power_sensor(sensor::Sensor *s) { this->power_sensor_ = s; }
  void set_dc_voltage_sensor(sensor::Sensor *s) { this->dc_voltage_sensor_ = s; }
  void set_temperature_sensor(sensor::Sensor *s) { this->temperature_sensor_ = s; }
  void set_overload_sensor(binary_sensor::BinarySensor *s) { this->overload_sensor_ = s; }

 protected:
  static constexpr size_t FRAME_LEN = 17;
  static constexpr uint8_t RESPONSE_HEADER[3] = {0xAE, 0x01, 0x12};
  static constexpr uint8_t FRAME_END = 0xEE;
  static constexpr uint8_t REQUEST_FRAME[6] = {0xAE, 0x01, 0x01, 0x03, 0x05, 0xEE};

  void send_request_();
  bool parse_byte_(uint8_t byte);
  void on_frame_();
  void publish_nan_();

  GPIOPin *flow_control_pin_{nullptr};
  uint32_t rx_timeout_ms_{200};
  uint32_t data_timeout_ms_{5000};
  bool send_request_enabled_{true};

  sensor::Sensor *ac_voltage_sensor_{nullptr};
  sensor::Sensor *power_sensor_{nullptr};
  sensor::Sensor *dc_voltage_sensor_{nullptr};
  sensor::Sensor *temperature_sensor_{nullptr};
  binary_sensor::BinarySensor *overload_sensor_{nullptr};

  std::array<uint8_t, FRAME_LEN> rx_buffer_{};
  size_t rx_size_{0};

  uint32_t last_byte_ms_{0};
  uint32_t last_valid_ms_{0};
  bool timed_out_{false};
};

}  // namespace fchao_inverter
}  // namespace esphome
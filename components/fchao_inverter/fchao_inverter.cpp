#include "fchao_inverter.h"
#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include <cmath>
 
namespace esphome {
namespace fchao_inverter {

static const char *const TAG = "fchao_inverter";

namespace {
int bcd2(uint8_t hi, uint8_t lo) {
  return ((hi >> 4) * 10 + (hi & 0x0F)) * 100 + ((lo >> 4) * 10 + (lo & 0x0F));
}
}  // namespace

void FchaoInverterComponent::setup() {
  this->check_uart_settings(9600);

  if (this->flow_control_pin_ != nullptr) {
    this->flow_control_pin_->setup();
    this->flow_control_pin_->digital_write(false);
  }
}

void FchaoInverterComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Fchao Inverter:");
  LOG_UPDATE_INTERVAL(this);
  LOG_PIN("  Flow Control Pin: ", this->flow_control_pin_);
  ESP_LOGCONFIG(TAG, "  RX Timeout: %u ms", this->rx_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Data Timeout: %u ms", this->data_timeout_ms_);
  ESP_LOGCONFIG(TAG, "  Send Request: %s", YESNO(this->send_request_enabled_));
  LOG_SENSOR("  ", "Voltage", this->voltage_sensor_);
  LOG_SENSOR("  ", "Power", this->power_sensor_);
  LOG_SENSOR("  ", "Battery Voltage", this->battery_voltage_sensor_);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
}

void FchaoInverterComponent::update() {
  if (this->send_request_enabled_) {
    this->send_request_();
  }
}

void FchaoInverterComponent::send_request_() {
  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(true);

  this->write_array(REQUEST_FRAME, sizeof(REQUEST_FRAME));
  this->flush();

  if (this->flow_control_pin_ != nullptr)
    this->flow_control_pin_->digital_write(false);
}

void FchaoInverterComponent::loop() {
  const uint32_t now = millis();

  if (this->rx_size_ > 0 && (now - this->last_byte_ms_ > this->rx_timeout_ms_)) {
    ESP_LOGVV(TAG, "Buffer cleared due to timeout (%zu bytes)", this->rx_size_);
    this->rx_size_ = 0;
  }

  while (this->available()) {
    uint8_t byte;
    this->read_byte(&byte);
    this->last_byte_ms_ = now;

    if (!this->parse_byte_(byte)) {
      this->rx_size_ = 0;
    }
  }

  if (this->last_valid_ms_ != 0 && (now - this->last_valid_ms_ > this->data_timeout_ms_) && !this->timed_out_) {
    ESP_LOGW(TAG, "No valid frame received in %u ms", this->data_timeout_ms_);
    this->publish_nan_();
    this->timed_out_ = true;
  }
}

bool FchaoInverterComponent::parse_byte_(uint8_t byte) {
  const size_t at = this->rx_size_;
  
  if (at >= FRAME_LEN) {
    return false;
  }

  this->rx_buffer_[this->rx_size_++] = byte;

  if (at < sizeof(RESPONSE_HEADER)) {
    return byte == RESPONSE_HEADER[at];
  }

  if (at < FRAME_LEN - 1)
    return true;

  if (byte != FRAME_END) {
    ESP_LOGW(TAG, "Invalid frame end: 0x%02X", byte);
    return false;
  }

  this->on_frame_();
  return false;
}

void FchaoInverterComponent::on_frame_() {
  ESP_LOGD(TAG, "Packet: %s", format_hex_pretty(this->rx_buffer_.data(), this->rx_buffer_.size()).c_str());

  int volt_ac = bcd2(this->rx_buffer_[4], this->rx_buffer_[5]);
  int power = bcd2(this->rx_buffer_[6], this->rx_buffer_[7]);
  int volt_batt = bcd2(this->rx_buffer_[8], this->rx_buffer_[9]);
  int temp = bcd2(this->rx_buffer_[10], this->rx_buffer_[11]);

  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(volt_ac);
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(power);
  if (this->battery_voltage_sensor_ != nullptr)
    this->battery_voltage_sensor_->publish_state(volt_batt / 10.0f);
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(temp);

  this->last_valid_ms_ = millis();
  this->timed_out_ = false;
}

void FchaoInverterComponent::publish_nan_() {
  if (this->voltage_sensor_ != nullptr)
    this->voltage_sensor_->publish_state(NAN);
  if (this->power_sensor_ != nullptr)
    this->power_sensor_->publish_state(NAN);
  if (this->battery_voltage_sensor_ != nullptr)
    this->battery_voltage_sensor_->publish_state(NAN);
  if (this->temperature_sensor_ != nullptr)
    this->temperature_sensor_->publish_state(NAN);
}

}  // namespace fchao_inverter
}  // namespace esphome
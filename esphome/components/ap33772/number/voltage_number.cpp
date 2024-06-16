#include "voltage_number.h"

namespace esphome {
namespace ap33772 {

static const char *const TAG = "ap33772.number.voltage";

void esphome::ap33772::VoltageNumber::dump_config() { LOG_NUMBER(TAG, "AP33772 Voltage Number", this); }

void VoltageNumber::setup() {
  float v;
  if (this->restore_) {
    this->rtc_ = global_preferences->make_preference<float>(this->get_object_id_hash());
    if (this->rtc_.load(&v)) {
      ESP_LOGD(TAG, "Voltage value restored: %f", v);
      this->control(v);
    }
  }
}

void VoltageNumber::control(float value) {
  this->publish_state(value);
  this->parent_->request_voltage(value);
  if (this->restore_) {
    ESP_LOGD(TAG, "Saving voltage value: %f", value);
    this->rtc_.save(&value);
  }
}

}  // namespace ap33772
}  // namespace esphome
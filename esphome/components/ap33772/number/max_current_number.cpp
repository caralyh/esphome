#include "max_current_number.h"

namespace esphome {
namespace ap33772 {

static const char *const TAG = "ap33772.number.max_current";

void esphome::ap33772::MaxCurrentNumber::dump_config() { LOG_NUMBER(TAG, "AP33772 Max Current Number", this); }

void MaxCurrentNumber::setup() {
  float v;
  if (this->restore_) {
    this->rtc_ = global_preferences->make_preference<float>(this->get_object_id_hash());
    if (this->rtc_.load(&v)) {
      this->control(v);
    }
  }
}

void MaxCurrentNumber::control(float value) {
  this->publish_state(value);
  this->parent_->request_max_current(value);
  if (this->restore_) {
    this->rtc_.save(&value);
  }
}

}  // namespace ap33772
}  // namespace esphome
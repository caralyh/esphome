#pragma once

#include "esphome/components/number/number.h"
#include "esphome/core/preferences.h"
#include "../ap33772.h"

namespace esphome {
namespace ap33772 {

class CurrentNumber : public number::Number, public Component, public Parented<AP33772Component> {
 public:
  CurrentNumber() = default;
  void dump_config() override;
  void setup() override;
  void set_restore(bool restore) { this->restore_ = restore; }

 protected:
  void control(float value) override;
  bool restore_{false};
  ESPPreferenceObject rtc_;
};

}  // namespace ap33772
}  // namespace esphome

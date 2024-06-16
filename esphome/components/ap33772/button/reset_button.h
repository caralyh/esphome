#pragma once

#include "esphome/components/button/button.h"
#include "esphome/components/number/number.h"
#include "../ap33772.h"

namespace esphome {
namespace ap33772 {

class ResetButton : public button::Button, public Parented<AP33772Component> {
 public:
  ResetButton() = default;

 protected:
  void press_action() override { this->parent_->reset(); };
};

}  // namespace ap33772
}  // namespace esphome

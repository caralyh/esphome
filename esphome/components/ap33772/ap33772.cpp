#include <sstream>
#include <iostream>
#include <iomanip>
#include "ap33772.h"
#include "pdo.h"
#include "esphome/core/log.h"
#include "esphome/core/application.h"

namespace esphome {
namespace ap33772 {

static const int SRCPDO_LENGTH = 28;

static const char *const TAG = "ap33772";

static const std::string NO_PD_STATE = "None";

static bool isEmptyRdo(RdoData rdo) { return rdo.byte0 == 0 && rdo.byte1 == 0 && rdo.byte2 == 0 && rdo.byte3 == 0; }

static bool checkI2CError(i2c::ErrorCode err) {
  if (err == i2c::ERROR_OK) {
    return false;
  }

  if (err == i2c::ERROR_NOT_INITIALIZED) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_NOT_INITIALIZED");
  } else if (err == i2c::ERROR_INVALID_ARGUMENT) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_INVALID_ARGUMENT");
  } else if (err == i2c::ERROR_NOT_ACKNOWLEDGED) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_NOT_ACKNOWLEDGED");
  } else if (err == i2c::ERROR_TIMEOUT) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_TIMEOUT");
  } else if (err == i2c::ERROR_TOO_LARGE) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_TOO_LARGE");
  } else if (err == i2c::ERROR_CRC) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_CRC");
  } else if (err == i2c::ERROR_UNKNOWN) {
    ESP_LOGE(TAG, "Failed to read status: ERROR_UNKNOWN");
  }

  return true;
}
// bool Pdo::is_fixed() { return (this->data.byte3 & 0xC0) == 0x00; }

// bool Pdo::is_pps() { return (this->data.byte3 & 0xF0) == 0xC0; }

// bool Pdo::can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current) {
//   if (this->is_fixed()) {
//     if (current > 0 && this->data.fixed.maxCurrent * 10 < current) {
//       return false;
//     }
//     if (current <= 0 && max_current > 0 && this->data.fixed.maxCurrent * 10 < max_current) {
//       return false;
//     }
//     // TODO: configurable tolerance/range for fixed voltage selection
//     if (this->data.fixed.voltage * 50 > voltage) {
//       return false;
//     }
//     return true;
//   } else if (this->is_pps()) {
//     if (voltage > 0) {
//       if (this->data.pps.minVoltage * 100 > voltage) {
//         return false;
//       }
//       if (this->data.pps.maxVoltage * 100 < voltage) {
//         return false;
//       }
//     }
//     if (current > 0 && current > this->data.pps.maxCurrent * 50) {
//       return false;
//     }
//     if (current <= 0 && max_current > 0 && this->data.pps.maxCurrent * 50 < max_current) {
//       return false;
//     }
//     return true;
//   }
//   return false;
// }

// RdoData Pdo::create_rdo(unsigned int position, unsigned int voltage, unsigned int current, unsigned int max_current)
// {
//   RdoData rdo;
//   if (this->is_fixed()) {
//     rdo.fixed.objPosition = position;
//     if (current > 0) {
//       rdo.fixed.opCurrent = current / 10;
//     }
//     if (max_current > 0) {
//       if (max_current > this->data.fixed.maxCurrent * 10) {
//         rdo.fixed.maxCurrent = this->data.fixed.maxCurrent;
//       } else {
//         rdo.fixed.maxCurrent = max_current / 10;
//       }
//     }
//   } else if (this->is_pps()) {
//     rdo.pps.objPosition = position;
//     if (voltage > 0) {
//       rdo.pps.voltage = voltage / 20;
//     }
//     if (current > 0) {
//       rdo.pps.opCurrent = current / 50;
//     }
//   }
//   return rdo;
// }

// std::string Pdo::type_name() {
//   if (this->is_fixed()) {
//     return "FIXED";
//   } else if (this->is_pps()) {
//     return "PPS";
//   }
//   return "";
// }

// std::string Pdo::description() {
//   std::stringstream ss;
//   ss << std::fixed << std::setprecision(2);
//   if (this->is_fixed()) {
//     ss << this->data.fixed.voltage * 50 / 1000 << "V @ ";
//     ss << this->data.fixed.maxCurrent * 10 / 1000 << "A";
//   } else if (this->is_pps()) {
//     ss << this->data.pps.minVoltage * 100 / 1000 << "V~";
//     ss << this->data.pps.maxVoltage * 100 / 1000 << "V @ ";
//     ss << this->data.pps.maxCurrent * 50 / 1000 << "A";
//   }
//   return ss.str();
// }

// std::string Pdo::pdo_description() {
//   if (!this->is_fixed() && !this->is_pps()) {
//     return "INVALID PDO";
//   }

//   std::stringstream ss;
//   ss << "[" << this->type_name() << "] " << this->description();
//   return ss.str();
// }

void AP33772Component::attach_interrupt_(InternalGPIOPin *irq_pin, esphome::gpio::InterruptType type) {
  irq_pin->attach_interrupt(AP33772Component::interrupt_handler_, this, type);
  this->interrup_enabled_ = true;
  ESP_LOGD(TAG, "Attach PD Interrupt");
}

void AP33772Component::write_mask_() { this->reg(Register::MASK) = this->interrupt_mask_; }

void AP33772Component::interrupt_handler_(AP33772Component *args) {}

#ifdef USE_NUMBER
void AP33772Component::set_number_by_name(std::string name, number::Number *number) {
  if (name.compare("voltage") == 0) {
    this->set_voltage_number(number);
  } else if (name.compare("current") == 0) {
    this->set_current_number(number);
  } else if (name.compare("max_current") == 0) {
    this->set_max_current_number(number);
  }
}
#endif

void AP33772Component::perform_initial_negotiation_() {
  this->initial_negotiation_ = InitialNegotiationStatus::WAITING;
  this->set_retry(
      "initial_nego", 10, 10,
      [this](const uint8_t remaining_attempts) {
        ESP_LOGD(TAG, "RETRIES LEFT: %d", remaining_attempts);
        this->read_status_();

        ESP_LOGD(TAG, "Ready: %s, Success: %s, New PDO: %s", YESNO(!!this->status_.isReady),
                 YESNO(!!this->status_.isSuccess), YESNO(!!this->status_.isNewpdo));

        if (this->status_.isReady && this->status_.isSuccess && this->status_.isNewpdo) {
          this->read_pdos_();
          this->initial_negotiation_ = InitialNegotiationStatus::SUCCESS;
          this->set_reset_count_(0);
          return RetryResult::DONE;
        } else {
          if (remaining_attempts == 0) {
            if (this->status_.isReady && this->status_.isSuccess) {
              this->initial_negotiation_ = InitialNegotiationStatus::NO_PDOS;
            } else if (this->status_.isReady) {
              this->initial_negotiation_ = InitialNegotiationStatus::NEGO_FAILURE;
            } else {
              this->initial_negotiation_ = InitialNegotiationStatus::NO_RESPONSE;
            }

            this->reset_once_();
          }
          return RetryResult::RETRY;
        }
      },
      2.0f);
}

void AP33772Component::setup() {
  this->init_reset_count_();
  this->pdos_.reserve(8);

  // for (uint8_t i = 0; i < 7; i++) {
  //   this->source_pdos_[i] = Pdo();
  // }

#ifdef USE_BINARY_SENSOR
  if (this->negotiation_success_binary_sensor_ != nullptr) {
    this->negotiation_success_binary_sensor_->publish_initial_state(false);
  }
#endif

#ifdef USE_TEXT_SENSOR
  if (this->current_pdo_text_sensor_ != nullptr) {
    this->current_pdo_text_sensor_->publish_state(NO_PD_STATE);
  }
#endif

  if (this->interrupt_pin_ != nullptr) {
    this->write_mask_();
    this->interrupt_pin_->setup();
    this->attach_interrupt_(this->interrupt_pin_, gpio::INTERRUPT_HIGH_LEVEL);
  } else {
    this->perform_initial_negotiation_();
  }
}
void AP33772Component::dump_config() {
  ESP_LOGCONFIG(TAG, "AP33772:");
  LOG_I2C_DEVICE(this);
  ESP_LOGCONFIG(TAG, "PD reset count: %d", this->pd_reset_count_);

  ESP_LOGCONFIG(TAG, "Current status:");
  ESP_LOGCONFIG(TAG, "Ready: %s", YESNO(!!this->status_.isReady));
  if (this->status_.isReady) {
    ESP_LOGCONFIG(TAG, "Negotiation Successfull: %s", YESNO(!!this->status_.isSuccess));
    ESP_LOGCONFIG(TAG, "New PDOs received: %s", YESNO(!!this->status_.isNewpdo));
  }
  ESP_LOGCONFIG(TAG, "Over Voltage Detected: %s", YESNO(!!this->status_.isOvp));
  ESP_LOGCONFIG(TAG, "Over Current Detected: %s", YESNO(!!this->status_.isOcp));
  ESP_LOGCONFIG(TAG, "Over Temperature Detected: %s", YESNO(!!this->status_.isOtp));
  ESP_LOGCONFIG(TAG, "Derating enabled: %s", YESNO(!!this->status_.isDr));
  ESP_LOGCONFIG(TAG, "Available PDOS: %d", (int) this->available_pdo_count_);

  if (this->available_pdo_count_ > 0) {
    ESP_LOGCONFIG(TAG, "PDOs:");
    for (Pdo pdo : this->pdos_) {
      ESP_LOGCONFIG(TAG, "%s", pdo.pdo_description().c_str());
    }
    // for (uint8_t i = 0; i < this->available_pdo_count_; i++) {
    //   ESP_LOGCONFIG(TAG, "%s", this->source_pdos_[i].pdo_description().c_str());
    // }
  }
  ESP_LOGCONFIG(TAG, "Initial negotiation status: %s",
                InitialNegotiationStatusName[(int) this->initial_negotiation_].data());

  if (this->negotiation_ != RdoNegotiationStatus::NONE) {
    ESP_LOGCONFIG(TAG, "Requested PDO: %d", this->rdo_requested_);
    ESP_LOGCONFIG(TAG, "Negotiation status: %s", NegotiationStatusName[(int) this->negotiation_].data());
    ESP_LOGCONFIG(TAG, "Request values | voltage: %dmV, current: %dmA, max current: %dmA", this->requested_voltage_,
                  this->requested_current_, this->requested_max_current_);
  }
}

void AP33772Component::update() {
  if (this->temperature_sensor_ != nullptr) {
    int temp = this->read_temp();
    if (temp > 0) {
      this->temperature_sensor_->publish_state(temp);
    } else {
      ESP_LOGW(TAG, "Invalid temperature measurement");
    }
  }
  // Only one sensor reading at a time, defer rest so that individual update() call won't take too long
  if (this->voltage_sensor_ != nullptr) {
    if ((this->status_.isReady && this->status_.isSuccess) || !this->voltage_sensor_->has_state()) {
      this->defer([this]() {
        uint8_t voltage = this->read_voltage();
        ESP_LOGD(TAG, "Voltage read: %u", voltage);
        if (voltage > 0) {
          this->voltage_sensor_->publish_state(voltage * 80);  // mV -> V
        } else {
          this->voltage_sensor_->publish_state(0);
        }
      });
    }
  }
  if (this->current_sensor_ != nullptr) {
    if ((this->status_.isReady && this->status_.isSuccess) || !this->current_sensor_->has_state()) {
      this->defer([this]() {
        uint8_t current = this->read_current();
        ESP_LOGD(TAG, "Current read: %u", current);
        if (current > 0) {
          this->current_sensor_->publish_state(current * 24);  // mA -> A
        } else {
          this->current_sensor_->publish_state(0);
        }
      });
    }
  }
}

void AP33772Component::loop() {
  if (this->negotiation_ == RdoNegotiationStatus::UPDATE_REQUIRED) {
    this->request_rdo();
  }
}

void AP33772Component::request_voltage(float voltage) {
  this->requested_voltage_ = voltage;

#ifdef USE_NUMBER
  if (this->voltage_number_ != nullptr && this->voltage_number_->state != this->requested_voltage_) {
    this->voltage_number_->publish_state((float) this->requested_voltage_);
  }
#endif
  this->set_negotiation_state_(RdoNegotiationStatus::UPDATE_REQUIRED);
}

void AP33772Component::request_current(float current) {
  this->requested_current_ = current;

#ifdef USE_NUMBER
  if (this->current_number_ != nullptr && this->current_number_->state != this->requested_current_) {
    this->current_number_->publish_state((float) this->requested_current_);
  }
#endif
  this->set_negotiation_state_(RdoNegotiationStatus::UPDATE_REQUIRED);
}

void AP33772Component::request_max_current(float max_current) {
  this->requested_max_current_ = max_current;

#ifdef USE_NUMBER
  if (this->max_current_number_ != nullptr && this->max_current_number_->state != this->requested_max_current_) {
    this->max_current_number_->publish_state((float) this->requested_max_current_);
  }
#endif
  this->set_negotiation_state_(RdoNegotiationStatus::UPDATE_REQUIRED);
}

void AP33772Component::reset() {
  this->set_reset_count_(this->pd_reset_count_ + 1);
  uint8_t data[4] = {0};
  auto err = this->write_register(Register::RDO, data, 4);
  checkI2CError(err);
}

void AP33772Component::clear_request() {
  this->requested_voltage_ = 0;
  this->requested_current_ = 0;
  this->requested_max_current_ = 0;

#ifdef USE_NUMBER
  if (this->voltage_number_ != nullptr) {
    this->voltage_number_->make_call().set_value(0).perform();
  }
  if (this->current_number_ != nullptr) {
    this->current_number_->make_call().set_value(0).perform();
  }
  if (this->max_current_number_ != nullptr) {
    this->max_current_number_->make_call().set_value(0).perform();
  }
#endif
  this->set_reset_count_(0);
  this->set_timeout(2000, [this]() {
    ESP_LOGD(TAG, "Reset PD after clear");
    this->reset();
  });
}

uint8_t AP33772Component::read_voltage() {
  uint8_t voltage;
  if (checkI2CError(this->read_register(Register::VOLTAGE, &voltage, 1))) {
    voltage = 0;
  }
  return voltage;
}

uint8_t AP33772Component::read_current() {
  uint8_t current;
  if (checkI2CError(this->read_register(Register::CURRENT, &current, 1))) {
    current = 0;
  }
  return current;
}

void AP33772Component::reset_once_() {
  if (this->pd_reset_count_ < 3) {
    ESP_LOGD(TAG, "Reset PD (count: %d)", (int) this->pd_reset_count_);
    this->reset();
  }
}

void AP33772Component::init_reset_count_() {
  uint32_t reset_hash = fnv1_hash("ap33772_reset_state_" + App.get_compilation_time());
  this->reset_pref_ = global_preferences->make_preference<uint8_t>(reset_hash);
  this->reset_pref_.load(&this->pd_reset_count_);
}

void AP33772Component::set_reset_count_(uint8_t count) {
  this->reset_pref_.save(&count);
  global_preferences->sync();
}

void AP33772Component::set_negotiation_state_(RdoNegotiationStatus status) {
  this->negotiation_ = status;
#ifdef USE_BINARY_SENSOR
  if (this->negotiation_success_binary_sensor_ != nullptr) {
    if (status == RdoNegotiationStatus::SUCCESS) {
      this->negotiation_success_binary_sensor_->publish_state(true);
    } else if (this->negotiation_success_binary_sensor_->state) {
      this->negotiation_success_binary_sensor_->publish_state(false);
    }
  }
#endif
#ifdef USE_TEXT_SENSOR
  if (this->current_pdo_text_sensor_ != nullptr) {
    if (status == RdoNegotiationStatus::SUCCESS) {
      this->current_pdo_text_sensor_->publish_state(this->pdos_[this->rdo_requested_].pdo_description());
    } else {
      if (this->current_pdo_text_sensor_->state.compare(NO_PD_STATE) != 0) {
        this->current_pdo_text_sensor_->publish_state(NO_PD_STATE);
      }
    }
  }
#endif
}

void AP33772Component::request_rdo() {
  if (this->initial_negotiation_ != InitialNegotiationStatus::SUCCESS) {
    return;
  }
  if (!this->has_request_values_()) {
    this->cancel_retry("nego_result");
    this->set_negotiation_state_(RdoNegotiationStatus::NONE);
    return;
  }

  std::vector<Pdo> candidates;
  std::copy_if(this->pdos_.begin(), this->pdos_.end(), std::back_inserter(candidates), [this](Pdo pdo) {
    return pdo.can_satisfy_request(this->requested_voltage_, this->requested_current_, this->requested_max_current_);
  });

  // for (Pdo pdo : this->pdos_) {
  //   if (pdo.can_satisfy_request(this->requested_voltage_, this->requested_current_, this->requested_max_current_)) {
  //     pdo_index = pdo.position() - 1;
  //   }
  // }
  // for (uint8_t i = 0; i < this->available_pdo_count_; i++) {
  //   if (this->source_pdos_[i].can_satisfy_request(this->requested_voltage_, this->requested_current_,
  //                                                 this->requested_max_current_)) {
  //     pdo_index = i + 1;
  //     break;
  //   }
  // }
  if (candidates.empty()) {
    ESP_LOGD(TAG, "Skipping rdo request, could not find suitable PDO");
    this->set_negotiation_state_(RdoNegotiationStatus::NO_PDO_MATCH);
    return;
  }
  std::sort(candidates.begin(), candidates.end());

  RdoData rdo =
      candidates[0].create_rdo(this->requested_voltage_, this->requested_current_, this->requested_max_current_);

  if (isEmptyRdo(rdo)) {
    ESP_LOGD(TAG, "Skipping rdo request, could not find suitable PDO");
    this->set_negotiation_state_(RdoNegotiationStatus::NO_PDO_MATCH);
    return;
  }
  ESP_LOGI(TAG, "Request RDO, mark negotiation pending");

  this->cancel_retry("nego_result");
  this->set_negotiation_state_(RdoNegotiationStatus::WAITING);

  this->negotiation_complete_ = false;
  this->rdo_requested_ = candidates[0].position();
  this->write_rdo_(rdo);

  this->set_retry(
      "nego_result", 5, 20,
      [this](const uint8_t remaining_attempts) {
        this->read_status_();

        if (this->status_.isReady) {
          if (this->status_.isNewpdo) {
            ESP_LOGI(TAG, "New PDOs received, reading...");
            this->read_pdos_();
          }

          if (this->status_.isSuccess) {
            ESP_LOGI(TAG, "Negotiation success");
            this->negotiation_complete_ = true;
            this->set_negotiation_state_(RdoNegotiationStatus::SUCCESS);
            return RetryResult::DONE;
          }
        }

        if (remaining_attempts == 0) {
          this->set_negotiation_state_(RdoNegotiationStatus::FAILED);
        }
        return RetryResult::RETRY;
      },
      2.0f);
}

void AP33772Component::configure_ntc(int TR25, int TR50, int TR75, int TR100) {
  uint8_t data[2];
  data[0] = TR25 & 0xFF;
  data[1] = (TR25 >> 8) & 0xFF;
  this->write_register(Register::TR25, data, 2);

  data[0] = TR50 & 0xFF;
  data[1] = (TR50 >> 8) & 0xFF;
  this->write_register(Register::TR50, data, 2);

  data[0] = TR75 & 0xFF;
  data[1] = (TR75 >> 8) & 0xFF;
  this->write_register(Register::TR75, data, 2);

  data[0] = TR100 & 0xFF;
  data[1] = (TR100 >> 8) & 0xFF;
  this->write_register(Register::TR100, data, 2);
}

void AP33772Component::set_derating_temp(int temperature) { this->reg(Register::DRTHR) = temperature; }

void AP33772Component::set_over_current_threshold(int OCP_current) { this->reg(Register::OCPTHR) = OCP_current / 50; }

void AP33772Component::set_over_temp_threshold(int OTP_temperature) { this->reg(Register::OTPTHR) = OTP_temperature; }

void AP33772Component::set_mask(Mask flag) { this->reg(Register::MASK) |= (uint8_t) flag; }

void AP33772Component::clear_mask(Mask flag) { this->reg(Register::MASK) &= ~(uint8_t) flag; }

void AP33772Component::start_nego() {
  this->set_reset_count_(0);
  this->pd_reset_count_ = 0;
  this->perform_initial_negotiation_();
}

void AP33772Component::configure_mask(bool ready, bool success, bool newpdo, bool ovp, bool ocp, bool otp, bool dr) {
  uint8_t mask = 0;
  if (ready) {
    mask |= (uint8_t) Mask::READY_EN;
  }
  if (success) {
    mask |= (uint8_t) Mask::SUCCESS_EN;
  }
  if (newpdo) {
    mask |= (uint8_t) Mask::NEWPDO_EN;
  }
  if (ovp) {
    mask |= (uint8_t) Mask::OVP_EN;
  }
  if (ocp) {
    mask |= (uint8_t) Mask::OCP_EN;
  }
  if (otp) {
    mask |= (uint8_t) Mask::OTP_EN;
  }
  if (dr) {
    mask |= (uint8_t) Mask::DR_EN;
  }
  this->interrupt_mask_ = mask;
}

void AP33772Component::on_safe_shutdown() {
  // Reset on shutdown for fresh state at boot, and to not leave power on
  this->reset();
}

void AP33772Component::update_error_and_warning_states_() {
  if (this->status_.isOvp) {
    this->status_set_error("Over voltage protection triggered");
  }
  if (this->status_.isOcp) {
    this->status_set_error("Over current protection triggered");
  }
  if (this->status_.isOtp) {
    this->status_set_error("Over temperature protection triggered");
  }

  if (!(this->status_.isOvp || this->status_.isOcp || this->status_.isOtp) && this->status_has_error()) {
    this->status_clear_error();
  }

  if (this->status_.isDr) {
    this->status_set_warning("Temperature de-rating triggered");
  } else if (this->status_has_warning()) {
    this->status_clear_warning();
  }
}

void AP33772Component::read_status_() {
  // ESP_LOGI(TAG, "Read PD status");
  // Note: reading status register clears it
  // this->status_.readStatus = this->reg(Register::STATUS).get();
  auto err = this->read_register(Register::STATUS, &this->status_.readStatus, 1, false);
  if (err != i2c::ERROR_OK) {
    if (err == i2c::ERROR_NOT_INITIALIZED) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_NOT_INITIALIZED");
    } else if (err == i2c::ERROR_INVALID_ARGUMENT) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_INVALID_ARGUMENT");
    } else if (err == i2c::ERROR_NOT_ACKNOWLEDGED) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_NOT_ACKNOWLEDGED");
    } else if (err == i2c::ERROR_TIMEOUT) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_TIMEOUT");
    } else if (err == i2c::ERROR_TOO_LARGE) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_TOO_LARGE");
    } else if (err == i2c::ERROR_CRC) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_CRC");
    } else if (err == i2c::ERROR_UNKNOWN) {
      ESP_LOGE(TAG, "Failed to read status: ERROR_UNKNOWN");
    }

    return;
  }

  this->update_error_and_warning_states_();
}

void AP33772Component::update_request_number_ranges_() {
#ifdef USE_NUMBER
  // ESP_LOGI(TAG, "Update request values");
  // float min_voltage = 100000;
  // float max_voltage = 0;
  // float max_current = 0;
  // int fixed_count = 0;
  // int pps_count = 0;

  // for (uint8_t i = 0; i < this->available_pdo_count_; i++) {
  //   if (this->source_pdos_[i].is_fixed()) {
  //     fixed_count++;

  //     float pdo_max_current = this->source_pdos_[i].data.fixed.maxCurrent * 10;
  //     float pdo_voltage = this->source_pdos_[i].data.fixed.voltage * 50;
  //     if (pdo_max_current > max_current) {
  //       max_current = pdo_max_current;
  //     }
  //     if (pdo_voltage > max_voltage) {
  //       max_voltage = pdo_voltage;
  //     }
  //     if (pdo_voltage < min_voltage) {
  //       min_voltage = pdo_voltage;
  //     }
  //   } else if (this->source_pdos_[i].is_pps()) {
  //     pps_count++;

  //     float pdo_max_current = this->source_pdos_[i].data.pps.maxCurrent * 50;
  //     float pdo_min_voltage = this->source_pdos_[i].data.pps.minVoltage * 100;
  //     float pdo_max_voltage = this->source_pdos_[i].data.pps.maxVoltage * 100;
  //     if (pdo_max_current > max_current) {
  //       max_current = pdo_max_current;
  //     }
  //     if (pdo_min_voltage < min_voltage) {
  //       min_voltage = pdo_min_voltage;
  //     }
  //     if (pdo_max_voltage > max_voltage) {
  //       max_voltage = pdo_max_voltage;
  //     }
  //   }
  // }

  // if (min_voltage < 100000) {
  //   this->voltage_number_->traits.set_min_value(min_voltage);
  // } else {
  //   this->voltage_number_->traits.set_min_value(0);
  // }
  // if (max_voltage > 0) {
  //   this->voltage_number_->traits.set_max_value(max_voltage);
  // } else {
  //   this->voltage_number_->traits.set_max_value(28000);
  // }
  // if (max_current > 0) {
  //   this->current_number_->traits.set_max_value(max_current);
  //   this->max_current_number_->traits.set_max_value(max_current);
  // } else {
  //   this->current_number_->traits.set_max_value(6400);
  //   this->max_current_number_->traits.set_max_value(6400);
  // }
#endif
}

void AP33772Component::read_pdos_() {
  ESP_LOGD(TAG, "Read PDOs");
  this->available_pdo_count_ = this->reg(Register::PDONUM).get();
  ESP_LOGD(TAG, "Advertised PDO count: %d", (int) this->available_pdo_count_);
  this->pdos_.clear();
  uint8_t pdos[SRCPDO_LENGTH];
  if (this->read_register(Register::SRCPDO, pdos, SRCPDO_LENGTH) != i2c::ERROR_OK) {
    ESP_LOGE(TAG, "Reading PDOs failed");
#ifdef USE_TEXT_SENSOR
    if (this->available_pdos_text_sensor_ != nullptr && this->available_pdos_text_sensor_->has_state()) {
      this->available_pdos_text_sensor_->publish_state("");
    }
#endif
    return;
  }

#ifdef USE_TEXT_SENSOR
  std::stringstream fixed_pdos_desc;
  std::stringstream pps_pdos_desc;
#endif
#ifdef USE_BINARY_SENSOR
  bool has_pps_pdo = false;
#endif

  for (int i = 0; i < this->available_pdo_count_; i++) {
    Pdo pdo = Pdo((unsigned long) pdos[i * 4], i + 1);
    this->pdos_.push_back(pdo);

    // this->source_pdos_[i].data.byte0 = pdos[i * 4];
    // this->source_pdos_[i].data.byte1 = pdos[i * 4 + 1];
    // this->source_pdos_[i].data.byte2 = pdos[i * 4 + 2];
    // this->source_pdos_[i].data.byte3 = pdos[i * 4 + 3];

#ifdef USE_BINARY_SENSOR
    if (this->pdos_[i].is_pps) {
      // if (this->source_pdos_[i].is_pps()) {
      has_pps_pdo = true;
    }
#endif

#ifdef USE_TEXT_SENSOR
    if (this->pdos_[i].is_fixed) {
      if (fixed_pdos_desc.tellp() > 0) {
        fixed_pdos_desc << " | ";
      }

      fixed_pdos_desc << this->pdos_[i].description();
    } else if (this->pdos_[i].is_pps) {
      if (pps_pdos_desc.tellp() > 0) {
        pps_pdos_desc << " | ";
      }
      pps_pdos_desc << this->pdos_[i].description();
    }
#endif
  }

#ifdef USE_BINARY_SENSOR
  if (this->has_pps_binary_sensor_ != nullptr) {
    this->has_pps_binary_sensor_->publish_state(has_pps_pdo);
  }
#endif

#ifdef USE_TEXT_SENSOR
  if (this->available_pdos_text_sensor_ != nullptr) {
    std::stringstream pdos_text;
    if (fixed_pdos_desc.tellp() > 0) {
      pdos_text << "[FIXED] " << fixed_pdos_desc.str();
    }
    if (pps_pdos_desc.tellp() > 0) {
      if (pdos_text.tellp() > 0) {
        pdos_text << " | ";
      }
      pdos_text << "[PPS] " << pps_pdos_desc.str();
    }
    this->available_pdos_text_sensor_->publish_state(pdos_text.str());
  }
#endif
#ifdef USE_NUMBER
  // this->update_request_number_ranges_();
#endif
}

void AP33772Component::write_rdo_(RdoData rdo) { checkI2CError(this->write_register(Register::RDO, rdo.data, 4)); }

bool AP33772Component::has_request_values_() {
  return (this->requested_voltage_ > 0 || this->requested_current_ > 0 || this->requested_max_current_ > 0);
}

}  // namespace ap33772
}  // namespace esphome
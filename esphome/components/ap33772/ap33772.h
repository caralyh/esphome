#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/core/preferences.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

#ifdef USE_NUMBER
#include "esphome/components/number/number.h"
#endif
#ifdef USE_BUTTON
#include "esphome/components/button/button.h"
#endif
#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif
#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif
#include "pdo.h"

namespace esphome {
namespace ap33772 {

enum Register : uint8_t {
  SRCPDO = 0x00,   // 28 | RO
  PDONUM = 0x1C,   // 1 | RO
  STATUS = 0x1D,   // 1 | RC
  MASK = 0x1E,     // 1 | RW
  VOLTAGE = 0x20,  // 1 | RO
  CURRENT = 0x21,  // 1 | RO
  TEMP = 0x22,     // 1 | RO
  OCPTHR = 0x23,   // 1 | RW
  OTPTHR = 0x24,   // 1 | RW
  DRTHR = 0x25,    // 1 | RW
  TR25 = 0x28,     // 2 | RW
  TR50 = 0x2A,     // 2 | RW
  TR75 = 0x2C,     // 2 | RW
  TR100 = 0x2E,    // 2 | RW
  RDO = 0x30       // 4 | WO
};

enum class Mask : uint8_t {
  READY_EN = 1 << 0,    // 0000 0001
  SUCCESS_EN = 1 << 1,  // 0000 0010
  NEWPDO_EN = 1 << 2,   // 0000 0100
  OVP_EN = 1 << 4,      // 0001 0000
  OCP_EN = 1 << 5,      // 0010 0000
  OTP_EN = 1 << 6,      // 0100 0000
  DR_EN = 1 << 7        // 1000 0000
};

enum class InitialNegotiationStatus : int {
  NONE = 0,      // initial
  WAITING,       // polling or waiting for negotiation
  SUCCESS,       // got pdos
  NO_PDOS,       // negotiation success but no pdos (esp rebooted after previous negotiation?)
  NEGO_FAILURE,  // negotiation failed (incompatible request?)
  NO_RESPONSE,   // no ready state achieved
};

enum class RdoNegotiationStatus : int {
  NONE = 0,
  UPDATE_REQUIRED,
  NO_PDO_MATCH,
  WAITING,
  SUCCESS,
  FAILED,
};

constexpr std::string_view InitialNegotiationStatusName[] = {
    "None", "Waiting", "Success", "No PDOs Received", "Negotiation Failure", "No Response"};
constexpr std::string_view NegotiationStatusName[] = {"None",    "Update Required", "No Matching PDO",
                                                      "Waiting", "Success",         "Failed"};

struct Status {
  union {
    struct {
      uint8_t isReady : 1;
      uint8_t isSuccess : 1;
      uint8_t isNewpdo : 1;
      uint8_t reserved : 1;
      uint8_t isOvp : 1;
      uint8_t isOcp : 1;
      uint8_t isOtp : 1;
      uint8_t isDr : 1;
    };
    uint8_t readStatus;
  };
  uint8_t readVolt;  // LSB: 80mV
  uint8_t readCurr;  // LSB: 24mA
  uint8_t readTemp;  // unit: 1C
};

// struct EventFlag {
//   union {
//     struct {
//       uint8_t newNegoSuccess : 1;
//       uint8_t newNegoFail : 1;
//       uint8_t negoSuccess : 1;
//       uint8_t negoFail : 1;
//       uint8_t reserved_1 : 4;
//     };
//     uint8_t negoEvent;
//   };
//   union {
//     struct {
//       uint8_t ovp : 1;
//       uint8_t ocp : 1;
//       uint8_t otp : 1;
//       uint8_t dr : 1;
//       uint8_t reserved_2 : 4;
//     };
//     uint8_t protectEvent;
//   };
// };

// struct PdoData {
//   union {
//     struct {
//       unsigned int maxCurrent : 10;  // unit: 10mA
//       unsigned int voltage : 10;     // unit: 50mV
//       unsigned int reserved_1 : 10;
//       unsigned int type : 2;
//     } fixed;
//     struct {
//       unsigned int maxCurrent : 7;  // unit: 50mA
//       unsigned int reserved_1 : 1;
//       unsigned int minVoltage : 8;  // unit: 100mV
//       unsigned int reserved_2 : 1;
//       unsigned int maxVoltage : 8;  // unit: 100mV
//       unsigned int reserved_3 : 3;
//       unsigned int apdo : 2;
//       unsigned int type : 2;
//     } pps;
//     struct {
//       uint8_t byte0;
//       uint8_t byte1;
//       uint8_t byte2;
//       uint8_t byte3;
//     };
//     unsigned long data;
//   };
// };

// struct RdoData {
//   union {
//     struct {
//       unsigned int maxCurrent : 10;  // unit: 10mA
//       unsigned int opCurrent : 10;   // unit: 10mA
//       unsigned int reserved_1 : 8;
//       unsigned int objPosition : 3;
//       unsigned int reserved_2 : 1;
//     } fixed;
//     struct {
//       unsigned int opCurrent : 7;  // unit: 50mA
//       unsigned int reserved_1 : 2;
//       unsigned int voltage : 11;  // unit: 20mV
//       unsigned int reserved_2 : 8;
//       unsigned int objPosition : 3;
//       unsigned int reserved_3 : 1;
//     } pps;
//     struct {
//       uint8_t byte0;
//       uint8_t byte1;
//       uint8_t byte2;
//       uint8_t byte3;
//     };
//     uint8_t data[4];
//   };
// };

// class Pdo {
//  public:
//   Pdo() = default;
//   PdoData data;
//   bool is_fixed();
//   bool is_pps();
//   bool can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current);
//   RdoData create_rdo(unsigned int position, unsigned int voltage, unsigned int current, unsigned int max_current);
//   std::string type_name();
//   std::string description();
//   std::string pdo_description();
// };

struct PDPrefs {
  float requested_voltage{0.0f};
  float requested_current{0.0f};
  float requested_max_current{0.0f};
};

class AP33772Component : public PollingComponent, public i2c::I2CDevice {
#ifdef USE_NUMBER
  SUB_NUMBER(voltage)
  SUB_NUMBER(current)
  SUB_NUMBER(max_current)
 public:
  void set_number_by_name(std::string name, number::Number *number);
#endif
#ifdef USE_BUTTON
  SUB_BUTTON(reset)
  SUB_BUTTON(clear)
  SUB_BUTTON(nego)
#endif
#ifdef USE_BINARY_SENSOR
  SUB_BINARY_SENSOR(negotiation_success)
  SUB_BINARY_SENSOR(has_pps)
#endif
#ifdef USE_TEXT_SENSOR
  SUB_TEXT_SENSOR(current_pdo)
  SUB_TEXT_SENSOR(available_pdos)
#endif

 public:
  void dump_config() override;
  float get_setup_priority() const override { return setup_priority::HARDWARE; };
  void setup() override;
  void update() override;
  void loop() override;

  void set_interrupt_pin(InternalGPIOPin *pin) { this->interrupt_pin_ = pin; }
  void set_temperature_sensor(sensor::Sensor *temperature) { temperature_sensor_ = temperature; }
  void set_voltage_sensor(sensor::Sensor *voltage) { voltage_sensor_ = voltage; }
  void set_current_sensor(sensor::Sensor *current) { current_sensor_ = current; }
  void request_voltage(float voltage);
  void request_current(float current);
  void request_max_current(float max_current);
  void reset();
  void clear_request();
  uint8_t read_voltage();
  uint8_t read_current();
  int read_temp() { return this->reg(Register::TEMP).get(); }

  void request_rdo();
  void configure_ntc(int TR25, int TR50, int TR75, int TR100);
  void set_derating_temp(int temperature);
  void set_over_current_threshold(int OCP_current);
  void set_over_temp_threshold(int OTP_temperature);
  void set_mask(Mask flag);
  void clear_mask(Mask flag);

  void start_nego();

  void configure_mask(bool ready, bool success, bool newpdo, bool ovp, bool ocp, bool otp, bool dr);

  void on_safe_shutdown() override;

 protected:
  void attach_interrupt_(InternalGPIOPin *irq_pin, esphome::gpio::InterruptType type);
  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *voltage_sensor_{nullptr};
  sensor::Sensor *current_sensor_{nullptr};
  void write_mask_();
  static void interrupt_handler_(AP33772Component *args);
  void update_error_and_warning_states_();
  void update_request_number_ranges_();
  Status status_ = {0};
  void read_status_();
  void read_pdos_();
  void write_rdo_(RdoData rdo);
  bool has_request_values_();
  void perform_initial_negotiation_();
  void reset_once_();
  void init_reset_count_();
  void set_reset_count_(uint8_t count);
  // void set_reset_flag_(bool flag);
  void set_negotiation_state_(RdoNegotiationStatus status);

  uint8_t available_pdo_count_ = 0;
  // Pdo source_pdos_[7];
  std::vector<Pdo> pdos_ = {};
  InternalGPIOPin *interrupt_pin_{};
  bool interrup_enabled_ = false;
  uint8_t interrupt_mask_ = 0;
  bool pd_reset_done_ = false;
  uint8_t pd_reset_count_ = 0;
  unsigned int requested_voltage_ = 0;      // mV
  unsigned int requested_current_ = 0;      // mA
  unsigned int requested_max_current_ = 0;  // mA
  uint8_t rdo_requested_ = 0;
  bool negotiation_complete_ = false;

  InitialNegotiationStatus initial_negotiation_ = InitialNegotiationStatus::NONE;
  RdoNegotiationStatus negotiation_ = RdoNegotiationStatus::NONE;
  ESPPreferenceObject reset_pref_;
};

template<typename... Ts> class RequestVoltageAction : public Action<Ts...> {
 public:
  RequestVoltageAction(AP33772Component *pd) : pd_(pd) {}
  TEMPLATABLE_VALUE(float, value)

  void play(Ts... x) override { this->pd_->request_voltage(this->value_.value(x...) * 1000); }

 protected:
  AP33772Component *pd_;
};

template<typename... Ts> class RequestCurrentAction : public Action<Ts...> {
 public:
  RequestCurrentAction(AP33772Component *pd) : pd_(pd) {}
  TEMPLATABLE_VALUE(float, value)

  void play(Ts... x) override { this->pd_->request_current(this->value_.value(x...) * 1000); }

 protected:
  AP33772Component *pd_;
};

template<typename... Ts> class RequestMaxCurrentAction : public Action<Ts...> {
 public:
  RequestMaxCurrentAction(AP33772Component *pd) : pd_(pd) {}
  TEMPLATABLE_VALUE(float, value)

  void play(Ts... x) override { this->pd_->request_max_current(this->value_.value(x...) * 1000); }

 protected:
  AP33772Component *pd_;
};

}  // namespace ap33772
}  // namespace esphome

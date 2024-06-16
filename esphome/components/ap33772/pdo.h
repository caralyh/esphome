#pragma once
#include "esphome/core/component.h"
#include <variant>
#include <bitset>

namespace esphome {
namespace ap33772 {

template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...) -> overload<Ts...>;

struct RdoData {
  union {
    struct {
      unsigned int maxCurrent : 10;  // unit: 10mA
      unsigned int opCurrent : 10;   // unit: 10mA
      unsigned int reserved_1 : 8;
      unsigned int objPosition : 3;
      unsigned int reserved_2 : 1;
    } fixed;
    struct {
      unsigned int opCurrent : 7;  // unit: 50mA
      unsigned int reserved_1 : 2;
      unsigned int voltage : 11;  // unit: 20mV
      unsigned int reserved_2 : 8;
      unsigned int objPosition : 3;
      unsigned int reserved_3 : 1;
    } pps;
    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    uint8_t data[4];
  };
};

struct PdoData {
  union {
    struct {
      unsigned int maxCurrent : 10;  // unit: 10mA
      unsigned int voltage : 10;     // unit: 50mV
      unsigned int reserved_1 : 10;
      unsigned int type : 2;
    } fixed;
    struct {
      unsigned int maxCurrent : 7;  // unit: 50mA
      unsigned int reserved_1 : 1;
      unsigned int minVoltage : 8;  // unit: 100mV
      unsigned int reserved_2 : 1;
      unsigned int maxVoltage : 8;  // unit: 100mV
      unsigned int reserved_3 : 3;
      unsigned int apdo : 2;
      unsigned int type : 2;
    } pps;
    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    unsigned long data;
  };
};

struct RawPdoData {
  union {
    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    unsigned long data;
  };
};

// type = 00b fixed, 11b pps
struct FixedPdoData {
  union {
    struct {
      unsigned int maxCurrent : 10;  // unit: 10mA
      unsigned int voltage : 10;     // unit: 50mV
      unsigned int reserved_1 : 10;
      unsigned int type : 2;
    };
    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    unsigned long data;
  };
};

struct PPSPdoData {
  union {
    struct {
      unsigned int maxCurrent : 7;  // unit: 50mA
      unsigned int reserved_1 : 1;
      unsigned int minVoltage : 8;  // unit: 100mV
      unsigned int reserved_2 : 1;
      unsigned int maxVoltage : 8;  // unit: 100mV
      unsigned int reserved_3 : 3;
      unsigned int apdo : 2;
      unsigned int type : 2;
    };
    struct {
      uint8_t byte0;
      uint8_t byte1;
      uint8_t byte2;
      uint8_t byte3;
    };
    unsigned long data;
  };
};

class FixedPdo {
  friend class Pdo;

 public:
  // FixedPdo(FixedPdoData input) : data(input) {}
  FixedPdo(PdoData input, int position) : data(input), position(position) {
    this->max_current = this->data.fixed.maxCurrent;
    this->voltage = this->data.fixed.voltage;
  }
  PdoData data;
  bool can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current);
  RdoData create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current);
  std::string type_name() { return "FIXED"; };
  std::string description();
  bool is_fixed() { return true; };
  bool is_pps() { return false; };
  int position;
  unsigned int max_current;
  unsigned int voltage;
};

class PPSPdo {
  friend class Pdo;

 public:
  // PPSPdo(PPSPdoData input) : data(input) {}
  PPSPdo(PdoData input, int position) : data(input), position(position) {
    this->max_current = this->data.pps.maxCurrent;
    this->min_voltage = this->data.pps.minVoltage;
    this->max_voltage = this->data.pps.maxVoltage;
  }
  PdoData data;
  bool can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current);
  RdoData create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current);
  std::string type_name() { return "PPS"; };
  std::string description();
  bool is_fixed() { return false; };
  bool is_pps() { return true; };
  int position;
  unsigned int max_current;
  unsigned int min_voltage;
  unsigned int max_voltage;
};

using PdoVariant = std::variant<std::monostate, FixedPdo, PPSPdo>;

class Pdo {
 public:
  Pdo(unsigned long input, int position) {
    PdoData data = {.data = input};
    if ((data.byte3 & 0xC0) == 0x00) {
      this->is_fixed = true;
      this->pdo_.emplace<FixedPdo>(data, position);
    } else if ((data.byte3 & 0xF0) == 0xC0) {
      this->is_pps = true;
      this->pdo_.emplace<PPSPdo>(data, position);
    }
  };
  bool can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current);
  RdoData create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current);
  std::string type_name();
  std::string description();
  std::string pdo_description();
  int position();

  bool is_fixed = false;
  bool is_pps = false;

  bool operator<(Pdo other);

 protected:
  PdoVariant pdo_;
};

}  // namespace ap33772
}  // namespace esphome
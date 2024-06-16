
#include <sstream>
#include <iostream>
#include <iomanip>
#include "ap33772.h"
#include "pdo.h"
namespace esphome {
namespace ap33772 {

// bool Pdo::is_fixed() { return (this->data.byte3 & 0xC0) == 0x00; }

// bool Pdo::is_pps() { return (this->data.byte3 & 0xF0) == 0xC0; }

bool Pdo::can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current) {
  return std::visit(overload{[](std::monostate) -> bool { return false; },
                             [voltage, current, max_current](auto &&pdo) -> bool {
                               return pdo.can_satisfy_request(voltage, current, max_current);
                             }},
                    this->pdo_);
}

RdoData Pdo::create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current) {
  return std::visit(overload{[](std::monostate) -> RdoData { return RdoData{}; },
                             [voltage, current, max_current](auto &&pdo) -> RdoData {
                               return pdo.create_rdo(voltage, current, max_current);
                             }},
                    this->pdo_);
}

std::string Pdo::type_name() {
  return std::visit(overload{[](std::monostate) -> std::string { return ""; },
                             [](auto &&pdo) -> std::string { return pdo.type_name(); }},
                    this->pdo_);
}

std::string Pdo::description() {
  return std::visit(overload{[](std::monostate) -> std::string { return ""; },
                             [](auto &&pdo) -> std::string { return pdo.description(); }},
                    this->pdo_);
}

std::string Pdo::pdo_description() {
  std::stringstream ss;
  ss << "[" << this->type_name() << "] " << this->description();
  return ss.str();
}

int Pdo::position() {
  return std::visit(overload{[](std::monostate) -> int { return 0; }, [](auto &&pdo) -> int { return pdo.position; }},
                    this->pdo_);
}

bool Pdo::operator<(Pdo other) {
  if (this->is_pps && other.is_pps) {
    auto a = std::get<PPSPdo>(this->pdo_);
    auto b = std::get<PPSPdo>(other.pdo_);
    if (a.min_voltage < b.min_voltage) {
      return true;
    }
    if (a.max_current < b.max_current) {
      return true;
    }
    if (a.max_voltage < b.max_voltage) {
      return true;
    }
  } else if (this->is_fixed && other.is_fixed) {
    auto a = std::get<FixedPdo>(this->pdo_);
    auto b = std::get<FixedPdo>(other.pdo_);
    if (a.voltage < b.voltage) {
      return true;
    }
    if (a.max_current < b.max_current) {
      return true;
    }
  } else if (this->is_pps && other.is_fixed) {
    return true;
  }
  return false;
}

bool ap33772::FixedPdo::can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current) {
  if (current > 0 && this->max_current * 10 < current) {
    return false;
  }
  if (current <= 0 && max_current > 0 && this->max_current * 10 < max_current) {
    return false;
  }
  // TODO: configurable tolerance/range for fixed voltage selection
  if (this->voltage * 50 > voltage) {
    return false;
  }
  return true;
}

bool PPSPdo::can_satisfy_request(unsigned int voltage, unsigned int current, unsigned int max_current) {
  if (voltage > 0) {
    if (this->min_voltage * 100 > voltage) {
      return false;
    }
    if (this->max_voltage * 100 < voltage) {
      return false;
    }
  }
  if (current > 0 && current > this->max_current * 50) {
    return false;
  }
  if (current <= 0 && max_current > 0 && this->max_current * 50 < max_current) {
    return false;
  }
  return true;
}

RdoData ap33772::FixedPdo::create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current) {
  RdoData rdo;
  rdo.fixed.objPosition = this->position;
  if (current > 0) {
    rdo.fixed.opCurrent = current / 10;
  }
  if (max_current > 0) {
    if (max_current > this->max_current * 10) {
      rdo.fixed.maxCurrent = this->max_current;
    } else {
      rdo.fixed.maxCurrent = max_current / 10;
    }
  }

  return rdo;
}

RdoData PPSPdo::create_rdo(unsigned int voltage, unsigned int current, unsigned int max_current) {
  RdoData rdo;
  rdo.pps.objPosition = this->position;
  if (voltage > 0) {
    rdo.pps.voltage = voltage / 20;
  }
  if (current > 0) {
    rdo.pps.opCurrent = current / 50;
  }

  return rdo;
}

std::string ap33772::FixedPdo::description() {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);

  ss << this->voltage * 50 / 1000 << "V @ ";
  ss << this->max_current * 10 / 1000 << "A";

  return ss.str();
}

std::string PPSPdo::description() {
  std::stringstream ss;
  ss << std::fixed << std::setprecision(2);

  ss << this->min_voltage * 100 / 1000 << "V~";
  ss << this->max_voltage * 100 / 1000 << "V @ ";
  ss << this->max_current * 50 / 1000 << "A";

  return ss.str();
}

}  // namespace ap33772

// namespace ap33772
}  // namespace esphome
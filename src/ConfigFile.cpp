/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include <bci/bs120x/ConfigFile.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <format>
#include <map>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <variant>

#include "StringUtil.h"
#include "Util.h"

namespace bci::bs120x {

using ConfigValue = ConfigFile::ConfigValue;
using util::Err;

static constexpr Result<bool> ConvBoolProperty(std::string_view str) noexcept {
  constexpr auto lowercase = [](std::string_view sv) {
    return std::ranges::transform_view(
        sv, [](char c) -> char { return std::tolower(c); });
  };

  const auto eq_icase = [=](std::string_view val) {
    return std::ranges::equal(lowercase(str), lowercase(val));
  };

  if (eq_icase("true")) {
    return true;
  }

  if (eq_icase("false")) {
    return false;
  }

  return Err(ErrorCode::kWrongConfigOptionType);
}

template <class T>
static constexpr Result<T> ParseProperty(std::string_view str) noexcept {
  T val;
  auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), val);
  if (ec == std::errc{} && ptr == str.data() + str.size()) {
    return val;
  }
  return Err(ErrorCode::kWrongConfigOptionType);
}

static constexpr Result<int> ConvIntProperty(std::string_view str) noexcept {
  return ParseProperty<int>(str);
}

static constexpr Result<float> ConvFloatProperty(
    std::string_view str) noexcept {
  return ParseProperty<float>(str);
}

ConfigFile::ConfigFile() = default;

void ConfigFile::Load(std::span<const std::uint8_t> buf) {
  config_.clear();

  std::string_view input_str{reinterpret_cast<const char*>(buf.data()),
                             buf.size()};

  // gets next line (trimmed)
  const auto next_line = [&input_str]() -> std::optional<std::string_view> {
    if (input_str.empty()) {
      return std::nullopt;
    }

    std::string_view line;
    auto pos = input_str.find('\n');
    if (pos != input_str.npos) {
      line = util::Trim(input_str.substr(0, pos + 1));
      input_str.remove_prefix(pos + 1);
    } else {
      line = util::Trim(input_str);
      input_str = {};
    }

    return line;
  };

  std::string section;
  std::string appended_key;

  while (auto l = next_line()) {
    auto lv = *l;

    if (lv.empty()) {
      continue;
    }

    if (lv.starts_with("*CFG")) {
      lv = util::TrimLeft(lv.substr(4));
      if (lv.empty()) {
        continue;
      }
    }

    if (lv.starts_with("//")) {
      continue;
    }

    if ((lv.front() == '[') && (lv.back() == ']')) {
      lv = util::Trim(lv.substr(1, lv.size() - 2));
      if (!lv.empty()) {
        section = lv;
      } else {
        continue;
      }
    }

    auto delim_pos = lv.find('=');
    if (delim_pos != std::string_view::npos) {
      auto key = util::TrimRight(lv.substr(0, delim_pos));
      if (key.empty()) {
        continue;
      }
      appended_key = std::format("{}.{}", section, key);

      auto value =
          util::TrimLeft(lv.substr(delim_pos + 1, lv.size() - delim_pos - 1));
      if (auto rslt = ConvBoolProperty(value)) {
        config_[appended_key] = *rslt;
      } else if (auto rslt = ConvIntProperty(value)) {
        config_[appended_key] = *rslt;
      } else if (auto rslt = ConvFloatProperty(value)) {
        config_[appended_key] = *rslt;
      } else {
        if (value.starts_with('"') && value.ends_with('"')) {
          value = util::Trim(value.substr(1, value.size() - 2));
        }
        config_[appended_key] = std::string(value);
      }
    }
  }
}

UnitConfig ConfigFile::ToUnitConfig() const {
  UnitConfig cfg{};

  cfg.cell_inhibit_enable =
      GetValBool("OperationalModes.CellInhibitEnable").value_or(false);
  cfg.udp_data_broadcast_enable =
      GetValBool("UDPData.UDPDataEnable").value_or(false);

  cfg.udp_data_port = GetValInt("UDPData.UDPDataPort").value_or(0);
  cfg.udp_data_period = GetValInt("UDPData.UDPDataPeriod_mS").value_or(0);
  cfg.box_id = GetValInt("CAN.BoxID").value_or(0);
  cfg.can_data_period = GetValInt("CAN.WritePeriod_ms").value_or(0);

  cfg.serial_number =
      GetValString("UnitData.SerialNumber").value_or(std::string{});
  cfg.firmware_version =
      GetValString("General.Firmware").value_or(std::string{});
  cfg.calibration_date =
      GetValString("UnitData.Calibration Date").value_or(std::string{});
  cfg.ip_address = GetValString("General.IPAddress").value_or(std::string{});

  return cfg;
}

template <class T>
inline static Result<T> GetValAs(const std::map<std::string, ConfigValue>& map,
                                 const std::string& key) {
  if (map.empty()) {
    return Err(ErrorCode::kNoConfigData);
  }

  auto val = map.find(key);
  if (val == map.end()) {
    return Err(ErrorCode::kInvalidConfigOption);
  }

  if (const T* v = std::get_if<T>(&val->second)) {
    return *v;
  }

  return Err(ErrorCode::kWrongConfigOptionType);
}

Result<bool> ConfigFile::GetValBool(const std::string& key) const {
  return GetValAs<bool>(config_, key);
}

Result<int> ConfigFile::GetValInt(const std::string& key) const {
  return GetValAs<int>(config_, key);
}

Result<float> ConfigFile::GetValFloat(const std::string& key) const {
  return GetValAs<float>(config_, key);
}

Result<std::string> ConfigFile::GetValString(const std::string& key) const {
  return GetValAs<std::string>(config_, key);
}

template <class T>
inline static ErrorCode SetConfVal(std::map<std::string, ConfigValue>& map,
                                   const std::string& key, T&& val) {
  if (map.empty()) {
    return ErrorCode::kNoConfigData;
  }

  auto current_val = map.find(key);

  if (current_val == map.end()) {
    return ErrorCode::kInvalidConfigOption;
  }

  if (!std::holds_alternative<std::remove_cvref_t<T>>(current_val->second)) {
    return ErrorCode::kWrongConfigOptionType;
  }

  map[key] = std::forward<T>(val);

  return ErrorCode::kSuccess;
}

ErrorCode ConfigFile::SetVal(const std::string& key, bool val) {
  return SetConfVal(config_, key, val);
}

ErrorCode ConfigFile::SetVal(const std::string& key, int val) {
  return SetConfVal(config_, key, val);
}

ErrorCode ConfigFile::SetVal(const std::string& key, float val) {
  return SetConfVal(config_, key, val);
}

ErrorCode ConfigFile::SetVal(const std::string& key, std::string val) {
  return SetConfVal(config_, key, std::move(val));
}

void ConfigFile::Clear() noexcept { config_.clear(); }

bool ConfigFile::IsEmpty() const noexcept { return config_.empty(); }

}  // namespace bci::bs120x

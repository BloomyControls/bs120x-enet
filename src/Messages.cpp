/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include "Messages.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <format>
#include <span>
#include <string>

#include "StringUtil.h"
#include "Util.h"

namespace bci::bs120x::messages {

using util::Err;

inline constexpr float BytesToFloat(std::uint8_t b0, std::uint8_t b1,
                                    float scale, float offset) {
  return (((b1 << 8) | b0) * scale) + offset;
}

inline constexpr std::array<std::uint8_t, 2> FloatToBytes(float val,
                                                          float scale,
                                                          float offset) {
  std::array<std::uint8_t, 2> bytes = {};
  auto scaled_val = static_cast<int16_t>((val - offset) / scale);
  bytes[0] = static_cast<std::uint8_t>(scaled_val & 0x00ff);
  bytes[1] = static_cast<std::uint8_t>((scaled_val & 0xff00) >> 8);
  return bytes;
}

ErrorCode ParseCellVReadBack(const Frame& frame,
                             std::array<float, kCellCount>& values) {
  int off;
  switch (frame.GetArbId()) {
    case ArbId::kVReadBack1_4:
      off = 0;
      break;
    case ArbId::kVReadBack5_8:
      off = 4;
      break;
    case ArbId::kVReadBack9_12:
      off = 8;
      break;
    default:
      return ErrorCode::kFrameIdMismatch;
  }

  const auto data = frame.GetData();
  if (data.size() != 8) {
    return ErrorCode::kInvalidFrameSize;
  }

  auto vals = std::span{values}.subspan(off, 4);
  for (int i = 0; i < 4; ++i) {
    vals[i] = BytesToFloat(data[i * 2], data[(i * 2) + 1], kCellVoltageScale,
                           kCellVoltageOffset);
  }

  return ErrorCode::kSuccess;
}

ErrorCode ParseCellIReadBack(const Frame& frame,
                             std::array<float, kCellCount>& values) {
  int off;
  switch (frame.GetArbId()) {
    case ArbId::kIReadBack1_4:
      off = 0;
      break;
    case ArbId::kIReadBack5_8:
      off = 4;
      break;
    case ArbId::kIReadBack9_12:
      off = 8;
      break;
    default:
      return ErrorCode::kFrameIdMismatch;
  }

  const auto data = frame.GetData();
  if (data.size() != 8) {
    return ErrorCode::kInvalidFrameSize;
  }

  auto vals = std::span{values}.subspan(off, 4);
  for (int i = 0; i < 4; ++i) {
    vals[i] = BytesToFloat(data[i * 2], data[(i * 2) + 1], kCellCurrentScale,
                           kCellCurrentReadBackOffset);
  }

  return ErrorCode::kSuccess;
}

ErrorCode ParseAIReadBack(const Frame& frame,
                          std::array<float, kAnalogInputCount>& values) {
  int off;
  switch (frame.GetArbId()) {
    case ArbId::kAIReadBack1_4:
      off = 0;
      break;
    case ArbId::kAIReadBack5_8:
      off = 4;
      break;
    default:
      return ErrorCode::kFrameIdMismatch;
  }

  const auto data = frame.GetData();
  if (data.size() != 8) {
    return ErrorCode::kInvalidFrameSize;
  }

  auto vals = std::span{values}.subspan(off, 4);
  for (int i = 0; i < 4; ++i) {
    vals[i] = BytesToFloat(data[i * 2], data[(i * 2) + 1], kAIVoltageScale,
                           kAIVoltageOffset);
  }

  return ErrorCode::kSuccess;
}

ErrorCode ParseDIOReadBack(const Frame& frame,
                           std::array<bool, kDioCount>& values) {
  if (frame.GetArbId() != ArbId::kDIOReadBack1_8) {
    return ErrorCode::kFrameIdMismatch;
  }

  const auto data = frame.GetData();
  if (data.size() != 8) {
    return ErrorCode::kInvalidFrameSize;
  }

  for (unsigned int i = 0; i < kDioCount; i++) {
    values[i] = data[0] & (1 << i);
  }

  return ErrorCode::kSuccess;
}

ErrorCode ParseStatusReadBack(const Frame& frame, UnitStatus& status) {
  if (frame.GetArbId() != ArbId::kStatusReadBack) {
    return ErrorCode::kFrameIdMismatch;
  }

  const auto data = frame.GetData();
  if (data.size() != 8) {
    return ErrorCode::kInvalidFrameSize;
  }

  for (unsigned int i = 0; i < kFanCount; i++) {
    status.fan_fail_status[i] = data[0] & (1 << i);
  }

  status.inhibit_state = data[0] & 0x10;

  for (unsigned int i = 0; i < kTempCount; i++) {
    status.temps[i] = static_cast<float>(data[i + 1]);
  }

  return ErrorCode::kSuccess;
}

Result<Frame> PackDIOSetAll(std::span<const bool> output,
                            std::span<const bool> direction) {
  if (output.size() != kDioCount || direction.size() != kDioCount) {
    return Err(ErrorCode::kInvalidArgument);
  }

  std::array<std::uint8_t, 8> data{};
  for (unsigned int i = 0; i < kDioCount; i++) {
    data[0] |= static_cast<std::uint8_t>(output[i] << i);
    data[1] |= static_cast<std::uint8_t>(direction[i] << i);
  }

  return Frame::Pack(ArbId::kSetDIOAll, 0, false, true, data);
}

Result<Frame> PackAOSetAll(std::span<const float> voltages) {
  if (voltages.size() != kAnalogOutputCount) {
    return Err(ErrorCode::kInvalidArgument);
  }

  std::array<std::uint8_t, 8> data{};
  for (unsigned int i = 0; i < kAnalogOutputCount; i++) {
    std::ranges::copy(
        FloatToBytes(voltages[i], kAOVoltageScale, kAOVoltageOffset),
        data.begin() + (i * 2));
  }

  return Frame::Pack(ArbId::kSetAOAll, 0, false, true, data);
}

Result<Frame> PackOpModeConfig(OpModeConfig config) {
  std::array<std::uint8_t, 8> data{};

  data[0] = static_cast<std::uint8_t>(config.calibration_enable) |
            static_cast<std::uint8_t>(config.pid_disable) << 1;
  data[1] = static_cast<std::uint8_t>(config.precision_mode_enable);

  return Frame::Pack(ArbId::kOpModeCfg, 0, false, true, data);
}

Result<Frame> PackCellISetAll(float isrc, float isnk) {
  std::array<std::uint8_t, 8> data{};
  std::ranges::copy(
      FloatToBytes(isrc, kCellCurrentScale, kCellCurrentCmdOffset),
      data.begin());
  std::ranges::copy(
      FloatToBytes(isnk, kCellCurrentScale, kCellCurrentCmdOffset),
      data.begin() + 2);

  return Frame::Pack(ArbId::kISetAll, 0, false, true, data);
}

Result<Frame> PackCellISinkSet(unsigned int cell, float isnk) {
  std::array<std::uint8_t, 8> data{};
  data[0] = static_cast<std::uint8_t>(cell);
  std::ranges::copy(
      FloatToBytes(isnk, kCellCurrentScale, kCellCurrentCmdOffset),
      data.begin() + 1);

  return Frame::Pack(ArbId::kISinkSetSgl, 0, false, true, data);
}

Result<Frame> PackCellISourceSet(unsigned int cell, float isrc) {
  std::array<std::uint8_t, 8> data{};
  data[0] = static_cast<std::uint8_t>(cell);
  std::ranges::copy(
      FloatToBytes(isrc, kCellCurrentScale, kCellCurrentCmdOffset),
      data.begin() + 1);

  return Frame::Pack(ArbId::kISrcSetSgl, 0, false, true, data);
}

Result<Frame> PackCellVSetAll(float voltage) {
  std::array<std::uint8_t, 8> data{};
  std::ranges::copy(
      FloatToBytes(voltage, kCellVoltageScale, kCellVoltageOffset),
      data.begin());

  return Frame::Pack(ArbId::kVSetAll, 0, false, true, data);
}

Result<Frame> PackCellVSet(unsigned int cell, float voltage) {
  std::array<std::uint8_t, 8> data{};
  data[0] = static_cast<std::uint8_t>(cell);
  std::ranges::copy(
      FloatToBytes(voltage, kCellVoltageScale, kCellVoltageOffset),
      data.begin() + 1);

  return Frame::Pack(ArbId::kVSetSgl, 0, false, true, data);
}

Result<Frame> PackCellEnableAll(bool enable) {
  std::array<std::uint8_t, 8> data{};
  data[0] = static_cast<std::uint8_t>(enable);

  return Frame::Pack(ArbId::kCellEnAll, 0, false, true, data);
}

Result<Frame> PackCellEnable(unsigned int cell, bool enable) {
  std::array<std::uint8_t, 8> data{};
  data[0] = static_cast<std::uint8_t>(cell);
  data[1] = static_cast<std::uint8_t>(enable);

  return Frame::Pack(ArbId::kCellEnSgl, 0, false, true, data);
}

namespace config {

Result<std::string> FmtResetCommand() { return "*RST"; }

Result<std::string> FmtSetIP(std::string_view ip) {
  ip = util::Trim(ip);

  if (!util::IsValidIpAddress(ip)) {
    return Err(ErrorCode::kInvalidIPAddress);
  }

  return std::format("*IPA {}", ip);
}

Result<std::string> FmtQueryConfig() { return "*CFG?"; }

Result<std::string> FmtSetOption(std::string_view opt, int value) {
  return std::format("*CFG <int> {} = {}", opt, value);
}

Result<std::string> FmtSetOption(std::string_view opt, bool value) {
  return std::format("*CFG <bool> {} = {}", opt, value ? "True" : "False");
}

Result<std::string> FmtSetOption(std::string_view opt, std::string_view value) {
  return std::format("*CFG <string> {} = {}", opt, value);
}

Result<std::string> FmtSetOption(std::string_view opt, double value) {
  return std::format("*CFG <double> {} = {}", opt, value);
}

bool IsConfigResponse(std::span<const std::uint8_t> buf) {
  if (buf.size() < 4) {
    return false;
  }

  return (buf[0] == '*') && (buf[1] == 'C') && (buf[2] == 'F') &&
         (buf[3] == 'G');
}

}  // namespace config
}  // namespace bci::bs120x::messages

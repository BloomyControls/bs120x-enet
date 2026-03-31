/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#ifndef BS120X_SRC_MESSAGES_H
#define BS120X_SRC_MESSAGES_H

#include <bci/bs120x/CommonTypes.h>

#include <array>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

#include "Frame.h"

/* Functions for parsing readback values from Frames or creating Frames
from setpoint values. */
namespace bci::bs120x::messages {

/* Scaling values for floating-point setpoints and readbacks. */
inline constexpr float kCellVoltageScale{0.0001f};
inline constexpr float kCellVoltageOffset{0.0f};
inline constexpr float kCellCurrentScale{0.0001f};
inline constexpr float kCellCurrentReadBackOffset{-3.2768f};
inline constexpr float kCellCurrentCmdOffset{0.0f};
inline constexpr float kAIVoltageScale{0.0001f};
inline constexpr float kAIVoltageOffset{0.0f};
inline constexpr float kAOVoltageScale{0.0001f};
inline constexpr float kAOVoltageOffset{0.0f};

ErrorCode ParseCellVReadBack(const Frame& frame,
                             std::array<float, kCellCount>& values);

ErrorCode ParseCellIReadBack(const Frame& frame,
                             std::array<float, kCellCount>& values);

ErrorCode ParseAIReadBack(const Frame& frame,
                          std::array<float, kAnalogInputCount>& values);

ErrorCode ParseDIOReadBack(const Frame& frame,
                           std::array<bool, kDioCount>& values);

ErrorCode ParseStatusReadBack(const Frame& frame, UnitStatus& status);

constexpr std::array<std::uint8_t, 4> CreateSizePacket(std::uint8_t len) {
  return {0, 0, 0, len};
}

Result<Frame> PackDIOSetAll(std::span<const bool> output,
                            std::span<const bool> direction);

Result<Frame> PackAOSetAll(std::span<const float> voltages);

Result<Frame> PackOpModeConfig(OpModeConfig config);

Result<Frame> PackCellISetAll(float isrc, float isnk);

Result<Frame> PackCellISinkSet(unsigned int cell, float isnk);

Result<Frame> PackCellISourceSet(unsigned int cell, float isrc);

Result<Frame> PackCellVSetAll(float voltage);

Result<Frame> PackCellVSet(unsigned int cell, float voltage);

Result<Frame> PackCellEnableAll(bool enable);

Result<Frame> PackCellEnable(unsigned int cell, bool enable);

/* Functions for generating configuration command frames. */
namespace config {

Result<std::string> FmtResetCommand();

Result<std::string> FmtSetIP(std::string_view ip);

Result<std::string> FmtQueryConfig();

Result<std::string> FmtSetOption(std::string_view opt, int value);

Result<std::string> FmtSetOption(std::string_view opt, bool value);

Result<std::string> FmtSetOption(std::string_view opt, std::string_view value);

Result<std::string> FmtSetOption(std::string_view opt, double value);

bool IsConfigResponse(std::span<const std::uint8_t> buf);

}  // namespace config

}  // namespace bci::bs120x::messages

#endif

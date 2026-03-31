/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/**
 * @file
 * @brief Contains common types for use with the Ethernet client.
 */

#ifndef BS120X_INCLUDE_BCI_BS120X_COMMONTYPES_H
#define BS120X_INCLUDE_BCI_BS120X_COMMONTYPES_H

#include <array>
#include <cstdint>
#include <string>

#include "bs120xenet_export.h"
#include "util/expected.hpp"

namespace bci::bs120x {

/// Total cell count
inline constexpr unsigned int kCellCount{12U};
/// Total analog input count
inline constexpr unsigned int kAnalogInputCount{8U};
/// Total analog output count
inline constexpr unsigned int kAnalogOutputCount{2U};
/// Total DIO count
inline constexpr unsigned int kDioCount{8U};
/// Total fan count
inline constexpr unsigned int kFanCount{4U};
/// Total temperature sensor count
inline constexpr unsigned int kTempCount{3U};

/// Error codes returned by driver functions
/// @note These values must match the macros in CInterface.h!
enum class ErrorCode : int {
  kSuccess = 0,                    ///< Success (no error)
  kChannelIndexOutOfRange = -100,  ///< Channel index out of range
  kInvalidIPAddress = -101,        ///< Invalid IP address
  kAlreadyConnected = -102,        ///< Driver already connected
  kTcpAlreadyConnected = -103,     ///< TCP client already connected
  kUdpAlreadyConnected = -104,     ///< UDP listener already connected
  kUdpSocketOpenFailed = -105,     ///< Failed to open socket for UDP listener
  kTcpSocketConfigFailed = -106,   ///< Failed to configure TCP socket
  kUdpSocketConfigFailed = -107,   ///< Failed to configure UDP socket
  kUdpSocketBindFailed = -108,     ///< Failed to bind UDP socket
  kTcpConnectFailed = -109,        ///< Failed to connect TCP client
  kReadFailed = -110,              ///< Failed to read message
  kSendFailed = -111,              ///< Failed to send message
  kNotConnected = -112,            ///< Not connected
  kFrameIdMismatch = -113,        ///< Frame Arb ID doesn't match expected value
  kInvalidFrameSize = -114,       ///< Frame size does not match known messages
  kAiIndexOutOfRange = -115,      ///< Analog input index out of range
  kDioIndexOutOfRange = -116,     ///< DIO index out of range
  kBufferTooSmall = -117,         ///< Buffer too small
  kVoltageOutOfRange = -118,      ///< Voltage out of range
  kCurrentOutOfRange = -119,      ///< Current out of range
  kReadbackTimedOut = -120,       ///< UDP read timed out
  kInvalidBoxId = -121,           ///< Invalid box ID
  kNoConfigData = -122,           ///< Config data has not been queried
  kInvalidConfigOption = -123,    ///< Invalid configuration option
  kWrongConfigOptionType = -124,  ///< Incorrect configuration option type
  kInvalidArgument = -125,        ///< Invalid argument
  kAllocationFailed = -126,       ///< Allocation failed
  kUnexpectedException = -127,    ///< Unexpected exception
};

/// Information about unit health and status.
struct UnitStatus {
  std::array<bool, kFanCount>
      fan_fail_status;  ///< Status of all unit fans. A TRUE value indicates a
                        ///< fan failure.
  bool inhibit_state;   ///< State of the cell inhibit. A TRUE value indicates
                        ///< ENABLED.
  std::array<float, kTempCount>
      temps;  ///< Temperatures reported by temperature sensors in Celsius.
};

/// Operating mode configuration values.
struct OpModeConfig {
  bool calibration_enable;     ///< Enables or disables calibration mode.
  bool pid_disable;            ///< Enables or disables cell PID control.
  bool precision_mode_enable;  ///< Enables or disables precision mode.
};

/// Information about the unit configuration.
struct UnitConfig {
  std::string serial_number;  ///< The unit's serial number.
  std::string
      firmware_version;  ///< The version of firmware installed on the unit.
  std::string calibration_date;   ///< The calibration date of the unit.
  bool cell_inhibit_enable;       ///< Indicates whether the unit's cell inhibit
                                  ///< lines are enabled.
  std::string ip_address;         ///< The unit's IP address.
  std::uint16_t udp_data_port;    ///< The UDP port the unit broadcasts data on.
  std::uint32_t udp_data_period;  ///< The interval in milliseconds at which the
                                  ///< unit broadcasts data.
  bool udp_data_broadcast_enable;  ///< Indicates whether the UDP data broadcast
                                   ///< is enabled.
  std::uint8_t box_id;             ///< The unit's box ID.
  std::uint32_t
      can_data_period;  ///< The interval at which the unit transmits CAN data.
};

/**
 * @brief Result type used to return values or error codes for driver functions.
 *
 * 'tl::expected' models C++23's 'std::expected' and adds some other useful
 * functions as well.
 *
 * @param T Type of the 'expected' response.
 */
template <class T>
using Result = tl::expected<T, ErrorCode>;

/**
 * @brief Get an error message string for a given error code.
 *
 * @param[in] ec Error code.
 *
 * @return Error message.
 */
BS120X_API const char* ErrorMessage(ErrorCode ec) noexcept;

}  // namespace bci::bs120x

#endif

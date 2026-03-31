/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include <bci/bs120x/CommonTypes.h>

namespace bci::bs120x {

/* Return an error message for a given error code. */
const char* ErrorMessage(ErrorCode ec) noexcept {
  switch (ec) {
    case ErrorCode::kSuccess:
      return "Success";
    case ErrorCode::kChannelIndexOutOfRange:
      return "Channel index out of range";
    case ErrorCode::kInvalidIPAddress:
      return "Invalid IP address";
    case ErrorCode::kAlreadyConnected:
      return "Already connected";
    case ErrorCode::kTcpAlreadyConnected:
      return "TCP connection already open";
    case ErrorCode::kUdpAlreadyConnected:
      return "UDP connection already open";
    case ErrorCode::kUdpSocketOpenFailed:
      return "Failed to open UDP socket";
    case ErrorCode::kTcpSocketConfigFailed:
      return "Failed to configure TCP socket";
    case ErrorCode::kUdpSocketConfigFailed:
      return "Failed to configure UDP socket";
    case ErrorCode::kUdpSocketBindFailed:
      return "Failed to bind UDP socket";
    case ErrorCode::kTcpConnectFailed:
      return "TCP connection failed";
    case ErrorCode::kReadFailed:
      return "Read failed";
    case ErrorCode::kSendFailed:
      return "Send failed";
    case ErrorCode::kNotConnected:
      return "Not connected";
    case ErrorCode::kFrameIdMismatch:
      return "Frame ID mismatch";
    case ErrorCode::kInvalidFrameSize:
      return "Invalid frame size";
    case ErrorCode::kAiIndexOutOfRange:
      return "Analog input index out of range";
    case ErrorCode::kDioIndexOutOfRange:
      return "DIO index out of range";
    case ErrorCode::kBufferTooSmall:
      return "Buffer too small";
    case ErrorCode::kVoltageOutOfRange:
      return "Voltage out of range";
    case ErrorCode::kCurrentOutOfRange:
      return "Current out of range";
    case ErrorCode::kReadbackTimedOut:
      return "Read timed out";
    case ErrorCode::kInvalidBoxId:
      return "Invalid Box ID";
    case ErrorCode::kNoConfigData:
      return "No configuration data present";
    case ErrorCode::kInvalidConfigOption:
      return "Invalid configuration option";
    case ErrorCode::kWrongConfigOptionType:
      return "Wrong configuration option type";
    case ErrorCode::kInvalidArgument:
      return "Invalid argument";
    case ErrorCode::kAllocationFailed:
      return "Allocation failed";
    case ErrorCode::kUnexpectedException:
      return "Unexpected exception";
  }

  return "Unknown error";
}

}  // namespace bci::bs120x

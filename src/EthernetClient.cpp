/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include <bci/bs120x/ConfigFile.h>
#include <bci/bs120x/EthernetClient.h>

#include <algorithm>
#include <utility>

#include "Frame.h"
#include "Messages.h"
#include "TcpClient.h"
#include "UdpListener.h"
#include "Util.h"

namespace config = bci::bs120x::messages::config;

namespace bci::bs120x {

inline constexpr float kCellMaxVoltage{5.0f};
inline constexpr float kCellMinVoltage{0.0f};
inline constexpr float kCellMaxCurrent{0.5f};
inline constexpr float kCellMinCurrent{0.0f};
inline constexpr float kAOMaxVoltage{5.0f};
inline constexpr float kAOMinVoltage{0.0f};

using util::Err;

unsigned int EthernetClient::Version() noexcept {
  return BS120XENET_VERSION;
}

EthernetClient::EthernetClient()
    : udp_listener_{std::make_unique<net::UdpListener>()},
      tcp_client_{std::make_unique<net::TcpClient>()},
      connected_{false},
      client_error_{ErrorCode::kSuccess},
      cell_voltages_{},
      cell_currents_{},
      analog_inputs_{},
      dio_states_{},
      status_{},
      config_{},
      unit_config_{} {}

EthernetClient::~EthernetClient() { Disconnect(); }

ErrorCode EthernetClient::Connect(std::string_view interface_ip,
                                  std::string_view device_ip,
                                  std::uint16_t udp_port,
                                  std::int32_t udp_timeout,
                                  std::uint16_t tcp_port) noexcept {
  std::lock_guard lg{mutex_};
  if (connected_) {
    return ErrorCode::kAlreadyConnected;
  }

  auto e = tcp_client_->Connect(device_ip, tcp_port);
  if (e != ErrorCode::kSuccess) {
    return e;
  }

  tcp_client_->SetErrorCallback([this](auto e) { ConnectionError(e); });

  if (e != ErrorCode::kSuccess) {
    tcp_client_->Disconnect();
    return e;
  }

  e = udp_listener_->Connect(interface_ip, udp_port, udp_timeout);
  if (e != ErrorCode::kSuccess) {
    tcp_client_->Disconnect();
    return e;
  }

  udp_listener_->SetDataReceivedCallback(
      [this](auto data) { UdpDataReceived(data); });
  udp_listener_->SetErrorCallback([this](auto e) { ConnectionError(e); });

  if (e != ErrorCode::kSuccess) {
    tcp_client_->Disconnect();
    udp_listener_->Disconnect();
    return e;
  }

  connected_ = true;

  return ErrorCode::kSuccess;
}

void EthernetClient::Disconnect() noexcept {
  std::lock_guard lg{mutex_};
  udp_listener_->Disconnect();
  tcp_client_->Disconnect();

  connected_ = false;
  cell_voltages_.fill(0.0f);
  cell_currents_.fill(0.0f);
  analog_inputs_.fill(0.0f);
  dio_states_.fill(false);
  status_ = {};
  config_.Clear();
  unit_config_ = {};
}

ErrorCode EthernetClient::SetDIOStates(std::span<const bool> outputs,
                                       std::span<const bool> directions) const {
  if (outputs.size() != kDioCount || directions.size() != kDioCount) {
    return ErrorCode::kInvalidArgument;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackDIOSetAll(outputs, directions));
}

ErrorCode EthernetClient::SetAnalogOutputs(
    std::span<const float> voltages) const {
  for (float voltage : voltages) {
    if (voltage > kAOMaxVoltage || voltage < kAOMinVoltage) {
      return ErrorCode::kVoltageOutOfRange;
    }
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackAOSetAll(voltages));
}

ErrorCode EthernetClient::SetOpModeConfig(OpModeConfig config) const {
  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackOpModeConfig(config));
}

ErrorCode EthernetClient::SetAllCellCurrents(float isrc, float isnk) const {
  if (isrc > kCellMaxCurrent || isrc < kCellMinCurrent ||
      isnk > kCellMaxCurrent || isnk < kCellMinCurrent) {
    return ErrorCode::kCurrentOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellISetAll(isrc, isnk));
}

ErrorCode EthernetClient::SetCellSinkCurrent(unsigned int cell,
                                             float isnk) const {
  if (cell >= kCellCount) {
    return ErrorCode::kChannelIndexOutOfRange;
  }

  if (isnk > kCellMaxCurrent || isnk < kCellMinCurrent) {
    return ErrorCode::kCurrentOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellISinkSet(cell, isnk));
}

ErrorCode EthernetClient::SetCellSourceCurrent(unsigned int cell,
                                               float isrc) const {
  if (cell >= kCellCount) {
    return ErrorCode::kChannelIndexOutOfRange;
  }

  if (isrc > kCellMaxCurrent || isrc < kCellMinCurrent) {
    return ErrorCode::kCurrentOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellISourceSet(cell, isrc));
}

ErrorCode EthernetClient::SetAllCellVoltage(float voltage) const {
  if (voltage > kCellMaxVoltage || voltage < kCellMinVoltage) {
    return ErrorCode::kVoltageOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellVSetAll(voltage));
}

ErrorCode EthernetClient::SetCellVoltage(unsigned int cell,
                                         float voltage) const {
  if (cell >= kCellCount) {
    return ErrorCode::kChannelIndexOutOfRange;
  }

  if (voltage > kCellMaxVoltage || voltage < kCellMinVoltage) {
    return ErrorCode::kVoltageOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellVSet(cell, voltage));
}

ErrorCode EthernetClient::EnableAllCells(bool enable) const {
  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellEnableAll(enable));
}

ErrorCode EthernetClient::EnableCell(unsigned int cell, bool enable) const {
  if (cell >= kCellCount) {
    return ErrorCode::kChannelIndexOutOfRange;
  }

  std::lock_guard lg{mutex_};
  return SendFrame(messages::PackCellEnable(cell, enable));
}

Result<float> EthernetClient::GetCellVoltage(unsigned int cell) const {
  if (cell >= kCellCount) {
    return Err(ErrorCode::kChannelIndexOutOfRange);
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return cell_voltages_[cell];
}

Result<std::array<float, kCellCount>> EthernetClient::GetAllCellVoltage()
    const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return cell_voltages_;
}

ErrorCode EthernetClient::GetAllCellVoltage(std::span<float> voltages) const {
  if (voltages.size() < kCellCount) {
    return ErrorCode::kInvalidArgument;
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return NotConnectedHelper();
  }

  std::ranges::copy(cell_voltages_, voltages.begin());

  return ErrorCode::kSuccess;
}

Result<float> EthernetClient::GetCellCurrent(unsigned int cell) const {
  if (cell >= kCellCount) {
    return Err(ErrorCode::kChannelIndexOutOfRange);
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return cell_currents_[cell];
}

Result<std::array<float, kCellCount>> EthernetClient::GetAllCellCurrent()
    const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return cell_currents_;
}

ErrorCode EthernetClient::GetAllCellCurrent(std::span<float> currents) const {
  if (currents.size() < kCellCount) {
    return ErrorCode::kInvalidArgument;
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return NotConnectedHelper();
  }

  std::ranges::copy(cell_currents_, currents.begin());

  return ErrorCode::kSuccess;
}

Result<float> EthernetClient::GetAnalogInput(unsigned int analog_input) const {
  if (analog_input >= kAnalogInputCount) {
    return Err(ErrorCode::kAiIndexOutOfRange);
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return analog_inputs_[analog_input];
}

Result<std::array<float, kAnalogInputCount>> EthernetClient::GetAllAnalogInput()
    const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return analog_inputs_;
}

ErrorCode EthernetClient::GetAllAnalogInput(
    std::span<float> analog_inputs) const {
  if (analog_inputs.size() < kAnalogInputCount) {
    return ErrorCode::kInvalidArgument;
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return NotConnectedHelper();
  }

  std::ranges::copy(analog_inputs_, analog_inputs.begin());

  return ErrorCode::kSuccess;
}

Result<bool> EthernetClient::GetDIOState(unsigned int dio) const {
  if (dio >= kDioCount) {
    return Err(ErrorCode::kDioIndexOutOfRange);
  }

  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return dio_states_[dio];
}

Result<std::array<bool, kDioCount>> EthernetClient::GetAllDIOState() const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return dio_states_;
}

ErrorCode EthernetClient::GetAllDIOState(std::span<bool> states) const {
  if (states.size() < kDioCount) {
    return ErrorCode::kInvalidArgument;
  }

  if (!connected_) {
    return NotConnectedHelper();
  }

  std::lock_guard lg{mutex_};

  std::ranges::copy(dio_states_, states.begin());

  return ErrorCode::kSuccess;
}

Result<UnitStatus> EthernetClient::GetStatus() const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return status_;
}

Result<UnitConfig> EthernetClient::GetUnitConfig() const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  if (config_.IsEmpty()) {
    return Err(ErrorCode::kNoConfigData);
  }

  return unit_config_;
}

ErrorCode EthernetClient::Reset() const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtResetCommand());
}

ErrorCode EthernetClient::SetIPAddress(std::string_view ip) const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtSetIP(ip));
}

ErrorCode EthernetClient::QueryConfig() const {
  std::lock_guard lg{mutex_};

  return SendFrame(config::FmtQueryConfig());
}

ErrorCode EthernetClient::SetConfigOption(std::string_view opt,
                                          int value) const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtSetOption(opt, value));
}

ErrorCode EthernetClient::SetConfigOption(std::string_view opt,
                                          bool value) const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtSetOption(opt, value));
}

ErrorCode EthernetClient::SetConfigOption(std::string_view opt,
                                          std::string_view value) const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtSetOption(opt, value));
}

ErrorCode EthernetClient::SetConfigOption(std::string_view opt,
                                          double value) const {
  std::lock_guard lg{mutex_};
  return SendFrame(config::FmtSetOption(opt, value));
}

ErrorCode EthernetClient::EnableUDPData(bool enable) const {
  std::lock_guard lg{mutex_};
  return SetConfigOption("UDPData.UDPDataEnable", enable);
}

ErrorCode EthernetClient::SetUDPDataPort(std::uint16_t port) const {
  std::lock_guard lg{mutex_};
  return SetConfigOption("UDPData.UDPDataPort", static_cast<int>(port));
}

ErrorCode EthernetClient::SetUDPDataPeriod(std::uint32_t period) const {
  std::lock_guard lg{mutex_};
  return SetConfigOption("UDPData.UDPDataPeriod_mS", static_cast<int>(period));
}

ErrorCode EthernetClient::SetBoxID(std::uint8_t id) const {
  if (id > 0xf) {
    return ErrorCode::kInvalidBoxId;
  }

  std::lock_guard lg{mutex_};
  return SetConfigOption("CAN.BoxID", static_cast<int>(id));
}

ErrorCode EthernetClient::SetCANPeriod(std::uint32_t period) const {
  std::lock_guard lg{mutex_};
  return SetConfigOption("CAN.WritePeriod_ms", static_cast<int>(period));
}

ErrorCode EthernetClient::EnableCellInhibit(bool enable) const {
  std::lock_guard lg{mutex_};
  return SetConfigOption("OperationalModes.CellInhibitEnable", enable);
}

Result<bool> EthernetClient::GetConfigValueBool(std::string_view opt) const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return config_.GetValBool(std::string(opt));
}

Result<int> EthernetClient::GetConfigValueInt(std::string_view opt) const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return config_.GetValInt(std::string(opt));
}

Result<float> EthernetClient::GetConfigValueFloat(std::string_view opt) const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return config_.GetValFloat(std::string(opt));
}

Result<std::string> EthernetClient::GetConfigValueString(
    std::string_view opt) const {
  std::lock_guard lg{mutex_};

  if (!connected_) {
    return Err(NotConnectedHelper());
  }

  return config_.GetValString(std::string(opt));
}

void EthernetClient::UdpDataReceived(
    std::span<const std::uint8_t> data) noexcept {
  std::lock_guard lg{mutex_};

  // dirty unused for now...
  [[maybe_unused]]
  ErrorCode err = ErrorCode::kSuccess;

  // normal non-config UDP packet is always 180 bytes, consisting of 10 18-byte
  // frames
  if (data.size() == 180) {
    for (int i = 0; i < 10; i++) {
      auto frame = Frame::Parse(data.subspan(i * 18, 18));

      if (frame) {
        switch (frame->GetArbId()) {
          case ArbId::kVReadBack1_4:
          case ArbId::kVReadBack5_8:
          case ArbId::kVReadBack9_12:
            err = messages::ParseCellVReadBack(*frame, cell_voltages_);
            break;
          case ArbId::kIReadBack1_4:
          case ArbId::kIReadBack5_8:
          case ArbId::kIReadBack9_12:
            err = messages::ParseCellIReadBack(*frame, cell_currents_);
            break;
          case ArbId::kAIReadBack1_4:
          case ArbId::kAIReadBack5_8:
            err = messages::ParseAIReadBack(*frame, analog_inputs_);
            break;
          case ArbId::kDIOReadBack1_8:
            err = messages::ParseDIOReadBack(*frame, dio_states_);
            break;
          case ArbId::kStatusReadBack:
            err = messages::ParseStatusReadBack(*frame, status_);
            break;
          default:
            err = ErrorCode::kFrameIdMismatch;
            break;
        }
      }
    }
  } else if (config::IsConfigResponse(data)) {
    config_.Load(data);
    unit_config_ = config_.ToUnitConfig();
  }

  // TODO: does this make sense?
  // Preferable to continue on receipt of an invalid frame. Maybe be valuable in
  // future to expose a count of invalid frames.
  /*
  if (err != ErrorCode::kSuccess) {
    client_error_ = err;
  }
  */
}

void EthernetClient::ConnectionError(ErrorCode e) noexcept {
  std::lock_guard lg{mutex_};
  client_error_ = e;
  connected_ = false;
}

ErrorCode EthernetClient::NotConnectedHelper() const noexcept {
  if (client_error_ != ErrorCode::kSuccess) {
    return std::exchange(client_error_, ErrorCode::kSuccess);
  }
  return ErrorCode::kNotConnected;
}

ErrorCode EthernetClient::SendFrame(
    const Result<std::span<const std::uint8_t>>& frame) const {
  if (!frame) {
    return frame.error();
  }

  if (!connected_) {
    return NotConnectedHelper();
  }

  auto size_pkt =
      messages::CreateSizePacket(static_cast<std::uint8_t>(frame->size()));
  ErrorCode err = tcp_client_->SendData(size_pkt);

  if (err != ErrorCode::kSuccess) {
    return err;
  }

  return tcp_client_->SendData(*frame);
}

ErrorCode EthernetClient::SendFrame(const Result<Frame>& frame) const {
  return SendFrame(frame.transform(&Frame::GetRawFrame));
}

ErrorCode EthernetClient::SendFrame(const Result<std::string>& frame) const {
  return SendFrame(frame.transform([](const std::string& str) {
    return std::span{reinterpret_cast<const std::uint8_t*>(str.data()),
                     str.size()};
  }));
}

}  // namespace bci::bs120x

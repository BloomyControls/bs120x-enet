/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include <bci/bs120x/CInterface.h>
#include <bci/bs120x/EthernetClient.h>

#include <algorithm>
#include <string>

using namespace bci::bs120x;
using ec = bci::bs120x::ErrorCode;
using cl = bci::bs120x::EthernetClient;

static EthernetClient& GetClient(Bs120xEnetHandle handle) {
  return *(EthernetClient*)handle;
}

template <class... Args>
static int WrapSet(ErrorCode (EthernetClient::*func)(Args...) const,
                   Bs120xEnetHandle handle, Args... args) noexcept try {
  if (!handle || !func) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return static_cast<int>((GetClient(handle).*func)(args...));
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

template <class T, class... Args>
static int WrapGet(Result<T> (EthernetClient::*func)(Args...) const,
                   Bs120xEnetHandle handle, T* res, Args... args) noexcept try {
  if (!handle || !func || !res) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  ec err = ec::kSuccess;
  if (auto result = (GetClient(handle).*func)(args...)) {
    *res = *result;
  } else {
    err = result.error();
  }

  return static_cast<int>(err);
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

template <class... Args>
static int WrapGet(ErrorCode (EthernetClient::*func)(Args...) const,
                   Bs120xEnetHandle handle, Args... args) noexcept try {
  if (!handle || !func) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return static_cast<int>((GetClient(handle).*func)(args...));
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

unsigned int Bs120xEnet_Version() {
  return BS120XENET_VERSION;
}

const char* Bs120xEnet_ErrorMessage(int error) {
  return ErrorMessage(static_cast<ErrorCode>(error));
}

int Bs120xEnet_Init(Bs120xEnetHandle* handle_out) try {
  if (!handle_out) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  EthernetClient*& client_ptr = *(EthernetClient**)handle_out;
  if (!client_ptr) {
    client_ptr = new EthernetClient();
  }

  return static_cast<int>(ec::kSuccess);
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

void Bs120xEnet_Destroy(Bs120xEnetHandle* handle) {
  if (handle) {
    delete (EthernetClient*)*handle;
    *handle = nullptr;
  }
}

int Bs120xEnet_Connect(Bs120xEnetHandle handle, const char* interface_ip,
                       const char* device_ip, uint16_t udp_port,
                       uint32_t udp_timeout, uint16_t tcp_port) try {
  if (!handle || !interface_ip || !device_ip) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return static_cast<int>(GetClient(handle).Connect(
      interface_ip, device_ip, udp_port, udp_timeout, tcp_port));
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

int Bs120xEnet_Disconnect(Bs120xEnetHandle handle) try {
  if (!handle) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  GetClient(handle).Disconnect();

  return static_cast<int>(ec::kSuccess);
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

int Bs120xEnet_SetDIOStates(Bs120xEnetHandle handle, const bool output[],
                            size_t output_len, const bool direction[],
                            size_t direction_len) {
  if (!output || !direction || !output_len || !direction_len) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapSet(&cl::SetDIOStates, handle, std::span{output, output_len},
                 std::span{direction, direction_len});
}

int Bs120xEnet_SetAnalogOutputs(Bs120xEnetHandle handle, const float voltages[],
                                size_t voltages_len) {
  if (!voltages || !voltages_len) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapSet(&cl::SetAnalogOutputs, handle,
                 std::span{voltages, voltages_len});
}

int Bs120xEnet_SetOpModeConfig(Bs120xEnetHandle handle,
                               struct Bs120xOpModeConfig config) {
  OpModeConfig cfg{
      .calibration_enable = config.calibration_mode,
      .pid_disable = config.pid_disable,
      .precision_mode_enable = config.precision_mode_enable,
  };

  return WrapSet(&cl::SetOpModeConfig, handle, cfg);
}

int Bs120xEnet_SetAllCellCurrents(Bs120xEnetHandle handle, float isrc,
                                  float isnk) {
  return WrapSet(&cl::SetAllCellCurrents, handle, isrc, isnk);
}

int Bs120xEnet_SetCellSinkCurrent(Bs120xEnetHandle handle, unsigned int cell,
                                  float isnk) {
  return WrapSet(&cl::SetCellSinkCurrent, handle, cell, isnk);
}

int Bs120xEnet_SetCellSourceCurrent(Bs120xEnetHandle handle, unsigned int cell,
                                    float isrc) {
  return WrapSet(&cl::SetCellSourceCurrent, handle, cell, isrc);
}

int Bs120xEnet_SetAllCellVoltage(Bs120xEnetHandle handle, float voltage) {
  return WrapSet(&cl::SetAllCellVoltage, handle, voltage);
}

int Bs120xEnet_SetCellVoltage(Bs120xEnetHandle handle, unsigned int cell,
                              float voltage) {
  return WrapSet(&cl::SetCellVoltage, handle, cell, voltage);
}

int Bs120xEnet_EnableAllCells(Bs120xEnetHandle handle, bool enable) {
  return WrapSet(&cl::EnableAllCells, handle, enable);
}

int Bs120xEnet_EnableCell(Bs120xEnetHandle handle, unsigned int cell,
                          bool enable) {
  return WrapSet(&cl::EnableCell, handle, cell, enable);
}

int Bs120xEnet_GetCellVoltage(Bs120xEnetHandle handle, unsigned int cell,
                              float* voltage_out) {
  return WrapGet(&cl::GetCellVoltage, handle, voltage_out, cell);
}

int Bs120xEnet_GetAllCellVoltage(Bs120xEnetHandle handle, float voltage_out[],
                                 size_t voltage_out_len) {
  if (!voltage_out || voltage_out_len < kCellCount) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapGet(&cl::GetAllCellVoltage, handle,
                 std::span{voltage_out, voltage_out_len});
}

int Bs120xEnet_GetCellCurrent(Bs120xEnetHandle handle, unsigned int cell,
                              float* current_out) {
  return WrapGet(&cl::GetCellCurrent, handle, current_out, cell);
}

int Bs120xEnet_GetAllCellCurrent(Bs120xEnetHandle handle, float current_out[],
                                 size_t current_out_len) {
  if (!current_out || current_out_len < kCellCount) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapGet(&cl::GetAllCellCurrent, handle,
                 std::span{current_out, current_out_len});
}

int Bs120xEnet_GetAnalogInput(Bs120xEnetHandle handle,
                              unsigned int analog_input, float* voltage_out) {
  return WrapGet(&cl::GetAnalogInput, handle, voltage_out, analog_input);
}

int Bs120xEnet_GetAllAnalogInput(Bs120xEnetHandle handle, float voltage_out[],
                                 size_t voltage_out_len) {
  if (!voltage_out || voltage_out_len < kAnalogInputCount) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapGet(&cl::GetAllAnalogInput, handle,
                 std::span{voltage_out, voltage_out_len});
}

int Bs120xEnet_GetDIOState(Bs120xEnetHandle handle, unsigned int dio,
                           bool* state_out) {
  return WrapGet(&cl::GetDIOState, handle, state_out, dio);
}

int Bs120xEnet_GetAllDIOState(Bs120xEnetHandle handle, bool state_out[],
                              size_t state_out_len) {
  if (!state_out || state_out_len < kDioCount) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  return WrapGet(&cl::GetAllDIOState, handle,
                 std::span{state_out, state_out_len});
}

int Bs120xEnet_GetStatus(Bs120xEnetHandle handle,
                         struct Bs120xStatus* status_out) try {
  if (!status_out) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  UnitStatus sts{};
  int err = WrapGet(&cl::GetStatus, handle, &sts);
  if (err == static_cast<int>(ec::kSuccess)) {
    std::ranges::copy(sts.fan_fail_status, status_out->fan_fail_status);
    status_out->inhibit_state = sts.inhibit_state;
    std::ranges::copy(sts.temps, status_out->temps);
  }

  return err;
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

int Bs120xEnet_GetUnitConfig(Bs120xEnetHandle handle,
                             struct Bs120xUnitConfig* unit_config_out) try {
  if (!unit_config_out) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  UnitConfig cfg{};
  int err = WrapGet(&cl::GetUnitConfig, handle, &cfg);
  if (err == static_cast<int>(ec::kSuccess)) {
    *unit_config_out = {};

    cfg.serial_number.copy(unit_config_out->serial_number,
                           sizeof(unit_config_out->serial_number) - 1);
    cfg.firmware_version.copy(unit_config_out->firmware_version,
                              sizeof(unit_config_out->firmware_version) - 1);
    cfg.calibration_date.copy(unit_config_out->calibration_date,
                              sizeof(unit_config_out->calibration_date) - 1);
    cfg.ip_address.copy(unit_config_out->ip_address,
                        sizeof(unit_config_out->ip_address) - 1);
    unit_config_out->cell_inhibit_enable = cfg.cell_inhibit_enable;
    unit_config_out->udp_data_port = cfg.udp_data_port;
    unit_config_out->udp_data_period = cfg.udp_data_period;
    unit_config_out->udp_data_broadcast_enable = cfg.udp_data_broadcast_enable;
    unit_config_out->box_id = cfg.box_id;
    unit_config_out->can_data_period = cfg.can_data_period;
  }

  return err;
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

int Bs120xEnet_Reset(Bs120xEnetHandle handle) {
  return WrapSet(&cl::Reset, handle);
}

int Bs120xEnet_SetIPAddress(Bs120xEnetHandle handle, const char* ip) {
  if (!ip) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapSet(&cl::SetIPAddress, handle, std::string_view{ip});
}

int Bs120xEnet_QueryConfig(Bs120xEnetHandle handle) {
  return WrapSet(&cl::QueryConfig, handle);
}

int Bs120xEnet_SetConfigOptionInt(Bs120xEnetHandle handle, const char* opt,
                                  int value) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapSet(&cl::SetConfigOption, handle, std::string_view{opt}, value);
}

int Bs120xEnet_SetConfigOptionBool(Bs120xEnetHandle handle, const char* opt,
                                   bool value) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapSet(&cl::SetConfigOption, handle, std::string_view{opt}, value);
}

int Bs120xEnet_SetConfigOptionString(Bs120xEnetHandle handle, const char* opt,
                                     const char* value) {
  if (!opt || !value) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapSet(&cl::SetConfigOption, handle, std::string_view{opt},
                 std::string_view{value});
}

int Bs120xEnet_SetConfigOptionFloat(Bs120xEnetHandle handle, const char* opt,
                                    double value) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapSet(&cl::SetConfigOption, handle, std::string_view{opt}, value);
}

int Bs120xEnet_EnableUDPData(Bs120xEnetHandle handle, bool enable) {
  return WrapSet(&cl::EnableUDPData, handle, enable);
}

int Bs120xEnet_SetUDPDataPort(Bs120xEnetHandle handle, uint16_t port) {
  return WrapSet(&cl::SetUDPDataPort, handle, port);
}

int Bs120xEnet_SetUDPDataPeriod(Bs120xEnetHandle handle, uint32_t period) {
  return WrapSet(&cl::SetUDPDataPeriod, handle, period);
}

int Bs120xEnet_SetBoxId(Bs120xEnetHandle handle, uint8_t id) {
  return WrapSet(&cl::SetBoxID, handle, id);
}

int Bs120xEnet_SetCANPeriod(Bs120xEnetHandle handle, uint32_t period) {
  return WrapSet(&cl::SetCANPeriod, handle, period);
}

int Bs120xEnet_EnableCellInhibit(Bs120xEnetHandle handle, bool enable) {
  return WrapSet(&cl::EnableCellInhibit, handle, enable);
}

int Bs120xEnet_GetConfigValueBool(Bs120xEnetHandle handle, const char* opt,
                                  bool* value_out) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapGet(&cl::GetConfigValueBool, handle, value_out,
                 std::string_view{opt});
}

int Bs120xEnet_GetConfigValueInt(Bs120xEnetHandle handle, const char* opt,
                                 int* value_out) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapGet(&cl::GetConfigValueInt, handle, value_out,
                 std::string_view{opt});
}

int Bs120xEnet_GetConfigValueFloat(Bs120xEnetHandle handle, const char* opt,
                                   float* value_out) {
  if (!opt) {
    return static_cast<int>(ec::kInvalidArgument);
  }
  return WrapGet(&cl::GetConfigValueFloat, handle, value_out,
                 std::string_view{opt});
}

int Bs120xEnet_GetConfigValueString(Bs120xEnetHandle handle, const char* opt,
                                    char buf[], size_t len) try {
  if (!opt || !buf || len == 0) {
    return static_cast<int>(ec::kInvalidArgument);
  }

  std::string res;
  int err =
      WrapGet(&cl::GetConfigValueString, handle, &res, std::string_view{opt});
  if (err == static_cast<int>(ec::kSuccess)) {
    if (len < res.size() + 1) {
      return static_cast<int>(ec::kBufferTooSmall);
    }

    res.copy(buf, len);
    buf[res.size()] = '\0';
  }

  return err;
} catch (const std::bad_alloc&) {
  return static_cast<int>(ec::kAllocationFailed);
} catch (...) {
  return static_cast<int>(ec::kUnexpectedException);
}

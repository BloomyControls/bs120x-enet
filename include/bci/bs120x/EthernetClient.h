/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/**
 * @file
 * Ethernet client declaration.
 */

#ifndef BS120X_INCLUDE_BCI_BS120X_ETHERNETCLIENT_H
#define BS120X_INCLUDE_BCI_BS120X_ETHERNETCLIENT_H

#include <array>
#include <cstdint>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>

#include "CommonTypes.h"
#include "ConfigFile.h"
#include "bs120xenet_export.h"

// These comments make sure doxygen generates docs properly.
/**
 * @namespace bci
 * @brief Bloomy Controls root namespace.
 */
/**
 * @namespace bci::bs120x
 * @brief Contains BS120x-related code.
 */

namespace bci::bs120x {

class Frame;

/**
* @brief Ethernet client for communicating with the Bloomy Controls Battery
Simulator (BS120x).
*
* This class implements commands and readbacks for all BS120x IO, as well as
* configuration utilities.
*/
class EthernetClient {
 public:
  /**
   * @brief Get the library version as a decimal integer. For example, version
   * 1.3.2 would return 10302.
   *
   * @return Library version.
   */
  BS120X_API static unsigned int Version() noexcept;

  /// Default constructor.
  BS120X_API EthernetClient();

  EthernetClient(const EthernetClient&) = delete;
  EthernetClient& operator=(const EthernetClient&) = delete;

  EthernetClient(EthernetClient&& other) noexcept = delete;
  EthernetClient& operator=(EthernetClient&& rhs) noexcept = delete;

  /// Default destructor.
  BS120X_API ~EthernetClient();

  /**
   * @brief Connect to a unit.
   *
   * @param[in] interface_ip IP address of the local NIC to connect from.
   *
   * @param[in] device_ip IP address of the unit.
   *
   * @param[in] udp_port UDP data port for the unit.
   *
   * @param[in] udp_timeout Timeout in milliseconds to receive UDP data
   * broadcast. A negative value will wait infinitely.
   *
   * @param[in] tcp_port TCP port to send commands on. BS120x default is 12345.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode Connect(std::string_view interface_ip,
                               std::string_view device_ip,
                               std::uint16_t udp_port,
                               std::int32_t udp_timeout = 1000,
                               std::uint16_t tcp_port = 12345) noexcept;

  /**
   * @brief Disconnect from the unit.
   */
  BS120X_API void Disconnect() noexcept;

  /**
   * @brief Set the output states and directions for all DIO.
   *
   * @param[in] outputs The output states for the DIO. Must have a length equal
   * to the number of unit DIO. A TRUE value corresponds to a HIGH output.
   *
   * @param[in] directions The directions for the DIO. Must have a length equal
   * to the number of unit DIO. A TRUE value corresponds to an output.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetDIOStates(std::span<const bool> outputs,
                                    std::span<const bool> directions) const;

  /**
   * @brief Set the voltages for the all analog outputs.
   *
   * @param[in] voltages The voltage setpoints for the analog outputs.
   * Must have a length equal to the number of unit analog outputs.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetAnalogOutputs(std::span<const float> voltages) const;

  /**
   * @brief Configures the operating mode of the unit.
   *
   * @param[in] config The operating mode options to set.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetOpModeConfig(OpModeConfig config) const;

  /**
   * @brief Sets the sinking and sourcing current limits for all cells.
   *
   * @param[in] isrc The sourcing current limit in Amps.
   *
   * @param[in] isnk The sinking current limit in Amps.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetAllCellCurrents(float isrc, float isnk) const;

  /**
   * @brief Sets the sinking current limit for a single cell.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @param[in] isnk The sinking current limit in Amps for the cell.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetCellSinkCurrent(unsigned int cell, float isnk) const;

  /**
   * @brief Sets the sourcing current limit for a cell.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @param[in] isrc The sourcing current limit in Amps for the cell.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetCellSourceCurrent(unsigned int cell,
                                            float isrc) const;

  /**
   * @brief Sets the voltage for all cells.
   *
   * @param[in] voltage The voltage setpoint.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetAllCellVoltage(float voltage) const;

  /**
   * @brief Sets the voltage for a single cell.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @param[in] voltage The voltage setpoint.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetCellVoltage(unsigned int cell, float voltage) const;

  /**
   * @brief Enables or disables all cells.
   *
   * @param[in] enable The state for the cells. A value of TRUE will enable all
   * cells.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode EnableAllCells(bool enable) const;

  /**
   * @brief Enables or disables a single cell.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @param[in] enable The state for the cell. A value of TRUE will enable the
   * cell.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode EnableCell(unsigned int cell, bool enable) const;

  /**
   * @brief Gets the voltage of a cell.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @return Result containing the cell voltage or an error code.
   */
  BS120X_API Result<float> GetCellVoltage(unsigned int cell) const;

  /**
   * @brief Gets the voltages of all cells.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @return Result containing an array of voltages or an error code.
   */
  BS120X_API Result<std::array<float, kCellCount>> GetAllCellVoltage() const;

  /**
   * @brief Gets the voltages of all cells.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[out] voltages Array to store the returned cell voltages.
   * Length must be equal to the number of cells.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode GetAllCellVoltage(std::span<float> voltages) const;

  /**
   * @brief Gets the current of a cell in Amps.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[in] cell The cell to target. Cells are 0-indexed.
   *
   * @return Result containing the current or an error code.
   */
  BS120X_API Result<float> GetCellCurrent(unsigned int cell) const;

  /**
   * @brief Get the currents of all cells in Amps.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @return Result containing an array of currents or an error code.
   */
  BS120X_API Result<std::array<float, kCellCount>> GetAllCellCurrent() const;

  /**
   * @brief Get the currents of all cells in Amps.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[out] currents Array to store the returned cell currents.
   * Length must be equal to the number of cells.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode GetAllCellCurrent(std::span<float> currents) const;

  /**
   * @brief Gets the voltage of an analog input.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[in] analog_input The analog input to target.
   *
   * @return Result containing a voltage or an error code.
   */
  BS120X_API Result<float> GetAnalogInput(unsigned int analog_input) const;

  /**
   * @brief Gets the voltages of all analog inputs.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @return Result containing an array of voltages or an error code.
   */
  BS120X_API Result<std::array<float, kAnalogInputCount>> GetAllAnalogInput()
      const;

  /**
   * @brief Gets the voltages of all analog inputs.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[out] analog_inputs Array to store the returned voltages.
   * Length must be equal to the number of analog inputs.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode GetAllAnalogInput(std::span<float> analog_inputs) const;

  /**
   * @brief Gets the state of a DIO.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[in] dio The DIO to target.
   *
   * @return Result containing a state or an error code.
   */
  BS120X_API Result<bool> GetDIOState(unsigned int dio) const;

  /**
   * @brief Gets the states of all DIO.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @return Result containing an array of states or an error code.
   */
  BS120X_API Result<std::array<bool, kDioCount>> GetAllDIOState() const;

  /**
   * @brief Gets the states of all DIO.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @param[out] states Array to store the returned states. Length must be equal
   * to the number of DIO.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode GetAllDIOState(std::span<bool> states) const;

  /**
   * @brief Gets the status of the unit.
   *
   * @note The BS120x unit must have UDP data broadcast enabled for this method
   * to return valid data.
   *
   * @return Result containing a UnitStatus structure or an error code.
   */
  BS120X_API Result<UnitStatus> GetStatus() const;

  /**
   * @brief Gets the configuration values of a unit.
   *
   * @note QueryConfig must be called prior to calling this method.
   *
   * @return Result containing a UnitConfig structure or an error code.
   */
  BS120X_API Result<UnitConfig> GetUnitConfig() const;

  /**
   * @brief Commands the unit to reset.
   *
   * @note Disconnect and Connect must be called after this method to
   * re-establish a connection with the unit.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode Reset() const;

  /**
   * @brief Sets the unit IP address.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] ip New IP address for the unit. Last octet may not have a
   * value of 1 or 255.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetIPAddress(std::string_view ip) const;

  /**
   * @brief Sends a request to the unit for its configuration information.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode QueryConfig() const;

  /**
   * @brief Sets a configuration option for the unit.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] opt The configuration option to set.
   *
   * @param[in] value The value to set the configuration option to.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetConfigOption(std::string_view opt, int value) const;

  /**
   * @brief Sets a configuration option for the unit.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] opt The configuration option to set.
   *
   * @param[in] value The value to set the configuration option to.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetConfigOption(std::string_view opt, bool value) const;

  /**
   * @brief Sets a configuration option for the unit.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] opt The configuration option to set.
   *
   * @param[in] value The value to set the configuration option to.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetConfigOption(std::string_view opt,
                                       std::string_view value) const;

  /**
   * @brief Sets a configuration option for the unit.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] opt The configuration option to set.
   *
   * @param[in] value The value to set the configuration option to.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetConfigOption(std::string_view opt,
                                       double value) const;

  /**
   * @brief Enables the UDP data broadcast for the unit.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] enable A value of TRUE will enable the UDP data broadcast.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode EnableUDPData(bool enable) const;

  /**
   * @brief Sets the port for the unit's UDP data broadcast.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] port The new UDP port.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetUDPDataPort(std::uint16_t port) const;

  /**
   * @brief Sets the period for the unit's UDP data broadcast.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] period The new period in milliseconds.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetUDPDataPeriod(std::uint32_t period) const;

  /**
   * @brief Sets the unit's box id.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] id The new box ID. Valid values are 0-15.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetBoxID(std::uint8_t id) const;

  /**
   * @brief Sets the period for the unit's CAN broadcast.
   *
   * @note Unit must be reset for changes to take effect.
   *
   * @param[in] period The new period in milliseconds.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetCANPeriod(std::uint32_t period) const;

  /**
   * @brief Enables or disables the cell inhibit lines.
   *
   * @param[in] enable The enable state for the cell inhibit lines.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode EnableCellInhibit(bool enable) const;

  /**
   * @brief Get a boolean configuration value.
   *
   * @note QueryConfig must be called prior to calling this method.
   *
   * @param[in] opt The configuration option to look up.
   *
   * @return Result containing the value or an error code.
   */
  BS120X_API Result<bool> GetConfigValueBool(std::string_view opt) const;

  /**
   * @brief Get an integer configuration value.
   *
   * @note QueryConfig must be called prior to calling this method.
   *
   * @param[in] opt The configuration option to look up.
   *
   * @return Result containing the value or an error code.
   */
  BS120X_API Result<int> GetConfigValueInt(std::string_view opt) const;

  /**
   * @brief Get a floating point configuration value.
   *
   * @note QueryConfig must be called prior to calling this method.
   *
   * @param[in] opt The configuration option to look up.
   *
   * @return Result containing the value or an error code.
   */
  BS120X_API Result<float> GetConfigValueFloat(std::string_view opt) const;

  /**
   * @brief Get a string configuration value.
   *
   * @note QueryConfig must be called prior to calling this method.
   *
   * @param[in] opt The configuration option to look up.
   *
   * @return Result containing the value or an error code.
   */
  BS120X_API Result<std::string> GetConfigValueString(
      std::string_view opt) const;

 private:
  class UdpListener;
  class TcpClient;

  std::unique_ptr<UdpListener> udp_listener_;
  std::unique_ptr<TcpClient> tcp_client_;
  bool connected_;

  mutable ErrorCode client_error_;
  mutable std::recursive_mutex mutex_;

  std::array<float, kCellCount> cell_voltages_;
  std::array<float, kCellCount> cell_currents_;
  std::array<float, kAnalogInputCount> analog_inputs_;
  std::array<bool, kDioCount> dio_states_;
  UnitStatus status_;
  ConfigFile config_;
  UnitConfig unit_config_;

  void UdpDataReceived(std::span<const std::uint8_t> data) noexcept;

  void ConnectionError(ErrorCode e) noexcept;

  // helper to make it easier to handle client errors when we're disconnected
  // note: this does not acquire the mutex! it is expected that the calling code
  // already has ownership
  ErrorCode NotConnectedHelper() const noexcept;

  ErrorCode SendFrame(const Result<std::span<const std::uint8_t>>& frame) const;

  ErrorCode SendFrame(const Result<Frame>& frame) const;

  ErrorCode SendFrame(const Result<std::string>& frame) const;
};

}  // namespace bci::bs120x

#endif /* BS120X_INCLUDE_BCI_BS120X_ETHERNETCLIENT_H */

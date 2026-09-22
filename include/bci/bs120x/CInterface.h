/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/**
 * @file
 * @brief C-style interface for the library.
 */

#ifndef BS120X_INCLUDE_BCI_BS120X_CINTERFACE_H
#define BS120X_INCLUDE_BCI_BS120X_CINTERFACE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "bs120xenet_export.h"

/**
 * @defgroup CInterface C Interface
 *
 * This interface provides a wrapper around the C++ library for use in C and
 * other languages.
 *
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @defgroup CErrors Error Codes
 * Error codes returned by the Ethernet client functions.
 *
 * @{
 */
/// Success (no error)
#define BCI_ENET_ERR_SUCCESS (0)
/// Channel index out of range
#define BCI_ENET_ERR_CHANNEL_INDEX_OUT_OF_RANGE (-100)
/// Invalid IP address
#define BCI_ENET_ERR_INVALID_IP_ADDRESS (-101)
/// Driver already connected
#define BCI_ENET_ERR_ALREADY_CONNECTED (-102)
/// TCP client already connected
#define BCI_ENET_ERR_TCP_ALREADY_CONNECTED (-103)
/// UDP listener already connected
#define BCI_ENET_ERR_UDP_ALREADY_CONNECTED (-104)
/// Failed to open socket for UDP listener
#define BCI_ENET_ERR_UDP_SOCKET_OPEN_FAILED (-105)
/// Failed to configure TCP socket
#define BCI_ENET_ERR_TCP_SOCKET_CONFIG_FAILED (-106)
/// Failed to configure UDP socket
#define BCI_ENET_ERR_UDP_SOCKET_CONFIG_FAILED (-107)
/// Failed to bind UDP socket
#define BCI_ENET_ERR_UDP_SOCKET_BIND_FAILED (-108)
/// Failed to connect TCP client
#define BCI_ENET_ERR_TCP_CONNECT_FAILED (-109)
/// Failed to read message
#define BCI_ENET_ERR_READ_FAILED (-110)
/// Failed to send message
#define BCI_ENET_ERR_SEND_FAILED (-111)
/// Not connected
#define BCI_ENET_ERR_NOT_CONNECTED (-112)
/// Frame Arb ID doesn't match expected value
#define BCI_ENET_ERR_FRAME_ID_MISMATCH (-113)
/// Frame size does not match known messages
#define BCI_ENET_ERR_INVALID_FRAME_SIZE (-114)
/// Analog input index out of range
#define BCI_ENET_ERR_AI_INDEX_OUT_OF_RANGE (-115)
/// DIO index out of range
#define BCI_ENET_ERR_DIO_INDEX_OUT_OF_RANGE (-116)
/// Buffer too small
#define BCI_ENET_ERR_BUFFER_TOO_SMALL (-117)
/// Voltage out of range
#define BCI_ENET_ERR_VOLTAGE_OUT_OF_RANGE (-118)
/// Current out of range
#define BCI_ENET_ERR_CURRENT_OUT_OF_RANGE (-119)
/// UDP read timed out
#define BCI_ENET_ERR_READBACK_TIMED_OUT (-120)
/// Invalid box ID
#define BCI_ENET_ERR_INVALID_BOX_ID (-121)
/// Config data has not been queried
#define BCI_ENET_ERR_NO_CONFIG_DATA (-122)
/// Invalid configuration option
#define BCI_ENET_ERR_INVALID_CONFIG_OPTION (-123)
/// Invalid configuration option type
#define BCI_ENET_ERR_WRONG_CONFIG_OPTION_TYPE (-124)
/// Invalid argument
#define BCI_ENET_ERR_INVALID_ARGUMENT (-125)
/// Allocation failed
#define BCI_ENET_ERR_ALLOCATION_FAILED (-126)
/// Unexpected exception
#define BCI_ENET_ERR_UNEXPECTED_EXCEPTION (-127)
/** @} */

/**
 * @addtogroup CSystem
 * @{
 */

/// BS1201 operational mode configuration structure.
struct Bs120xOpModeConfig {
  bool calibration_mode;       ///< Enables or disables calibration mode.
  bool pid_disable;            ///< Enables or disables cell PID control.
  bool precision_mode_enable;  ///< Enables or disables precision mode.
};

/// BS1201 status structure.
struct Bs120xStatus {
  bool fan_fail_status[4];  ///< Unit fan status. TRUE indicates a fan failure.
  bool inhibit_state;       ///< Cell inhibit state. TRUE indicates ENABLED.
  float temps[3];           ///< Temperature sensor values in Celsius.
};

/** @} */

/**
 * @addtogroup CConfiguration
 * @{
 */

/// BS1201 unit configuration structure. Strings are null-terminated.
struct Bs120xUnitConfig {
  char serial_number[128];     ///< Serial number.
  char firmware_version[128];  ///< Firmware version.
  char calibration_date[128];  ///< Calibration date.
  bool cell_inhibit_enable;    ///< Indicates whether the unit's cell inhibit
                               ///< lines are enabled.
  char ip_address[32];         ///< IP address.
  uint16_t udp_data_port;      ///< UDP data broadcast port.
  uint32_t udp_data_period;    ///< UDP data broadcast interval in milliseconds.
  bool udp_data_broadcast_enable;  ///< Indicates whether UDP data broadcast is
                                   ///< enabled.
  uint8_t box_id;                  ///< Box ID.
  uint32_t can_data_period;        ///< CAN transmit period in milliseconds.
};
/** @} */

/// Ethernet client handle.
typedef void* Bs120xEnetHandle;

/**
 * @brief Get the library version as an unsigned base 10 integer. For example,
 * version 1.2.3 would return 10203.
 *
 * This is intended to be used to check for the existence of certain
 * functionality based on library version, particularly in wrappers (such as
 * Python).
 *
 * @return The library version.
 */
BS120X_API unsigned int Bs120xEnet_Version();

/**
 * @brief Get an error message to describe an error code returned by the driver.
 *
 * @param[in] error Error code.
 *
 * @return Null-terminated error message string.
 */
BS120X_API const char* Bs120xEnet_ErrorMessage(int error);

/**
 * @brief Initialize an Ethernet client. Must be destroyed by the caller!
 *
 * @param[out] handle_out Pointer to a handle to initialize. (Handle should be
 * zeroed.)
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_Init(Bs120xEnetHandle* handle_out);

/**
 * @brief Destroy an Ethernet client.
 *
 * @param[in,out] handle Handle pointer to a handle to destroy.
 */
BS120X_API void Bs120xEnet_Destroy(Bs120xEnetHandle* handle);

/**
 * @brief Connect to a unit.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] interface_ip IP address of the local NIC to connect from.
 *
 * @param[in] device_ip IP address of the unit.
 *
 * @param[in] udp_port UDP data port for the unit.
 *
 * @param[in] udp_timeout Timeout in milliseconds to receive UDP data broadcast.
 * A negative value will wait indefinitely.
 *
 * @param[in] tcp_port TCP port to send commands on. BS120x default is 12345.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_Connect(Bs120xEnetHandle handle,
                                  const char* interface_ip,
                                  const char* device_ip, uint16_t udp_port,
                                  uint32_t udp_timeout, uint16_t tcp_port);

/**
 * @brief Disconnect from the unit.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_Disconnect(Bs120xEnetHandle handle);

/**
 * @defgroup CAuxiliaryIO Auxiliary IO
 * Functions for controlling and reading auxiliary IO.
 * @{
 */

/**
 * @brief Set the output states and directions for all DIO.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] output Array of output states. A TRUE value corresponds to a HIGH
 * output.
 *
 * @param[in] output_len Length of the output array. Must be equal to the
 * number of DIO.
 *
 * @param[in] direction Array of DIO directions. A TRUE value corresponds
 * to an output.
 *
 * @param[in] direction_len Length of the direction array. Must be equal
 * to the number of DIO.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetDIOStates(Bs120xEnetHandle handle,
                                       const bool output[], size_t output_len,
                                       const bool direction[],
                                       size_t direction_len);

/**
 * @brief Set the voltages for all analog outputs.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] voltages Array of analog output voltages.
 *
 * @param[in] voltages_len Length of the voltages array. Must be equal to
 * the number of analog outputs.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetAnalogOutputs(Bs120xEnetHandle handle,
                                           const float voltages[],
                                           size_t voltages_len);

/** @} */

/**
 * @addtogroup CSystem
 * @{
 */

/**
 * @brief Configures the operating mode of the unit.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] config The operating mode options to set.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetOpModeConfig(Bs120xEnetHandle handle,
                                          struct Bs120xOpModeConfig config);

/** @} */

/**
 * @defgroup CCells Cells
 * Functions for controlling and reading back cells.
 *
 * @{
 */

/**
 * @brief Sets the sinking and sourcing current limits for all cells.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] isrc The sourcing current limit in Amps.
 *
 * @param[in] isnk The sinking current limit in Amps.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetAllCellCurrents(Bs120xEnetHandle handle,
                                             float isrc, float isnk);

/**
 * @brief Sets the sinking current limit for a single cell.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[in] isnk The sinking current limit in Amps for the cell.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetCellSinkCurrent(Bs120xEnetHandle handle,
                                             unsigned int cell, float isnk);

/**
 * @brief Sets teh sourcing current limit for a single cell.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[in] isrc The sourcing current limit in Amps for the cell.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetCellSourceCurrent(Bs120xEnetHandle handle,
                                               unsigned int cell, float isrc);

/**
 * @brief Sets the voltage for all cells.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] voltage The voltage setpoint.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetAllCellVoltage(Bs120xEnetHandle handle,
                                            float voltage);

/**
 * @brief Sets the voltage for a single cell.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[in] voltage The voltage setpoint.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetCellVoltage(Bs120xEnetHandle handle,
                                         unsigned int cell, float voltage);

/**
 * @brief Enables or disables all cells.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] enable The state for the cells. A value of TRUE will enable all
 * cells.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_EnableAllCells(Bs120xEnetHandle handle, bool enable);

/**
 * @brief Enables or disables a single cell.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[in] enable The state for the cell. A value of TRUE will enable the
 * cell.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_EnableCell(Bs120xEnetHandle handle, unsigned int cell,
                                     bool enable);

/**
 * @brief Gets the voltage of a cell.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[out] voltage_out Pointer to the returned voltage.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetCellVoltage(Bs120xEnetHandle handle,
                                         unsigned int cell, float* voltage_out);

/**
 * @brief Gets the voltages of all cells.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] voltage_out Array to store the returned voltages.
 *
 * @param[in] voltage_out_len Length of the voltages_out array. Must be
 * equal to the number of cells.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetAllCellVoltage(Bs120xEnetHandle handle,
                                            float voltage_out[],
                                            size_t voltage_out_len);

/**
 * @brief Gets the current of a cell in Amps.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] cell The cell to target. Cells are 0-indexed.
 *
 * @param[out] current_out Pointer to the returned current.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetCellCurrent(Bs120xEnetHandle handle,
                                         unsigned int cell, float* current_out);

/**
 * @brief Get the currents of all cells in Amps.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] current_out Array to store the returned currents.
 *
 * @param[in] current_out_len Length of the current_out array. Must be
 * equal to the number of cells.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetAllCellCurrent(Bs120xEnetHandle handle,
                                            float current_out[],
                                            size_t current_out_len);

/** @} */

/**
 * @addtogroup CAuxiliaryIO
 * @{
 */

/**
 * @brief Gets the voltage of an analog input.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] analog_input The analog input to target. Analog inputs are
 * 0-indexed.
 *
 * @param[out] voltage_out Pointer to the returned voltage.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetAnalogInput(Bs120xEnetHandle handle,
                                         unsigned int analog_input,
                                         float* voltage_out);

/**
 * @brief Gets the voltages of all analog inputs.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] voltage_out Array to store the returned voltages.
 *
 * @param[in] voltage_out_len Length of the voltages_out array. Must be
 * equal to the number of analog inputs.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetAllAnalogInput(Bs120xEnetHandle handle,
                                            float voltage_out[],
                                            size_t voltage_out_len);

/**
 * @brief Gets the output state of a DIO.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] dio The DIO to target. DIO are 0-indexed.
 *
 * @param[out] state_out Pointer to the returned state.
 *
 * @return 0 or a negative error code.
 */
BS120X_API int Bs120xEnet_GetDIOState(Bs120xEnetHandle handle, unsigned int dio,
                                      bool* state_out);

/**
 * @brief Gets the output states of all DIO.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] state_out Array to store the returned states.
 *
 * @param[in] state_out_len Length of the state_out array. Must be
 * equal to the number of DIO.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetAllDIOState(Bs120xEnetHandle handle,
                                         bool state_out[],
                                         size_t state_out_len);

/** @} */

/**
 * @defgroup CSystem System
 * Functions related to system status and control.
 * @{
 */

/**
 * @brief Gets the status of the unit.
 *
 * @note The BS120x unit must have UDP data broadcast enabled for this method to
 * return valid data.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] status_out Pointer to the returned status.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetStatus(Bs120xEnetHandle handle,
                                    struct Bs120xStatus* status_out);
/** @} */

/**
 * @defgroup CConfiguration Configuration
 * Functions for setting and reading unit configuration information.
 * @{
 */

/**
 * @brief Gets the configuration values of a unit after a QueryConfig call.
 *
 * @note QueryConfig must be called prior to calling this method.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[out] unit_config_out Pointer to the returned configuration.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetUnitConfig(
    Bs120xEnetHandle handle, struct Bs120xUnitConfig* unit_config_out);
/** @} */

/**
 * @addtogroup CSystem
 * @{
 */

/**
 * @brief Commands the unit to reset.
 *
 * @note Disconnect and Connect must be called after this method to establish a
 * new connection.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @return 0 on success or a negative error code.
 */

BS120X_API int Bs120xEnet_Reset(Bs120xEnetHandle handle);

/** @} */

/**
 * @addtogroup CConfiguration
 * @{
 */
/**
 * @brief Sets the unit IP address. Unit must be reset for changes to take
 * effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] ip New IP address for the unit. Last octet may not have a
 * value of 1 or 255.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetIPAddress(Bs120xEnetHandle handle, const char* ip);

/**
 * @brief Sends a request to the unit for its configuration information.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_QueryConfig(Bs120xEnetHandle handle);

/**
 * @brief Sets an integer configuration option. Unit must be reset for changes
 * to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to set.
 *
 * @param[in] value The value to set the configuration option to.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetConfigOptionInt(Bs120xEnetHandle handle,
                                             const char* opt, int value);

/**
 * @brief Sets a boolean configuration option. Unit must be reset for changes to
 * take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to set.
 *
 * @param[in] value The value to set the configuration option to.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetConfigOptionBool(Bs120xEnetHandle handle,
                                              const char* opt, bool value);

/**
 * @brief Sets a string configuration option. Unit must be reset for changes to
 * take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to set.
 *
 * @param[in] value The value to set the configuration option to.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetConfigOptionString(Bs120xEnetHandle handle,
                                                const char* opt,
                                                const char* value);

/**
 * @brief Sets a floating-point configuration option. Unit must be reset for
 * changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to set.
 *
 * @param[in] value The value to set the configuration option to.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetConfigOptionFloat(Bs120xEnetHandle handle,
                                               const char* opt, double value);

/**
 * @brief Enables the UDP data broadcast for the unit. Unit must be reset for
 * changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] enable A value of TRUE will enable the UDP data broadcast.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_EnableUDPData(Bs120xEnetHandle handle, bool enable);

/**
 * @brief Sets the port for the unit's UDP data broadcast. Unit must be reset
 * for changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] port The new UDP port.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetUDPDataPort(Bs120xEnetHandle handle,
                                         uint16_t port);

/**
 * @brief Sets the period for the unit's UDP data broadcast. Unit must be
 * reset for changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] period The new period in milliseconds.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetUDPDataPeriod(Bs120xEnetHandle handle,
                                           uint32_t period);

/**
 * @brief Sets the unit's box ID. Unit must be reset for changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] id The new box ID. Valid values are 0-15.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetBoxId(Bs120xEnetHandle handle, uint8_t id);

/**
 * @brief Sets the period for the unit's CAN broadcast. Unit must be reset for
 * changes to take effect.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] period The new period in milliseconds.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_SetCANPeriod(Bs120xEnetHandle handle,
                                       uint32_t period);

/**
 * @brief Enables or disables the cell inhibit lines.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] enable The enable state for the cell inhibit lines.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_EnableCellInhibit(Bs120xEnetHandle handle,
                                            bool enable);

/**
 * @brief Get a boolean configuration value.
 *
 * @note QueryConfig must be called prior to calling this method.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to look up.
 *
 * @param[out] value_out Pointer to the returned configuration value.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetConfigValueBool(Bs120xEnetHandle handle,
                                             const char* opt, bool* value_out);

/**
 * @brief Get an integer configuration value.
 *
 * @note QueryConfig must be called prior to calling this method.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to look up.
 *
 * @param[out] value_out Pointer to the returned configuration value.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetConfigValueInt(Bs120xEnetHandle handle,
                                            const char* opt, int* value_out);

/**
 * @brief Get a floating-point configuration value.
 *
 * @note QueryConfig must be called prior to calling this method.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to look up.
 *
 * @param[out] value_out Pointer to the returned configuration value.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetConfigValueFloat(Bs120xEnetHandle handle,
                                              const char* opt,
                                              float* value_out);

/**
 * @brief Get a string configuration value.
 *
 * @note QueryConfig must be called prior to calling this method.
 *
 * @param[in] handle Ethernet client handle.
 *
 * @param[in] opt The configuration option to look up.
 *
 * @param[out] buf Buffer to hold the returned configuration value.
 *
 * @param[in] len Length of the buffer.
 *
 * @return 0 on success or a negative error code.
 */
BS120X_API int Bs120xEnet_GetConfigValueString(Bs120xEnetHandle handle,
                                               const char* opt, char buf[],
                                               size_t len);
/** @} */

/** @} */
#ifdef __cplusplus
} /* extern "C" */
#endif /* __cplusplus */

#endif /* BS120X_INCLUDE_BCI_BS120X_CINTERFACE_H */

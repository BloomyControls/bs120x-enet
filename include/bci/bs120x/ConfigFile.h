/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#ifndef BS120X_INCLUDE_BCI_BS120X_CONFIGFILE_H
#define BS120X_INCLUDE_BCI_BS120X_CONFIGFILE_H

#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <variant>

#include "CommonTypes.h"
#include "bs120xenet_export.h"

namespace bci::bs120x {

/**
 * @brief Class for reading configuration information sent by the BS120x
 * and accessing configuration values.
 *
 * This class is not exposed directly to the user.
 */
class ConfigFile {
 public:
  using ConfigValue = std::variant<bool, std::string, int, float>;

  /**
   * @brief Default constructor.
   */
  BS120X_API ConfigFile();

  /**
   * @brief Parses and stores configuration information sent by the BS120x.
   *
   * @param[in] buf Configuration frame buffer.
   */
  BS120X_API void Load(std::span<const std::uint8_t> buf);

  /**
   * @brief Packages configuration information into UnitConfig.
   *
   * @return UnitConfig structure containing standard configuration information.
   */
  BS120X_API UnitConfig ToUnitConfig() const;

  /**
   * @brief Retrieves a boolean configuration value.
   *
   * @param[in] key The configuration option to look up.
   *
   * @return The configuration option value.
   */
  BS120X_API Result<bool> GetValBool(const std::string& key) const;

  /**
   * @brief Retrieves an integer configuration value.
   *
   * @param[in] key The configuration option to look up.
   *
   * @return The configuration option value.
   */
  BS120X_API Result<int> GetValInt(const std::string& key) const;

  /**
   * @brief Retrieves a floating-point configuration value.
   *
   * @param[in] key The configuration option to look up.
   *
   * @return The configuration option value.
   */
  BS120X_API Result<float> GetValFloat(const std::string& key) const;

  /**
   * @brief Retrieves a string configuration value.
   *
   * @param[in] key The configuration value to look up.
   *
   * @return The configuration option value.
   */
  BS120X_API Result<std::string> GetValString(const std::string& key) const;

  /**
   * @brief Sets a boolean configuration option in the local copy of
   * configuration data.
   *
   * @note This does not set configuration values on the BS120x!
   *
   * @param[in] key The configuration option to set.
   *
   * @param[in] The configuration option value to set.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetVal(const std::string& key, bool val);

  /**
   * @brief Sets an integer configuration option in the local copy of
   * configuration data.
   *
   * @note This does not set configuration values on the BS120x!
   *
   * @param[in] key The configuration option to set.
   *
   * @param[in] The configuration option value to set.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetVal(const std::string& key, int val);

  /**
   * @brief Sets a floating-point configuration option in the local copy of
   * configuration data.
   *
   * @note This does not set configuration values on the BS120x!
   *
   * @param[in] key The configuration option to set.
   *
   * @param[in] The configuration option value to set.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetVal(const std::string& key, float val);

  /**
   * @brief Sets a string configuration option in the local copy of
   * configuration data.
   *
   * @note This does not set configuration values on the BS120x!
   *
   * @param[in] key The configuration option to set.
   *
   * @param[in] The configuration option value to set.
   *
   * @return An error code.
   */
  BS120X_API ErrorCode SetVal(const std::string& key, std::string val);

  /**
   * @brief Clears the local copy of configuration data.
   */
  BS120X_API void Clear() noexcept;

  /**
   * @brief Checks whether there is a local copy of configuration data
   * stored.
   *
   * @return TRUE indicates that no configuration data has been retrieved.
   */
  BS120X_API bool IsEmpty() const noexcept;

 private:
  std::map<std::string, ConfigValue> config_;
};

}  // namespace bci::bs120x

#endif

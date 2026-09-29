/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 *
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/**
 * @file
 * @brief Device discovery functionality.
 */
#ifndef BS120X_ENET_INCLUDE_BCI_BS120X_DISCOVERY_H
#define BS120X_ENET_INCLUDE_BCI_BS120X_DISCOVERY_H

#include <string_view>
#include <vector>

#include "CommonTypes.h"
#include "bs120xenet_export.h"

namespace bci::bs120x {

/**
 * @brief Discovers devices over a period of 2 seconds on the specified network
 * interface.
 *
 * @param[in] interface_ip interface address
 *
 * @return Result containing a list of `UnitConfig`s for each discovered device
 * or an error code.
 */
BS120X_API Result<std::vector<UnitConfig>> DiscoverDevices(
    std::string_view interface_ip);

}  // namespace bci::bs120x

#endif  /* BS120X_ENET_INCLUDE_BCI_BS120X_DISCOVERY_H */

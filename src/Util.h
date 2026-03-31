/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/* General utilities. */

#ifndef BS120X_SRC_UTIL_H
#define BS120X_SRC_UTIL_H

#include <bci/bs120x/EthernetClient.h>

#include <string_view>

namespace bci::bs120x::util {

/* Convert a numeric error code to the 'unexpected' value of a Result. */
template <class T>
inline constexpr auto Err(T&& val) {
  return tl::unexpected(std::forward<T>(val));
}

/* Check whether an IP address is valid. */
bool IsValidIpAddress(std::string_view ip);

}  // namespace bci::bs120x::util

#endif /* BS120X_SRC_UTIL_H */

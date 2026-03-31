/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

/*
 * Utilties for string manipulation used to parse BS120x configuration
 * information.
 */

#ifndef BS120X_SRC_STRINGUTIL_H
#define BS120X_SRC_STRINGUTIL_H

#include <string_view>

namespace bci::bs120x::util {

inline constexpr std::string_view kTrimChars{" \t\v\r\n\0"};

inline constexpr std::string_view TrimLeft(std::string_view str) noexcept {
  if (str.size() == 0) {
    return str;
  }

  const auto pos = str.find_first_not_of(kTrimChars);
  str.remove_prefix((std::min)(pos, str.size()));

  return str;
}

inline constexpr std::string_view TrimRight(std::string_view str) noexcept {
  if (str.size() == 0) {
    return str;
  }

  const auto pos = str.find_last_not_of(kTrimChars);
  if (pos != str.npos) {
    str.remove_suffix(str.size() - pos - 1);
  }

  return str;
}

inline constexpr std::string_view Trim(std::string_view str) noexcept {
  return TrimRight(TrimLeft(str));
}

}  // namespace bci::bs120x::util

#endif

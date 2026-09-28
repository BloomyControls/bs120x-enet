/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include "Util.h"

#include <array>
#include <charconv>
#include <cstdint>
#include <ranges>
#include <string_view>

namespace bci::bs120x::util {

bool IsValidIpAddress(std::string_view ip) {
  if (ip.length() > 15 || ip.length() < 7) {
    return false;
  }

  std::array<std::uint8_t, 4> octets{};

  for (int i{}; const auto val : std::views::lazy_split(ip, '.')) {
    if (i >= 4) {
      return false;
    }

    std::string_view part(&*val.begin(), std::ranges::distance(val));
    if (part.empty()) {
      return false;
    }

    auto [ptr, ec] =
        std::from_chars(part.data(), part.data() + part.size(), octets[i++]);
    if (ec != std::errc{} || ptr != (part.data() + part.size())) {
      return false;
    }
  }

  if (octets.back() <= 1 || octets.back() == 255) {
    return false;
  }

  return true;
}

}  // namespace bci::bs120x::util

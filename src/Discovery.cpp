/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 *
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include <bci/bs120x/Discovery.h>

#include <chrono>
#include <memory>
#include <thread>

#include <bci/bs120x/ConfigFile.h>

#include "UdpListener.h"
#include "Util.h"

namespace bci::bs120x {

using ec = ErrorCode;
using util::Err;

constexpr auto kMaxDuration{std::chrono::seconds(2)};

Result<std::vector<UnitConfig>> DiscoverDevices(
    std::string_view interface_ip) {
  std::vector<UnitConfig> devices;

  const auto rxc = [&](std::span<const std::uint8_t> data) {
    ConfigFile file;
    file.Load(data);
    if (file.IsEmpty()) {
      return;
    }
    auto uc = file.ToUnitConfig();
    for (auto&& d : devices) {
      if (d.ip_address == uc.ip_address) {
        return;
      }
    }
    devices.push_back(std::move(uc));
  };

  ec status{ec::kSuccess};
  const auto ecb = [&](ErrorCode e) { status = e; };

  auto listener = std::make_unique<net::UdpListener>();

  listener->SetDataReceivedCallback(rxc);
  listener->SetErrorCallback(ecb);

  // first setup handlers for responses and errors...
  // in this case, we won't be using the timeout feature, we will just set it to
  // be pretty long
  ec ret = listener->Connect(interface_ip, 58431, 5000);
  if (ret != ec::kSuccess) {
    return Err(ret);
  }

  const auto now = [] { return std::chrono::steady_clock::now(); };

  const auto start = now();
  while (status == ec::kSuccess && (now() - start) < kMaxDuration) {
    std::this_thread::yield();
  }

  listener->Disconnect();

  if (status != ec::kSuccess) {
    return Err(status);
  }

  return devices;
}

}  // namespace bci::bs120x

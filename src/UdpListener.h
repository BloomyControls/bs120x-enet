/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#ifndef BS120X_SRC_UDPLISTENER_H
#define BS120X_SRC_UDPLISTENER_H

#include <bci/bs120x/EthernetClient.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <cstdint>
#include <functional>
#include <span>
#include <thread>

namespace bci::bs120x {

/* Class to handle UDP connection to BS120x and receive incoming messages. */
class EthernetClient::UdpListener {
 public:
  using DataReceivedCallbackFn =
      std::function<void(std::span<const std::uint8_t>)>;
  using ErrorCallbackFn = std::function<void(ErrorCode)>;

  UdpListener();

  ~UdpListener() noexcept;

  ErrorCode Connect(std::string_view interface_ip, std::uint16_t port,
                    std::int32_t timeout) noexcept;

  void Disconnect() noexcept;

  void SetDataReceivedCallback(DataReceivedCallbackFn callback) noexcept;

  void SetErrorCallback(ErrorCallbackFn callback) noexcept;

 private:
  boost::asio::io_context io_context_;
  boost::asio::ip::udp::socket sock_;
  boost::asio::steady_timer timer_;
  std::array<std::uint8_t, 8192> buf_;
  std::int32_t timeout_;
  std::thread io_worker_;

  DataReceivedCallbackFn data_handler_;
  ErrorCallbackFn error_handler_;

  void StartReceive() noexcept;
};

}  // namespace bci::bs120x

#endif /* BS120X_SRC_UDPLISTENER_H */

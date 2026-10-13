/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#ifndef BS120X_SRC_TCPCLIENT_H
#define BS120X_SRC_TCPCLIENT_H

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <cstdint>
#include <functional>
#include <span>

#include <bci/bs120x/CommonTypes.h>

namespace bci::bs120x::net {

/* Class to handle TCP connection to BS120x and send commands. */
class TcpClient {
 public:
  using ErrorCallbackFn = std::function<void(ErrorCode)>;

  TcpClient();

  ~TcpClient() noexcept;

  ErrorCode Connect(std::string_view device_ip, std::uint16_t port) noexcept;

  void Disconnect() noexcept;

  void SetErrorCallback(ErrorCallbackFn callback) noexcept;

  ErrorCode SendData(std::span<const std::uint8_t> data) noexcept;

 private:
  boost::asio::io_context io_context_;
  boost::asio::ip::tcp::socket sock_;

  std::array<std::uint8_t, 1024> buf_;

  ErrorCallbackFn error_handler_;
};

}  // namespace bci::bs120x::net

#endif /* BS120X_SRC_TCPCLIENT_H */

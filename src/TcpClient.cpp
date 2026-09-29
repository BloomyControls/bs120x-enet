/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include "TcpClient.h"

#include <algorithm>
#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/error_code.hpp>
#include <cstdint>
#include <span>

namespace asio = boost::asio;
namespace bs = boost::system;

#include <bci/bs120x/CommonTypes.h>

#include "Util.h"

namespace bci::bs120x::net {

TcpClient::TcpClient()
    : io_context_{}, sock_{io_context_}, buf_{}, error_handler_{} {}

TcpClient::~TcpClient() noexcept { Disconnect(); }

ErrorCode TcpClient::Connect(std::string_view device_ip,
                             std::uint16_t port) noexcept {
  if (sock_.is_open()) {
    return ErrorCode::kTcpAlreadyConnected;
  }

  bs::error_code ec{};

  auto address = asio::ip::make_address(device_ip, ec);
  if (ec) {
    return ErrorCode::kInvalidIPAddress;
  }

  auto endpoint = asio::ip::tcp::endpoint(address, port);

  // would_block is never set from async_functions, so it's a safe way to signal
  // an incomplete async operation
  ec = asio::error::would_block;
  const auto connect_handler = [&](auto&& e) { ec = e; };
  sock_.async_connect(endpoint, connect_handler);

  // block until connect completes
  io_context_.restart();
  do {
    io_context_.run_one();
  } while (ec == asio::error::would_block);

  if (ec) {
    return ErrorCode::kTcpConnectFailed;
  }

  // set TCP keepalive options to allow us to detect a disconnection and respond
  sock_.set_option(asio::socket_base::keep_alive(true), ec);
  sock_.set_option(
      asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPIDLE>(1), ec);
  sock_.set_option(
      asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPINTVL>(1), ec);
  sock_.set_option(
      asio::detail::socket_option::integer<IPPROTO_TCP, TCP_KEEPCNT>(5), ec);
  sock_.set_option(asio::socket_base::linger(false, 0), ec);

  return ErrorCode::kSuccess;
}

void TcpClient::Disconnect() noexcept {
  bs::error_code ignored;
  if (sock_.is_open()) {
    sock_.cancel(ignored);
    sock_.shutdown(asio::ip::tcp::socket::shutdown_both, ignored);
    sock_.close(ignored);
    io_context_.stop();
  }
}

void TcpClient::SetErrorCallback(ErrorCallbackFn callback) noexcept {
  error_handler_ = std::move(callback);
}

ErrorCode TcpClient::SendData(std::span<const std::uint8_t> data) noexcept {
  if (!sock_.is_open()) {
    return ErrorCode::kNotConnected;
  }

  std::size_t data_len = data.size();

  if (data_len > buf_.size()) {
    return ErrorCode::kBufferTooSmall;
  }

  std::ranges::copy(data, buf_.begin());

  bs::error_code ec = asio::error::would_block;
  const auto write_handler = [&](auto&& e, auto&&) { ec = e; };

  asio::async_write(sock_, asio::buffer(buf_, data_len), write_handler);

  // would_block is never set from async functions.
  // This will block until the send is complete.
  do {
    io_context_.run_one();
  } while (ec == asio::error::would_block);

  if (ec) {
    if (error_handler_) {
      error_handler_(ErrorCode::kSendFailed);
    }
    return ErrorCode::kSendFailed;
  }

  return ErrorCode::kSuccess;
}

}  // namespace bci::bs120x::net

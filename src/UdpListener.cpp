/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include "UdpListener.h"

#include <boost/asio/buffer.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/system_timer.hpp>
#include <boost/system/error_code.hpp>
#include <chrono>
#include <cstdint>
#include <span>

namespace asio = boost::asio;
namespace bs = boost::system;

#include <bci/bs120x/CommonTypes.h>

namespace bci::bs120x::net {

UdpListener::UdpListener()
    : io_context_{},
      sock_{io_context_},
      timer_{io_context_},
      buf_{},
      timeout_{1000},
      io_worker_{},
      data_handler_{},
      error_handler_{} {}

UdpListener::~UdpListener() noexcept { Disconnect(); }

ErrorCode UdpListener::Connect(std::string_view interface_ip,
                               std::uint16_t port,
                               std::int32_t timeout) noexcept {
  if (sock_.is_open()) {
    return ErrorCode::kUdpAlreadyConnected;
  }

  bs::error_code ec{};

  auto address = asio::ip::make_address(interface_ip, ec);
  if (ec) {
    return ErrorCode::kInvalidIPAddress;
  }

  auto endpoint = asio::ip::udp::endpoint(address, port);

  sock_.open(endpoint.protocol(), ec);
  if (ec) {
    return ErrorCode::kUdpSocketOpenFailed;
  }

  // this is big enough for a worst case full UDP frame (not likely to ever
  // happen....)
  asio::socket_base::receive_buffer_size rx_buf_opt{1024 * 64};
  sock_.set_option(rx_buf_opt, ec);
  if (ec) {
    Disconnect();
    return ErrorCode::kUdpSocketConfigFailed;
  }

  sock_.bind(endpoint, ec);
  if (ec) {
    Disconnect();
    return ErrorCode::kUdpSocketBindFailed;
  }

  timeout_ = timeout;
  StartReceive();

  // this might throw, but if creating a thread fails, that's irrecoverable and
  // we're okay with termination
  io_context_.restart();
  io_worker_ = std::thread([this] { io_context_.run(); });

  return ErrorCode::kSuccess;
}

void UdpListener::Disconnect() noexcept {
  bs::error_code ignored;
  if (sock_.is_open()) {
    sock_.cancel(ignored);
    sock_.shutdown(asio::ip::udp::socket::shutdown_both, ignored);
    sock_.close(ignored);
    timer_.cancel();
    io_context_.stop();
    if (io_worker_.joinable()) {
      io_worker_.join();
    }
  }
}

void UdpListener::SetDataReceivedCallback(
    DataReceivedCallbackFn callback) noexcept {
  data_handler_ = std::move(callback);
}

void UdpListener::SetErrorCallback(ErrorCallbackFn callback) noexcept {
  error_handler_ = std::move(callback);
}

void UdpListener::StartReceive() noexcept {
  const auto callback = [this](bs::error_code ec, std::size_t len) {
    if (!ec) {
      if (data_handler_ && len > 0) {
        data_handler_(std::span{buf_.data(), len});
      }
      StartReceive();
    } else if (ec != asio::error::operation_aborted) {
      if (error_handler_) {
        // we won't disconnect here as a formality; just leave it up to the
        // receiver whether they want to disconnect or not
        error_handler_(ErrorCode::kReadFailed);
      }
    }
  };

  const auto timeout_handler = [this](const bs::error_code& ec) {
    if (!ec) {
      if (error_handler_) {
        error_handler_(ErrorCode::kReadbackTimedOut);
      }
    }
  };

  // if the timeout is less than 0, set to the maximum time point
  if (timeout_ < 0) {
    timer_.expires_at(std::chrono::steady_clock::time_point::max());
  } else {
    timer_.expires_after(std::chrono::milliseconds(timeout_));
  }

  timer_.async_wait(timeout_handler);
  sock_.async_receive(asio::buffer(buf_), callback);
}

}  // namespace bci::bs120x::net

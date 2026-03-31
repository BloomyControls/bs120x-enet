/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#include "Frame.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

#include "Util.h"

namespace bci::bs120x {

using util::Err;

Frame::Frame() : data_length_{}, raw_frame_{} {}

Result<Frame> Frame::Parse(std::span<const std::uint8_t> buf) {
  if (buf.size() < 10) {
    return Err(ErrorCode::kBufferTooSmall);
  }

  Frame f;
  f.data_length_ = (buf[6] << 24) | (buf[7] << 16) | (buf[8] << 8) | buf[9];

  if (f.data_length_ > 8 || (f.data_length_ + 10) > buf.size()) {
    return Err(ErrorCode::kInvalidFrameSize);
  }

  std::copy_n(buf.begin(), f.data_length_ + 10, f.raw_frame_.begin());

  return f;
}

Result<Frame> Frame::Pack(ArbId arb_id, std::uint8_t box_id, bool ext_frame,
                          bool is_data_frame,
                          std::span<const std::uint8_t> data) {
  if (data.size() < 1 || data.size() > 8) {
    return Err(ErrorCode::kInvalidFrameSize);
  }

  Frame f;

  f.data_length_ = static_cast<std::uint32_t>(data.size());

  auto id = static_cast<std::uint32_t>(arb_id);

  f.raw_frame_[0] = (id & 0xff000000) >> 24;
  f.raw_frame_[1] = (id & 0x00ff0000) >> 16;
  f.raw_frame_[2] = (id & 0x0000ff00) >> 8;
  f.raw_frame_[3] = (id & 0x000000f0) | (box_id & 0x0f);

  f.raw_frame_[4] = static_cast<std::uint8_t>(ext_frame);

  f.raw_frame_[5] = static_cast<std::uint8_t>(!is_data_frame);

  f.raw_frame_[6] =
      static_cast<std::uint8_t>((f.data_length_ & 0xff000000) >> 24);
  f.raw_frame_[7] =
      static_cast<std::uint8_t>((f.data_length_ & 0x00ff0000) >> 16);
  f.raw_frame_[8] =
      static_cast<std::uint8_t>((f.data_length_ & 0x0000ff00) >> 8);
  f.raw_frame_[9] = static_cast<std::uint8_t>(f.data_length_ & 0x000000ff);

  std::copy_n(data.begin(), f.data_length_, f.raw_frame_.begin() + 10);

  return f;
}

ArbId Frame::GetArbId() const noexcept {
  return static_cast<ArbId>((raw_frame_[0] << 24) | (raw_frame_[1] << 16) |
                            (raw_frame_[2] << 8) | (raw_frame_[3] & 0xf0));
}

std::uint8_t Frame::GetBoxId() const noexcept { return raw_frame_[3] & 0x0f; }

std::uint32_t Frame::GetDataLength() const noexcept { return data_length_; }

std::span<const std::uint8_t> Frame::GetData() const noexcept {
  return std::span<const std::uint8_t>(raw_frame_).subspan(10, data_length_);
}

std::span<const std::uint8_t> Frame::GetRawFrame() const noexcept {
  return raw_frame_;
}

}  // namespace bci::bs120x

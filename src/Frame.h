/*
 * Copyright © 2026, Bloomy Controls, Inc. All rights reserved.
 * Use of this source code is governed by a BSD-3-clause license that can be
 * found in the LICENSE file or at https://opensource.org/license/BSD-3-Clause
 */

#ifndef BS120X_SRC_FRAMES_H
#define BS120X_SRC_FRAMES_H

#include <bci/bs120x/CommonTypes.h>

#include <array>
#include <cstdint>
#include <span>

namespace bci::bs120x {

/* Valid arbitration IDs for BS120x frames. */
enum class ArbId : std::uint32_t {
  kHILModeEn = 0x00000080,
  kVSet1_4 = 0x000000A0,
  kVSet5_8 = 0x000000B0,
  kVSet9_12 = 0x000000C0,
  kStatusReadBack = 0x00000100,
  kVReadBack1_4 = 0x00000120,
  kVReadBack5_8 = 0x00000130,
  kVReadBack9_12 = 0x00000140,
  kIReadBack1_4 = 0x00000180,
  kIReadBack5_8 = 0x00000190,
  kIReadBack9_12 = 0x000001A0,
  kSetDIOAll = 0x00000200,
  kSetAOAll = 0x00000220,
  kDIOReadBack1_8 = 0x00000280,
  kAIReadBack1_4 = 0x000002A0,
  kAIReadBack5_8 = 0x000002B0,
  kBoxModeMsgCfg = 0x00000400,
  kOpModeCfg = 0x00000410,
  kISetAll = 0x00000480,
  kISinkSetSgl = 0x000004A0,
  kISrcSetSgl = 0x000004B0,
  kVSetAll = 0x00000500,
  kVSetSgl = 0x00000510,
  kCellEnAll = 0x00000540,
  kCellEnSgl = 0x00000550
};

/* Class to handle constructing frames or parsing frames from a byte buffer.
 * BS120x frames have a CAN frame-like structure.
 */
class Frame {
  Frame();

 public:
  [[nodiscard]]
  static Result<Frame> Parse(std::span<const std::uint8_t> buf);

  [[nodiscard]]
  static Result<Frame> Pack(ArbId arb_id, std::uint8_t box_id, bool ext_frame,
                            bool is_data_frame,
                            std::span<const std::uint8_t> data);

  ArbId GetArbId() const noexcept;

  std::uint8_t GetBoxId() const noexcept;

  std::uint32_t GetDataLength() const noexcept;

  std::span<const std::uint8_t> GetData() const noexcept;

  std::span<const std::uint8_t> GetRawFrame() const noexcept;

 private:
  std::uint32_t data_length_;
  std::array<std::uint8_t, 18> raw_frame_;
};

}  // namespace bci::bs120x

#endif

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

namespace stackchan::hal::board {

constexpr const char* kStackChanSdMountPoint = "/sdcard";

bool ensure_stackchan_sdcard_mounted();

}  // namespace stackchan::hal::board

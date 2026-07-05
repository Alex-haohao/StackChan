/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "../../avatar/elements/emotion.h"
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace stackchan::avatar::full_sprite {

constexpr int kCanvasWidth              = 320;
constexpr int kCanvasHeight             = 240;
constexpr int kOpenMouthWeightThreshold = 40;
constexpr int kClosedEyeWeightThreshold = 30;
constexpr int kDefaultFrameDurationMs   = 160;

struct FrameGroup {
    std::string id;
    std::vector<std::string> frames;
    std::vector<int> durations_ms;
};

struct SpritePack {
    std::string root;
    std::map<std::string, FrameGroup> frame_groups;
    std::map<Emotion, std::string> emotion_groups;
    std::string idle_group  = "idle";
    std::string blink_group = "blink";
    std::string talk_group  = "talk";
};

struct RuntimeState {
    Emotion emotion       = Emotion::Neutral;
    int left_eye_weight   = 100;
    int right_eye_weight  = 100;
    int mouth_weight      = 0;
};

std::string join_pack_path(const std::string& root, const std::string& relative_path);
const FrameGroup* find_group(const SpritePack& pack, const std::string& group_id);
const FrameGroup* select_group(const SpritePack& pack, const RuntimeState& state);
std::size_t frame_index_for_elapsed(const FrameGroup& group, std::uint32_t elapsed_ms);
std::string frame_path_for_elapsed(const SpritePack& pack, const FrameGroup& group, std::uint32_t elapsed_ms);

}  // namespace stackchan::avatar::full_sprite

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "full_sprite_pack.h"

namespace stackchan::avatar::full_sprite {

std::string join_pack_path(const std::string& root, const std::string& relative_path)
{
    if (root.empty()) {
        return relative_path;
    }
    if (relative_path.empty()) {
        return root;
    }
    if (relative_path.front() == '/') {
        return relative_path;
    }
    if (root.back() == '/') {
        return root + relative_path;
    }
    return root + "/" + relative_path;
}

const FrameGroup* find_group(const SpritePack& pack, const std::string& group_id)
{
    auto it = pack.frame_groups.find(group_id);
    if (it == pack.frame_groups.end() || it->second.frames.empty()) {
        return nullptr;
    }
    return &it->second;
}

static bool is_blinking(const RuntimeState& state)
{
    return state.left_eye_weight <= kClosedEyeWeightThreshold &&
           state.right_eye_weight <= kClosedEyeWeightThreshold;
}

const FrameGroup* select_group(const SpritePack& pack, const RuntimeState& state)
{
    if (is_blinking(state)) {
        if (const FrameGroup* group = find_group(pack, pack.blink_group)) {
            return group;
        }
    }

    if (state.mouth_weight >= kOpenMouthWeightThreshold) {
        if (const FrameGroup* group = find_group(pack, pack.talk_group)) {
            return group;
        }
    }

    auto emotion_group = pack.emotion_groups.find(state.emotion);
    if (emotion_group != pack.emotion_groups.end()) {
        if (const FrameGroup* group = find_group(pack, emotion_group->second)) {
            return group;
        }
    }

    if (const FrameGroup* group = find_group(pack, pack.idle_group)) {
        return group;
    }

    for (const auto& [id, group] : pack.frame_groups) {
        if (!group.frames.empty()) {
            return &group;
        }
    }
    return nullptr;
}

static int duration_at(const FrameGroup& group, std::size_t index)
{
    if (index < group.durations_ms.size() && group.durations_ms[index] > 0) {
        return group.durations_ms[index];
    }
    return kDefaultFrameDurationMs;
}

std::size_t frame_index_for_elapsed(const FrameGroup& group, std::uint32_t elapsed_ms)
{
    if (group.frames.empty()) {
        return 0;
    }

    std::uint32_t total_duration = 0;
    for (std::size_t i = 0; i < group.frames.size(); ++i) {
        total_duration += static_cast<std::uint32_t>(duration_at(group, i));
    }
    if (total_duration == 0) {
        return 0;
    }

    std::uint32_t loop_time = elapsed_ms % total_duration;
    for (std::size_t i = 0; i < group.frames.size(); ++i) {
        const auto duration = static_cast<std::uint32_t>(duration_at(group, i));
        if (loop_time < duration) {
            return i;
        }
        loop_time -= duration;
    }
    return group.frames.size() - 1;
}

std::string frame_path_for_elapsed(const SpritePack& pack, const FrameGroup& group, std::uint32_t elapsed_ms)
{
    if (group.frames.empty()) {
        return "";
    }
    return join_pack_path(pack.root, group.frames[frame_index_for_elapsed(group, elapsed_ms)]);
}

}  // namespace stackchan::avatar::full_sprite

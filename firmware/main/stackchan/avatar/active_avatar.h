/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <lvgl.h>
#include <memory>
#include <smooth_lvgl.hpp>
#include <stackchan/avatar/avatar/avatar.h>

namespace stackchan::avatar {

struct ActiveAvatar {
    std::unique_ptr<Avatar> avatar;
    uitk::lvgl_cpp::Container* panel = nullptr;
};

ActiveAvatar create_active_avatar(lv_obj_t* parent, const lv_font_t* font = &lv_font_montserrat_16);

}  // namespace stackchan::avatar

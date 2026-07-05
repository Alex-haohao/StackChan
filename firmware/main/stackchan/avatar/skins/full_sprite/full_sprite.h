/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "full_sprite_pack.h"
#include "../../avatar/avatar.h"
#include <lvgl.h>
#include <memory>
#include <smooth_lvgl.hpp>

namespace stackchan::avatar {

class FullSpriteAvatar : public Avatar {
public:
    bool init(lv_obj_t* parent, const lv_font_t* font = &lv_font_montserrat_16,
              const char* pack_root = "/sdcard/stackchan/avatar/active");
    void update() override;

    uitk::lvgl_cpp::Container* getPanel() const;

private:
    bool setFrameForState(std::uint32_t now_ms);

    full_sprite::SpritePack _pack;
    std::unique_ptr<uitk::lvgl_cpp::Container> _panel;
    std::unique_ptr<uitk::lvgl_cpp::Image> _frame;
    std::string _current_frame_path;
    std::string _current_group_id;
    std::uint32_t _current_group_started_ms = 0;
};

std::unique_ptr<FullSpriteAvatar> create_full_sprite_avatar(lv_obj_t* parent, const lv_font_t* font,
                                                            const char* pack_root);

}  // namespace stackchan::avatar

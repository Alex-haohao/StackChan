/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "active_avatar.h"
#include "skins/full_sprite/full_sprite.h"
#include "skins/default/default.h"
#include <esp_log.h>
#include <hal/board/stackchan_sdcard.h>

namespace stackchan::avatar {
namespace {

constexpr const char* kTag = "ActiveAvatar";

}

ActiveAvatar create_active_avatar(lv_obj_t* parent, const lv_font_t* font)
{
    ESP_LOGI(kTag, "creating active avatar");

    if (hal::board::ensure_stackchan_sdcard_mounted()) {
        ESP_LOGI(kTag, "microSD available, trying full-sprite avatar pack");
        auto full_sprite = create_full_sprite_avatar(parent, font, "/sdcard/stackchan/avatar/active");
        if (full_sprite) {
            ESP_LOGI(kTag, "using full-sprite avatar");
            ActiveAvatar active_avatar;
            active_avatar.panel  = full_sprite->getPanel();
            active_avatar.avatar = std::move(full_sprite);
            return active_avatar;
        }

        ESP_LOGW(kTag, "full-sprite avatar pack failed, falling back to default avatar");
    } else {
        ESP_LOGW(kTag, "microSD avatar pack unavailable, falling back to default avatar");
    }

    auto avatar = std::make_unique<DefaultAvatar>();
    avatar->init(parent, font);
    ActiveAvatar active_avatar;
    active_avatar.panel  = avatar->getPanel();
    active_avatar.avatar = std::move(avatar);
    return active_avatar;
}

}  // namespace stackchan::avatar

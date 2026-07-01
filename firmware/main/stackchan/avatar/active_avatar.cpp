/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "active_avatar.h"
#include "skins/default/default.h"

namespace stackchan::avatar {

ActiveAvatar create_active_avatar(lv_obj_t* parent, const lv_font_t* font)
{
    auto avatar = std::make_unique<DefaultAvatar>();
    avatar->init(parent, font);
    ActiveAvatar active_avatar;
    active_avatar.panel  = avatar->getPanel();
    active_avatar.avatar = std::move(avatar);
    return active_avatar;
}

}  // namespace stackchan::avatar

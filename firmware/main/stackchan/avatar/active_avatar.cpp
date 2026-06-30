/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "active_avatar.h"
#include <sdkconfig.h>

#if CONFIG_STACKCHAN_AVATAR_SKIN_IMAGE
#include "skins/image/image_avatar.h"
#else
#include "skins/default/default.h"
#endif

namespace stackchan::avatar {

ActiveAvatar create_active_avatar(lv_obj_t* parent, const lv_font_t* font)
{
#if CONFIG_STACKCHAN_AVATAR_SKIN_IMAGE
    auto avatar = std::make_unique<image::ImageAvatar>();
#else
    auto avatar = std::make_unique<DefaultAvatar>();
#endif
    avatar->init(parent, font);
    ActiveAvatar active_avatar;
    active_avatar.panel  = avatar->getPanel();
    active_avatar.avatar = std::move(avatar);
    return active_avatar;
}

}  // namespace stackchan::avatar

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "image_avatar_pack.h"
#include "image_sprite_feature.h"
#include <stackchan/avatar/avatar.h>
#include <smooth_lvgl.hpp>
#include <memory>

namespace stackchan::avatar::image {

class ImageAvatar : public Avatar {
public:
    lv_color_t primaryColor   = lv_color_white();
    lv_color_t secondaryColor = lv_color_black();

    void init(lv_obj_t* parent, const lv_font_t* font = &lv_font_montserrat_16);
    void init(lv_obj_t* parent, const ImageAvatarPack& pack, const lv_font_t* font);
    uitk::lvgl_cpp::Container* getPanel() const;
    void setEmotion(const Emotion& emotion) override;

private:
    void applyExpression(const ImageExpressionSet& expression);

    const ImageAvatarPack* _pack = nullptr;
    std::unique_ptr<uitk::lvgl_cpp::Container> _panel;
    std::unique_ptr<uitk::lvgl_cpp::Image> _body;
    ImageSpriteFeature* _leftEye  = nullptr;
    ImageSpriteFeature* _rightEye = nullptr;
    ImageSpriteFeature* _mouth    = nullptr;
};

}  // namespace stackchan::avatar::image

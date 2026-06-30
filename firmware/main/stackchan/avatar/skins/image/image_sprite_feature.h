/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "image_avatar_pack.h"
#include <stackchan/avatar/avatar/elements/feature.h>
#include <smooth_lvgl.hpp>
#include <memory>

namespace stackchan::avatar::image {

enum class ImageSpriteRole {
    Eye,
    Mouth,
};

class ImageSpriteFeature : public Feature {
public:
    ImageSpriteFeature(lv_obj_t* parent, ImageSpriteRole role, const ImageSpriteFrames& frames);
    ~ImageSpriteFeature();

    void setFrames(const ImageSpriteFrames& frames);
    void setPosition(const uitk::Vector2i& position) override;
    void setWeight(int weight) override;
    void setRotation(int rotation) override;
    void setVisible(bool visible) override;
    void setSize(int size) override;
    void setEmotion(const Emotion& emotion) override;

private:
    int frameIndexForWeight() const;
    void applyFrame();
    void applyPosition();
    void applyScale();

    ImageSpriteRole _role;
    const ImageSpriteFrames* _frames = nullptr;
    std::unique_ptr<uitk::lvgl_cpp::Image> _image;
};

}  // namespace stackchan::avatar::image

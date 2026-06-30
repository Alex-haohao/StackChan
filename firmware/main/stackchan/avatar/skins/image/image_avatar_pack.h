/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <lvgl.h>
#include <stackchan/avatar/avatar/elements/emotion.h>

namespace stackchan::avatar::image {

struct ImageSpriteFrames {
    const lv_image_dsc_t* const* frames;
    int frameCount;
    int centerX;
    int centerY;
    int minOffsetX;
    int maxOffsetX;
    int minOffsetY;
    int maxOffsetY;
    bool reverseWeight;
};

struct ImageExpressionSet {
    ImageSpriteFrames leftEye;
    ImageSpriteFrames rightEye;
    ImageSpriteFrames mouth;
};

struct ImageAvatarPack {
    const char* id;
    int canvasWidth;
    int canvasHeight;
    const lv_image_dsc_t* body;
    const ImageExpressionSet& (*expressionForEmotion)(Emotion emotion);
};

}  // namespace stackchan::avatar::image

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "my_stackchan_pack.h"

extern "C" {
LV_IMAGE_DECLARE(my_stackchan_body_base);

#define DECLARE_EMOTION_IMAGES(emotion)              \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_left_eye_0);  \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_left_eye_1);  \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_left_eye_2);  \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_left_eye_3);  \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_right_eye_0); \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_right_eye_1); \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_right_eye_2); \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_right_eye_3); \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_mouth_0);     \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_mouth_1);     \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_mouth_2);     \
    LV_IMAGE_DECLARE(my_stackchan_##emotion##_mouth_3);

DECLARE_EMOTION_IMAGES(neutral)
DECLARE_EMOTION_IMAGES(happy)
DECLARE_EMOTION_IMAGES(angry)
DECLARE_EMOTION_IMAGES(sad)
DECLARE_EMOTION_IMAGES(doubt)
DECLARE_EMOTION_IMAGES(sleepy)

#undef DECLARE_EMOTION_IMAGES
}

namespace stackchan::avatar::image {
namespace {

constexpr int kEyeFrameCount    = 4;
constexpr int kMouthFrameCount  = 4;
constexpr int kLeftEyeCenterX   = -20;
constexpr int kRightEyeCenterX  = 20;
constexpr int kEyeCenterY       = -36;
constexpr int kMouthCenterX     = 0;
constexpr int kMouthCenterY     = -11;
constexpr int kEyeMinOffsetX    = -12;
constexpr int kEyeMaxOffsetX    = 12;
constexpr int kEyeMinOffsetY    = -8;
constexpr int kEyeMaxOffsetY    = 8;
constexpr int kMouthMinOffsetX  = -12;
constexpr int kMouthMaxOffsetX  = 12;
constexpr int kMouthMinOffsetY  = -8;
constexpr int kMouthMaxOffsetY  = 8;

#define DEFINE_EMOTION_FRAMES(emotion)                                                                    \
    const lv_image_dsc_t* const emotion##LeftEyeFrames[] = {                                             \
        &my_stackchan_##emotion##_left_eye_0, &my_stackchan_##emotion##_left_eye_1,                       \
        &my_stackchan_##emotion##_left_eye_2, &my_stackchan_##emotion##_left_eye_3,                       \
    };                                                                                                    \
    const lv_image_dsc_t* const emotion##RightEyeFrames[] = {                                            \
        &my_stackchan_##emotion##_right_eye_0, &my_stackchan_##emotion##_right_eye_1,                     \
        &my_stackchan_##emotion##_right_eye_2, &my_stackchan_##emotion##_right_eye_3,                     \
    };                                                                                                    \
    const lv_image_dsc_t* const emotion##MouthFrames[] = {                                                \
        &my_stackchan_##emotion##_mouth_0, &my_stackchan_##emotion##_mouth_1,                             \
        &my_stackchan_##emotion##_mouth_2, &my_stackchan_##emotion##_mouth_3,                             \
    };                                                                                                    \
    const ImageExpressionSet emotion##Expression = {                                                      \
        {emotion##LeftEyeFrames, kEyeFrameCount, kLeftEyeCenterX, kEyeCenterY, kEyeMinOffsetX,            \
         kEyeMaxOffsetX, kEyeMinOffsetY, kEyeMaxOffsetY, true},                                           \
        {emotion##RightEyeFrames, kEyeFrameCount, kRightEyeCenterX, kEyeCenterY, kEyeMinOffsetX,          \
         kEyeMaxOffsetX, kEyeMinOffsetY, kEyeMaxOffsetY, true},                                           \
        {emotion##MouthFrames, kMouthFrameCount, kMouthCenterX, kMouthCenterY, kMouthMinOffsetX,          \
         kMouthMaxOffsetX, kMouthMinOffsetY, kMouthMaxOffsetY, false},                                    \
    };

DEFINE_EMOTION_FRAMES(neutral)
DEFINE_EMOTION_FRAMES(happy)
DEFINE_EMOTION_FRAMES(angry)
DEFINE_EMOTION_FRAMES(sad)
DEFINE_EMOTION_FRAMES(doubt)
DEFINE_EMOTION_FRAMES(sleepy)

#undef DEFINE_EMOTION_FRAMES

const ImageExpressionSet& expressionForEmotion(Emotion emotion)
{
    switch (emotion) {
        case Emotion::Happy:
            return happyExpression;
        case Emotion::Angry:
            return angryExpression;
        case Emotion::Sad:
            return sadExpression;
        case Emotion::Doubt:
            return doubtExpression;
        case Emotion::Sleepy:
            return sleepyExpression;
        case Emotion::Neutral:
        default:
            return neutralExpression;
    }
}

const ImageAvatarPack kMyStackChanPack = {
    "img4635-hatch-pet-stackchan",
    320,
    240,
    &my_stackchan_body_base,
    expressionForEmotion,
};

}  // namespace

const ImageAvatarPack& getMyStackChanPack()
{
    return kMyStackChanPack;
}

}  // namespace stackchan::avatar::image

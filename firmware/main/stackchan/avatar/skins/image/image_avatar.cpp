/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "image_avatar.h"
#include "packs/my_stackchan_pack.h"
#include <stackchan/avatar/skins/default/default.h>

using namespace uitk::lvgl_cpp;

namespace stackchan::avatar::image {

void ImageAvatar::init(lv_obj_t* parent, const lv_font_t* font)
{
    init(parent, getMyStackChanPack(), font);
}

void ImageAvatar::init(lv_obj_t* parent, const ImageAvatarPack& pack, const lv_font_t* font)
{
    _pack = &pack;

    _panel = std::make_unique<Container>(parent);
    _panel->align(LV_ALIGN_CENTER, 0, 0);
    _panel->setSize(pack.canvasWidth, pack.canvasHeight);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setBgColor(secondaryColor);
    _panel->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    _body = std::make_unique<Image>(_panel->get());
    _body->setAlign(LV_ALIGN_CENTER);
    _body->setSrc(pack.body);
    _body->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    const auto& expression = pack.expressionForEmotion(Emotion::Neutral);

    auto leftEye  = std::make_unique<ImageSpriteFeature>(_panel->get(), ImageSpriteRole::Eye, expression.leftEye);
    auto rightEye = std::make_unique<ImageSpriteFeature>(_panel->get(), ImageSpriteRole::Eye, expression.rightEye);
    auto mouth    = std::make_unique<ImageSpriteFeature>(_panel->get(), ImageSpriteRole::Mouth, expression.mouth);

    _leftEye  = leftEye.get();
    _rightEye = rightEye.get();
    _mouth    = mouth.get();

    _key_elements.leftEye      = std::move(leftEye);
    _key_elements.rightEye     = std::move(rightEye);
    _key_elements.mouth        = std::move(mouth);
    _key_elements.speechBubble = std::make_unique<DefaultSpeechBubble>(_panel->get(), primaryColor, secondaryColor, font);

    setEmotion(Emotion::Neutral);
}

Container* ImageAvatar::getPanel() const
{
    if (_panel) {
        return _panel.get();
    }
    return NULL;
}

void ImageAvatar::setEmotion(const Emotion& emotion)
{
    if (_pack) {
        applyExpression(_pack->expressionForEmotion(emotion));
    }
    Avatar::setEmotion(emotion);
}

void ImageAvatar::applyExpression(const ImageExpressionSet& expression)
{
    if (_leftEye) {
        _leftEye->setFrames(expression.leftEye);
    }
    if (_rightEye) {
        _rightEye->setFrames(expression.rightEye);
    }
    if (_mouth) {
        _mouth->setFrames(expression.mouth);
    }
}

}  // namespace stackchan::avatar::image

/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "image_sprite_feature.h"
#include "image_avatar_mapping.h"

using namespace uitk;
using namespace uitk::lvgl_cpp;

namespace stackchan::avatar::image {

ImageSpriteFeature::ImageSpriteFeature(lv_obj_t* parent, ImageSpriteRole role, const ImageSpriteFrames& frames)
    : _role(role)
{
    _image = std::make_unique<Image>(parent);
    _image->setAlign(LV_ALIGN_CENTER);
    _image->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    setFrames(frames);
    setPosition(_position);
    setWeight(_role == ImageSpriteRole::Eye ? 100 : 0);
    setRotation(0);
    setSize(0);
    setVisible(true);
}

ImageSpriteFeature::~ImageSpriteFeature()
{
    _image.reset();
}

void ImageSpriteFeature::setFrames(const ImageSpriteFrames& frames)
{
    _frames = &frames;
    applyFrame();
    applyPosition();
}

void ImageSpriteFeature::setPosition(const Vector2i& position)
{
    Element::setPosition(position);
    applyPosition();
}

void ImageSpriteFeature::setWeight(int weight)
{
    Feature::setWeight(weight);
    applyFrame();
}

void ImageSpriteFeature::setRotation(int rotation)
{
    Element::setRotation(rotation);
    _image->setPivot(_image->getWidth() / 2, _image->getHeight() / 2);
    _image->setRotation(rotation);
}

void ImageSpriteFeature::setVisible(bool visible)
{
    Element::setVisible(visible);
    _image->setHidden(!visible);
}

void ImageSpriteFeature::setSize(int size)
{
    Feature::setSize(size);
    applyScale();
}

void ImageSpriteFeature::setEmotion(const Emotion& emotion)
{
    if (getIgnoreEmotion() || _role != ImageSpriteRole::Eye) {
        return;
    }

    switch (emotion) {
        case Emotion::Happy:
            setWeight(35);
            break;
        case Emotion::Angry:
        case Emotion::Sad:
            setWeight(70);
            break;
        case Emotion::Doubt:
            setWeight(65);
            break;
        case Emotion::Sleepy:
            setWeight(20);
            break;
        case Emotion::Neutral:
        default:
            setWeight(100);
            break;
    }
}

int ImageSpriteFeature::frameIndexForWeight() const
{
    if (!_frames || _frames->frameCount <= 0) {
        return 0;
    }

    return selectFrameForWeight(_weight, _frames->frameCount, _frames->reverseWeight);
}

void ImageSpriteFeature::applyFrame()
{
    if (!_frames || !_frames->frames || _frames->frameCount <= 0) {
        return;
    }
    _image->setSrc(_frames->frames[frameIndexForWeight()]);
}

void ImageSpriteFeature::applyPosition()
{
    if (!_frames) {
        return;
    }

    const int offsetX = mapNormalizedToRange(_position.x, _frames->minOffsetX, _frames->maxOffsetX);
    const int offsetY = mapNormalizedToRange(_position.y, _frames->minOffsetY, _frames->maxOffsetY);
    _image->setPos(_frames->centerX + offsetX, _frames->centerY + offsetY);
}

void ImageSpriteFeature::applyScale()
{
    const auto scale = static_cast<uint32_t>(mapNormalizedToRange(_size, 220, 292));
    _image->setScale(scale);
}

}  // namespace stackchan::avatar::image

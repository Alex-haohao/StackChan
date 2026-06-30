#pragma once

#include <stackchan/avatar/avatar/elements/emotion.h>

namespace stackchan::avatar::image {

int clampInt(int value, int minValue, int maxValue);
int selectFrameForWeight(int weight, int frameCount);
int mapNormalizedToRange(int normalized, int minValue, int maxValue);
const char* emotionAssetName(Emotion emotion);

}  // namespace stackchan::avatar::image

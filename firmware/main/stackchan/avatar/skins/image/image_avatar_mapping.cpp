#include "image_avatar_mapping.h"

namespace stackchan::avatar::image {

int clampInt(int value, int minValue, int maxValue)
{
    if (value < minValue) {
        return minValue;
    }
    if (value > maxValue) {
        return maxValue;
    }
    return value;
}

int selectFrameForWeight(int weight, int frameCount)
{
    if (frameCount <= 1) {
        return 0;
    }

    const int clampedWeight = clampInt(weight, 0, 100);
    const int maxFrame      = frameCount - 1;
    return (clampedWeight * maxFrame + 50) / 100;
}

int selectFrameForWeight(int weight, int frameCount, bool reverse)
{
    const int selected = selectFrameForWeight(weight, frameCount);
    if (!reverse || frameCount <= 1) {
        return selected;
    }
    return (frameCount - 1) - selected;
}

int mapNormalizedToRange(int normalized, int minValue, int maxValue)
{
    const int clamped = clampInt(normalized, -100, 100);
    return minValue + ((clamped + 100) * (maxValue - minValue) + 100) / 200;
}

const char* emotionAssetName(Emotion emotion)
{
    switch (emotion) {
        case Emotion::Happy:
            return "happy";
        case Emotion::Angry:
            return "angry";
        case Emotion::Sad:
            return "sad";
        case Emotion::Doubt:
            return "doubt";
        case Emotion::Sleepy:
            return "sleepy";
        case Emotion::Neutral:
        default:
            return "neutral";
    }
}

}  // namespace stackchan::avatar::image

#include <cstdlib>
#include <iostream>
#include <stackchan/avatar/avatar/elements/emotion.h>
#include <stackchan/avatar/skins/image/image_avatar_mapping.h>

namespace {

using stackchan::avatar::Emotion;
using stackchan::avatar::image::emotionAssetName;
using stackchan::avatar::image::mapNormalizedToRange;
using stackchan::avatar::image::selectFrameForWeight;

void expectEqual(int actual, int expected, const char* label)
{
    if (actual != expected) {
        std::cerr << label << ": expected " << expected << ", got " << actual << '\n';
        std::exit(1);
    }
}

void expectString(const char* actual, const char* expected, const char* label)
{
    if (std::string(actual) != expected) {
        std::cerr << label << ": expected " << expected << ", got " << actual << '\n';
        std::exit(1);
    }
}

void testFrameSelection()
{
    expectEqual(selectFrameForWeight(-20, 3), 0, "negative weight clamps to first frame");
    expectEqual(selectFrameForWeight(0, 3), 0, "zero weight uses first frame");
    expectEqual(selectFrameForWeight(50, 3), 1, "middle weight uses middle frame");
    expectEqual(selectFrameForWeight(100, 3), 2, "full weight uses last frame");
    expectEqual(selectFrameForWeight(180, 3), 2, "high weight clamps to last frame");
    expectEqual(selectFrameForWeight(80, 1), 0, "one-frame sprite always selects zero");
}

void testNormalizedRangeMapping()
{
    expectEqual(mapNormalizedToRange(-100, -16, 16), -16, "min normalized maps to min");
    expectEqual(mapNormalizedToRange(0, -16, 16), 0, "center normalized maps to center");
    expectEqual(mapNormalizedToRange(100, -16, 16), 16, "max normalized maps to max");
    expectEqual(mapNormalizedToRange(150, -16, 16), 16, "high normalized clamps");
    expectEqual(mapNormalizedToRange(-150, -16, 16), -16, "low normalized clamps");
}

void testEmotionNames()
{
    expectString(emotionAssetName(Emotion::Neutral), "neutral", "neutral emotion");
    expectString(emotionAssetName(Emotion::Happy), "happy", "happy emotion");
    expectString(emotionAssetName(Emotion::Angry), "angry", "angry emotion");
    expectString(emotionAssetName(Emotion::Sad), "sad", "sad emotion");
    expectString(emotionAssetName(Emotion::Doubt), "doubt", "doubt emotion");
    expectString(emotionAssetName(Emotion::Sleepy), "sleepy", "sleepy emotion");
}

}  // namespace

int main()
{
    testFrameSelection();
    testNormalizedRangeMapping();
    testEmotionNames();
    return 0;
}

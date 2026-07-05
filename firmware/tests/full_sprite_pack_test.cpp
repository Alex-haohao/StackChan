#include <cassert>
#include <string>
#include <vector>

#include <stackchan/avatar/skins/full_sprite/full_sprite_pack.h>

using stackchan::avatar::Emotion;
using namespace stackchan::avatar::full_sprite;

static SpritePack make_pack()
{
    SpritePack pack;
    pack.frame_groups["idle"] = FrameGroup{
        .id           = "idle",
        .frames       = {"frames/idle/00.png", "frames/idle/01.png"},
        .durations_ms = {100, 300},
    };
    pack.frame_groups["talk"] = FrameGroup{
        .id           = "talk",
        .frames       = {"frames/talk/00.png", "frames/talk/01.png"},
        .durations_ms = {80, 120},
    };
    pack.frame_groups["blink"] = FrameGroup{
        .id           = "blink",
        .frames       = {"frames/blink/00.png", "frames/blink/01.png", "frames/blink/02.png"},
        .durations_ms = {40, 60, 100},
    };
    pack.frame_groups["happy"] = FrameGroup{
        .id           = "happy",
        .frames       = {"frames/happy/00.png"},
        .durations_ms = {200},
    };
    pack.emotion_groups[Emotion::Neutral] = "idle";
    pack.emotion_groups[Emotion::Happy]   = "happy";
    return pack;
}

static void selects_talk_only_for_open_mouth_weights()
{
    SpritePack pack = make_pack();

    const RuntimeState closed_mouth{
        .emotion          = Emotion::Neutral,
        .left_eye_weight  = 100,
        .right_eye_weight = 100,
        .mouth_weight     = 20,
    };
    const RuntimeState open_mouth{
        .emotion          = Emotion::Neutral,
        .left_eye_weight  = 100,
        .right_eye_weight = 100,
        .mouth_weight     = 40,
    };

    assert(select_group(pack, closed_mouth)->id == "idle");
    assert(select_group(pack, open_mouth)->id == "talk");
}

static void closed_eyes_select_blink_before_emotion()
{
    SpritePack pack = make_pack();
    const RuntimeState state{
        .emotion          = Emotion::Happy,
        .left_eye_weight  = 25,
        .right_eye_weight = 25,
        .mouth_weight     = 0,
    };

    assert(select_group(pack, state)->id == "blink");
}

static void emotion_group_falls_back_to_idle()
{
    SpritePack pack = make_pack();
    RuntimeState state{
        .emotion          = Emotion::Happy,
        .left_eye_weight  = 100,
        .right_eye_weight = 100,
        .mouth_weight     = 0,
    };
    assert(select_group(pack, state)->id == "happy");

    state.emotion = Emotion::Sad;
    assert(select_group(pack, state)->id == "idle");
}

static void frame_index_loops_by_durations()
{
    SpritePack pack         = make_pack();
    const FrameGroup& group = pack.frame_groups.at("idle");

    assert(frame_index_for_elapsed(group, 0) == 0);
    assert(frame_index_for_elapsed(group, 99) == 0);
    assert(frame_index_for_elapsed(group, 100) == 1);
    assert(frame_index_for_elapsed(group, 399) == 1);
    assert(frame_index_for_elapsed(group, 400) == 0);
}

static void joins_pack_root_and_relative_frame_path()
{
    assert(join_pack_path("/sdcard/stackchan/avatar/active", "frames/idle/00.png") ==
           "/sdcard/stackchan/avatar/active/frames/idle/00.png");
    assert(join_pack_path("/sdcard/stackchan/avatar/active/", "frames/idle/00.png") ==
           "/sdcard/stackchan/avatar/active/frames/idle/00.png");
}

int main()
{
    selects_talk_only_for_open_mouth_weights();
    closed_eyes_select_blink_before_emotion();
    emotion_group_falls_back_to_idle();
    frame_index_loops_by_durations();
    joins_pack_root_and_relative_frame_path();
    return 0;
}

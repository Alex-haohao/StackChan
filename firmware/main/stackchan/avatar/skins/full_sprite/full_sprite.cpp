/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "full_sprite.h"
#include "../default/default.h"

#include <ArduinoJson.h>
#include <esp_log.h>
#include <hal/hal.h>
#include <sys/stat.h>
#include <cstdio>

using namespace uitk::lvgl_cpp;

namespace stackchan::avatar {
namespace {

constexpr const char* kTag = "FullSpriteAvatar";
constexpr const char* kManifestName = "pack.json";

class FullSpriteFeature : public Feature {
public:
    void setEmotion(const Emotion& emotion) override
    {
        if (!getIgnoreEmotion()) {
            _emotion = emotion;
        }
    }

    Emotion getEmotion() const override
    {
        return _emotion;
    }

private:
    Emotion _emotion = Emotion::Neutral;
};

bool file_exists(const std::string& path)
{
    struct stat st {};
    return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

bool read_text_file(const std::string& path, std::string& out)
{
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) {
        return false;
    }

    std::fseek(file, 0, SEEK_END);
    const long size = std::ftell(file);
    if (size <= 0) {
        std::fclose(file);
        return false;
    }
    std::rewind(file);

    out.assign(static_cast<std::size_t>(size), '\0');
    const std::size_t read = std::fread(out.data(), 1, out.size(), file);
    std::fclose(file);
    return read == out.size();
}

bool parse_emotion_key(const char* key, Emotion& emotion)
{
    if (key == nullptr) {
        return false;
    }
    const std::string value(key);
    if (value == "neutral" || value == "idle") {
        emotion = Emotion::Neutral;
    } else if (value == "happy" || value == "laughing") {
        emotion = Emotion::Happy;
    } else if (value == "angry") {
        emotion = Emotion::Angry;
    } else if (value == "sad" || value == "crying") {
        emotion = Emotion::Sad;
    } else if (value == "doubt" || value == "doubtful") {
        emotion = Emotion::Doubt;
    } else if (value == "sleepy") {
        emotion = Emotion::Sleepy;
    } else {
        return false;
    }
    return true;
}

void load_emotion_mappings(const ArduinoJson::JsonObject& emotion_map, full_sprite::SpritePack& pack)
{
    for (ArduinoJson::JsonPair pair : emotion_map) {
        Emotion emotion;
        if (!parse_emotion_key(pair.key().c_str(), emotion)) {
            continue;
        }
        const char* group = pair.value().as<const char*>();
        if (group != nullptr) {
            pack.emotion_groups[emotion] = group;
        }
    }
}

bool load_pack_manifest(const char* pack_root, full_sprite::SpritePack& pack)
{
    const std::string root(pack_root);
    const std::string manifest_path = full_sprite::join_pack_path(root, kManifestName);

    std::string manifest_json;
    if (!read_text_file(manifest_path, manifest_json)) {
        ESP_LOGW(kTag, "pack manifest not readable: %s", manifest_path.c_str());
        return false;
    }

    ArduinoJson::JsonDocument doc;
    auto error = ArduinoJson::deserializeJson(doc, manifest_json);
    if (error) {
        ESP_LOGW(kTag, "pack manifest JSON failed: %s", error.c_str());
        return false;
    }

    const char* target = doc["target"].as<const char*>();
    if (target == nullptr || std::string(target) != "m5stack-stackchan-esp-idf-full-frame") {
        ESP_LOGW(kTag, "unsupported avatar pack target");
        return false;
    }

    if (doc["canvas"]["width"].as<int>() != full_sprite::kCanvasWidth ||
        doc["canvas"]["height"].as<int>() != full_sprite::kCanvasHeight) {
        ESP_LOGW(kTag, "avatar pack canvas must be %dx%d", full_sprite::kCanvasWidth, full_sprite::kCanvasHeight);
        return false;
    }

    pack = {};
    pack.root = root;

    ArduinoJson::JsonObject groups = doc["frameGroups"].as<ArduinoJson::JsonObject>();
    for (ArduinoJson::JsonPair group_pair : groups) {
        full_sprite::FrameGroup group;
        group.id = group_pair.key().c_str();
        ArduinoJson::JsonObject group_json = group_pair.value().as<ArduinoJson::JsonObject>();

        ArduinoJson::JsonArray frames = group_json["frames"].as<ArduinoJson::JsonArray>();
        for (ArduinoJson::JsonVariant frame : frames) {
            const char* relative_path = frame.as<const char*>();
            if (relative_path == nullptr) {
                continue;
            }
            const std::string full_path = full_sprite::join_pack_path(root, relative_path);
            if (!file_exists(full_path)) {
                ESP_LOGW(kTag, "skip missing frame: %s", full_path.c_str());
                continue;
            }
            group.frames.emplace_back(relative_path);
        }

        ArduinoJson::JsonArray durations = group_json["durationsMs"].as<ArduinoJson::JsonArray>();
        for (ArduinoJson::JsonVariant duration : durations) {
            group.durations_ms.emplace_back(duration.as<int>());
        }

        if (!group.frames.empty()) {
            pack.frame_groups[group.id] = std::move(group);
        }
    }

    if (doc["mappings"]["emotion"].is<ArduinoJson::JsonObject>()) {
        load_emotion_mappings(doc["mappings"]["emotion"].as<ArduinoJson::JsonObject>(), pack);
    }

    if (full_sprite::find_group(pack, pack.idle_group) == nullptr) {
        ESP_LOGW(kTag, "avatar pack has no idle frame group");
        return false;
    }

    const char* pack_id = doc["packId"].as<const char*>();
    ESP_LOGI(kTag, "loaded avatar pack: %s (%d groups)", pack_id != nullptr ? pack_id : "unknown",
             static_cast<int>(pack.frame_groups.size()));
    return true;
}

}  // namespace

bool FullSpriteAvatar::init(lv_obj_t* parent, const lv_font_t* font, const char* pack_root)
{
    if (!load_pack_manifest(pack_root, _pack)) {
        return false;
    }

    _panel = std::make_unique<Container>(parent);
    _panel->align(LV_ALIGN_CENTER, 0, 0);
    _panel->setSize(full_sprite::kCanvasWidth, full_sprite::kCanvasHeight);
    _panel->setRadius(0);
    _panel->setBorderWidth(0);
    _panel->setBgColor(lv_color_black());
    _panel->removeFlag(LV_OBJ_FLAG_SCROLLABLE);

    _frame = std::make_unique<Image>(_panel->get());
    _frame->align(LV_ALIGN_CENTER, 0, 0);

    _key_elements.leftEye  = std::make_unique<FullSpriteFeature>();
    _key_elements.rightEye = std::make_unique<FullSpriteFeature>();
    _key_elements.mouth    = std::make_unique<FullSpriteFeature>();
    _key_elements.leftEye->setWeight(100);
    _key_elements.rightEye->setWeight(100);
    _key_elements.mouth->setWeight(0);
    _key_elements.speechBubble =
        std::make_unique<DefaultSpeechBubble>(_panel->get(), lv_color_white(), lv_color_black(), font);

    return setFrameForState(GetHAL().millis());
}

void FullSpriteAvatar::update()
{
    setFrameForState(GetHAL().millis());
    Avatar::update();
}

Container* FullSpriteAvatar::getPanel() const
{
    return _panel.get();
}

bool FullSpriteAvatar::setFrameForState(std::uint32_t now_ms)
{
    const full_sprite::RuntimeState state{
        .emotion          = getEmotion(),
        .left_eye_weight  = leftEye().getWeight(),
        .right_eye_weight = rightEye().getWeight(),
        .mouth_weight     = mouth().getWeight(),
    };

    const full_sprite::FrameGroup* group = full_sprite::select_group(_pack, state);
    if (group == nullptr) {
        return false;
    }

    if (_current_group_id != group->id) {
        _current_group_id = group->id;
        _current_group_started_ms = now_ms;
    }

    const std::uint32_t elapsed = now_ms - _current_group_started_ms;
    std::string path = full_sprite::frame_path_for_elapsed(_pack, *group, elapsed);
    if (path.empty() || path == _current_frame_path) {
        return true;
    }

    _current_frame_path = std::move(path);
    _frame->setSrc(_current_frame_path.c_str());
    return true;
}

std::unique_ptr<FullSpriteAvatar> create_full_sprite_avatar(lv_obj_t* parent, const lv_font_t* font,
                                                            const char* pack_root)
{
    auto avatar = std::make_unique<FullSpriteAvatar>();
    if (!avatar->init(parent, font, pack_root)) {
        return nullptr;
    }
    return avatar;
}

}  // namespace stackchan::avatar

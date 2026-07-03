/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import "testing"

func TestSelfHostedGenerateLicenseUsesStableSerialFromMac(t *testing.T) {
	license, err := BuildSelfHostedGenerateLicense("AA-BB-CC-DD-EE-FF")
	if err != nil {
		t.Fatalf("BuildSelfHostedGenerateLicense returned error: %v", err)
	}

	if license.SerialNumber != "stackchan-aabbccddeeff" {
		t.Fatalf("SerialNumber = %q, want %q", license.SerialNumber, "stackchan-aabbccddeeff")
	}
	if license.BoardName != "StackChan" {
		t.Fatalf("BoardName = %q, want StackChan", license.BoardName)
	}
}

func TestSelfHostedDeviceListRequiresManagerDevice(t *testing.T) {
	devices := BuildSelfHostedDeviceList(SelfHostedDeviceListInput{
		MacAddress:       "aa:bb:cc:dd:ee:ff",
		ManagerHasDevice: false,
		CompatAgentID:    1,
	})

	if len(devices.List) != 0 {
		t.Fatalf("len(devices.List) = %d, want 0", len(devices.List))
	}
}

func TestBuildSelfHostedCompatModelListUsesManagerModelIDs(t *testing.T) {
	modelList := BuildSelfHostedCompatModelList(
		[]ManagerModelBasic{
			{ID: "LLM_DoubaoLLM", ModelName: "DeepSeek via Volc Ark"},
		},
		ManagerAgent{LLMModelID: "LLM_DoubaoLLM"},
	)

	models := modelList["modelList"].([]map[string]interface{})
	if len(models) != 1 {
		t.Fatalf("len(modelList) = %d, want 1", len(models))
	}
	if models[0]["name"] != "LLM_DoubaoLLM" {
		t.Fatalf("model name = %q, want manager model id", models[0]["name"])
	}
	if models[0]["description"] != "DeepSeek via Volc Ark" {
		t.Fatalf("description = %q, want model name", models[0]["description"])
	}
}

func TestBuildSelfHostedCompatTTSListGroupsVoicesByManagerLanguages(t *testing.T) {
	ttsList := BuildSelfHostedCompatTTSList(
		ManagerAgent{TTSVoiceID: "voice-a", TTSLanguage: "中文"},
		[]ManagerVoice{
			{ID: "voice-a", Name: "Warm Voice", Languages: "中文,English", VoiceDemo: "https://example.com/demo.wav"},
		},
	)

	languages := ttsList["languages"].([]string)
	if len(languages) != 2 || languages[0] != "中文" || languages[1] != "English" {
		t.Fatalf("languages = %#v, want Chinese and English", languages)
	}

	voices := ttsList["tts_voices"].(map[string][]map[string]interface{})
	if len(voices["中文"]) != 1 {
		t.Fatalf("len(voices[中文]) = %d, want 1", len(voices["中文"]))
	}
	if voices["中文"][0]["voice_id"] != "voice-a" {
		t.Fatalf("voice_id = %q, want voice-a", voices["中文"][0]["voice_id"])
	}
}

func TestBuildSelfHostedCompatAgentUpdateMapsLegacyFieldsWithoutDroppingMemory(t *testing.T) {
	pitch := 2
	update := BuildSelfHostedManagerAgentUpdate(
		SelfHostedCompatAgentUpdateInput{
			AgentName:      "stackchan",
			LLMModel:       "LLM_DeepSeekLLM",
			TTSVoice:       "voice-b",
			TTSSpeechSpeed: "fast",
			TTSPitch:       &pitch,
			Language:       "中文",
			Character:      "You are StackChan.",
			Memory:         "Keep useful facts.",
			MemoryType:     CompatMemoryShortTerm,
		},
		ManagerAgent{MemModelID: "Memory_custom_short"},
	)

	if update.LLMModelID != "LLM_DeepSeekLLM" {
		t.Fatalf("LLMModelID = %q", update.LLMModelID)
	}
	if update.TTSVoiceID != "voice-b" {
		t.Fatalf("TTSVoiceID = %q", update.TTSVoiceID)
	}
	if update.TTSRate == nil || *update.TTSRate != 50 {
		t.Fatalf("TTSRate = %v, want 50", update.TTSRate)
	}
	if update.TTSPitch == nil || *update.TTSPitch != 100 {
		t.Fatalf("TTSPitch = %v, want 100", update.TTSPitch)
	}
	if update.MemModelID != "Memory_custom_short" {
		t.Fatalf("MemModelID = %q, want existing non-empty memory model", update.MemModelID)
	}
	if update.TTSLanguage != "中文" || update.Language != "中文" || update.LangCode != "zh_CN" {
		t.Fatalf("language fields = %q/%q/%q", update.TTSLanguage, update.Language, update.LangCode)
	}
}

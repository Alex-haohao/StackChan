/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import (
	"strconv"
	"strings"
)

const (
	CompatMemoryOff       = "OFF"
	CompatMemoryShortTerm = "SHORT_TERM"

	managerMemoryNone       = "Memory_nomem"
	managerMemoryLocalShort = "Memory_mem_local_short"
)

type SelfHostedCompatAgentUpdateInput struct {
	AgentName      string
	LLMModel       string
	TTSVoice       string
	TTSSpeechSpeed string
	TTSPitch       *int
	Language       string
	Character      string
	Memory         string
	MemoryType     string
}

func BuildSelfHostedCompatAgent(config SelfHostedConfig, agent ManagerAgent) map[string]interface{} {
	now := firstNonEmpty(agent.UpdatedAt, agent.CreatedAt, "1970-01-01T00:00:00Z")
	agentName := firstNonEmpty(agent.AgentName, config.AgentName)
	language := firstNonEmpty(agent.TTSLanguage, agent.Language, compatLanguageCode(agent.LangCode), "zh")

	return map[string]interface{}{
		"id":                    config.CompatAgentID,
		"user_id":               0,
		"agent_name":            agentName,
		"tts_voice":             agent.TTSVoiceID,
		"llm_model":             agent.LLMModelID,
		"assistant_name":        agentName,
		"user_name":             "self-hosted",
		"created_at":            firstNonEmpty(agent.CreatedAt, now),
		"updated_at":            now,
		"memory":                agent.SummaryMemory,
		"character":             agent.SystemPrompt,
		"long_memory_switch":    0,
		"lang_code":             compatLanguageCode(agent.LangCode),
		"language":              language,
		"tts_speech_speed":      compatSpeechSpeed(agent.TTSRate),
		"asr_speed":             "normal",
		"is_deleted":            0,
		"tts_pitch":             compatPitch(agent.TTSPitch),
		"agent_template_id":     0,
		"knowledge_base_ids":    []int{},
		"memory_updated_at":     now,
		"source":                "self_hosted",
		"mcp_endpoints":         []string{},
		"memory_type":           compatMemoryType(agent.MemModelID),
		"max_message_count":     0,
		"deviceCount":           0,
		"product_mcp_endpoints": []string{},
	}
}

func BuildSelfHostedCompatModelList(models []ManagerModelBasic, agent ManagerAgent) map[string]interface{} {
	items := make([]map[string]interface{}, 0, len(models)+1)
	seen := map[string]bool{}
	for _, model := range models {
		id := strings.TrimSpace(model.ID)
		if id == "" || seen[id] {
			continue
		}
		seen[id] = true
		items = append(items, map[string]interface{}{
			"name":            id,
			"description":     firstNonEmpty(model.ModelName, id),
			"xiaozhi_version": []string{"self_hosted"},
			"role":            []string{"assistant"},
		})
	}
	if agent.LLMModelID != "" && !seen[agent.LLMModelID] {
		items = append(items, map[string]interface{}{
			"name":            agent.LLMModelID,
			"description":     agent.LLMModelID,
			"xiaozhi_version": []string{"self_hosted"},
			"role":            []string{"assistant"},
		})
	}
	return map[string]interface{}{"modelList": items}
}

func BuildSelfHostedCompatTTSList(agent ManagerAgent, voices []ManagerVoice) map[string]interface{} {
	voiceMap := map[string][]map[string]interface{}{}
	languages := []string{}
	seenLanguages := map[string]bool{}
	addLanguage := func(language string) {
		language = strings.TrimSpace(language)
		if language == "" || seenLanguages[language] {
			return
		}
		seenLanguages[language] = true
		languages = append(languages, language)
	}

	fallbackLanguage := firstNonEmpty(agent.TTSLanguage, agent.Language, compatLanguageCode(agent.LangCode), "zh")
	for _, voice := range voices {
		id := strings.TrimSpace(voice.ID)
		if id == "" {
			continue
		}
		voiceLanguages := splitCompatLanguages(firstNonEmpty(voice.Languages, fallbackLanguage))
		if len(voiceLanguages) == 0 {
			voiceLanguages = []string{fallbackLanguage}
		}
		for _, language := range voiceLanguages {
			addLanguage(language)
			voiceMap[language] = append(voiceMap[language], map[string]interface{}{
				"top":        voice.ID == agent.TTSVoiceID,
				"voice_id":   voice.ID,
				"voice_name": firstNonEmpty(voice.Name, voice.ID),
				"language":   language,
				"created_at": "",
				"voice_demo": voice.VoiceDemo,
			})
		}
	}

	if len(voiceMap) == 0 && agent.TTSVoiceID != "" {
		addLanguage(fallbackLanguage)
		voiceMap[fallbackLanguage] = []map[string]interface{}{
			{
				"top":        true,
				"voice_id":   agent.TTSVoiceID,
				"voice_name": agent.TTSVoiceID,
				"language":   fallbackLanguage,
				"created_at": "",
				"voice_demo": "",
			},
		}
	}

	return map[string]interface{}{
		"languages":  languages,
		"tts_voices": voiceMap,
	}
}

func BuildSelfHostedManagerAgentUpdate(input SelfHostedCompatAgentUpdateInput, current ManagerAgent) ManagerAgentUpdate {
	update := ManagerAgentUpdate{
		AgentName:     strings.TrimSpace(input.AgentName),
		LLMModelID:    strings.TrimSpace(input.LLMModel),
		TTSVoiceID:    strings.TrimSpace(input.TTSVoice),
		TTSLanguage:   strings.TrimSpace(input.Language),
		SystemPrompt:  strings.TrimSpace(input.Character),
		SummaryMemory: strings.TrimSpace(input.Memory),
	}

	if update.TTSLanguage != "" {
		update.Language = update.TTSLanguage
		update.LangCode = managerLangCode(update.TTSLanguage)
	}
	if input.TTSSpeechSpeed != "" {
		rate := managerSpeechRate(input.TTSSpeechSpeed)
		update.TTSRate = &rate
	}
	if input.TTSPitch != nil {
		pitch := managerPitch(*input.TTSPitch)
		update.TTSPitch = &pitch
	}

	switch strings.ToUpper(strings.TrimSpace(input.MemoryType)) {
	case CompatMemoryOff:
		update.MemModelID = managerMemoryNone
	case CompatMemoryShortTerm:
		if current.MemModelID != "" && !strings.Contains(strings.ToLower(current.MemModelID), "nomem") {
			update.MemModelID = current.MemModelID
		} else {
			update.MemModelID = managerMemoryLocalShort
		}
	}

	return update
}

func BuildSelfHostedCompatMcpEndpoint(address string) map[string]interface{} {
	return map[string]interface{}{
		"token": strings.TrimSpace(address),
	}
}

func BuildSelfHostedCompatEndpointList(agentID int, tools []string) map[string]interface{} {
	endpointTools := make([]map[string]interface{}, 0, len(tools))
	for _, tool := range tools {
		name := strings.TrimSpace(tool)
		if name == "" {
			continue
		}
		endpointTools = append(endpointTools, map[string]interface{}{
			"name":        name,
			"description": "",
			"inputSchema": map[string]interface{}{
				"type":       "object",
				"properties": map[string]interface{}{},
			},
		})
	}
	status := "offline"
	if len(endpointTools) > 0 {
		status = "online"
	}
	return map[string]interface{}{
		"endpoints": []map[string]interface{}{
			{
				"endpointId":      "agent_" + intToString(agentID),
				"connectionCount": len(endpointTools),
				"status":          status,
				"rpm":             0,
				"lastRequestTime": nil,
				"totalRequests":   0,
				"tools":           endpointTools,
				"brokers":         []string{},
			},
		},
	}
}

func compatSpeechSpeed(rate int) string {
	if rate <= -25 {
		return "slow"
	}
	if rate >= 25 {
		return "fast"
	}
	return "normal"
}

func managerSpeechRate(speed string) int {
	switch strings.ToLower(strings.TrimSpace(speed)) {
	case "slow":
		return -50
	case "fast":
		return 50
	default:
		return 0
	}
}

func compatPitch(pitch int) int {
	if pitch > 2 || pitch < -2 {
		pitch = pitch / 50
	}
	if pitch > 2 {
		return 2
	}
	if pitch < -2 {
		return -2
	}
	return pitch
}

func managerPitch(pitch int) int {
	if pitch > 2 {
		pitch = 2
	}
	if pitch < -2 {
		pitch = -2
	}
	return pitch * 50
}

func compatMemoryType(memoryModelID string) string {
	if strings.Contains(strings.ToLower(strings.TrimSpace(memoryModelID)), "nomem") {
		return CompatMemoryOff
	}
	return CompatMemoryShortTerm
}

func splitCompatLanguages(value string) []string {
	replacer := strings.NewReplacer("、", ",", "；", ",", ";", ",", "，", ",", "/", ",")
	parts := strings.Split(replacer.Replace(value), ",")
	languages := make([]string, 0, len(parts))
	seen := map[string]bool{}
	for _, part := range parts {
		language := strings.TrimSpace(part)
		if language == "" || seen[language] {
			continue
		}
		seen[language] = true
		languages = append(languages, language)
	}
	return languages
}

func compatLanguageCode(value string) string {
	switch strings.ToLower(strings.TrimSpace(value)) {
	case "zh_cn", "zh-cn", "zh":
		return "zh"
	case "en_us", "en-us", "en":
		return "en"
	default:
		return strings.TrimSpace(value)
	}
}

func managerLangCode(language string) string {
	switch strings.ToLower(strings.TrimSpace(language)) {
	case "zh", "zh_cn", "zh-cn", "中文", "普通话":
		return "zh_CN"
	case "en", "en_us", "en-us", "english":
		return "en_US"
	default:
		return ""
	}
}

func intToString(value int) string {
	return strconv.Itoa(value)
}

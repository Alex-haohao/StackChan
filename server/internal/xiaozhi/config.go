/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import (
	"strconv"
	"strings"

	"github.com/gogf/gf/v2/frame/g"
)

const (
	ProviderOfficial   = "official"
	ProviderSelfHosted = "self_hosted"
)

type SelfHostedConfig struct {
	ManagerAPIBaseURL    string
	ManagerToken         string
	AgentID              string
	CompatAgentID        int
	AgentName            string
	Board                string
	AppVersion           string
	GenerateLicenseToken string
}

func ProviderMode() string {
	value := strings.ToLower(strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.provider").String()))
	if value == "" {
		return ProviderOfficial
	}
	return value
}

func IsSelfHostedProvider() bool {
	return ProviderMode() == ProviderSelfHosted
}

func GetCompatToken() (string, error) {
	if IsSelfHostedProvider() {
		token := strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.generate_license_token").String())
		if token == "" {
			return "self-hosted", nil
		}
		return token, nil
	}
	return GetToken()
}

func GetSelfHostedConfig() SelfHostedConfig {
	compatAgentID := g.Cfg().MustGet(ctx, "xiaozhi.self_host_compat_agent_id").Int()
	if compatAgentID == 0 {
		compatAgentID = 1
	}

	agentID := strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_agent_id").String())
	if agentID == "" && compatAgentID > 0 {
		agentID = strconv.Itoa(compatAgentID)
	}

	agentName := strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_agent_name").String())
	if agentName == "" {
		agentName = "StackChan"
	}
	board := strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_board").String())
	if board == "" {
		board = "StackChan"
	}
	appVersion := strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_app_version").String())
	if appVersion == "" {
		appVersion = "self-hosted"
	}

	return SelfHostedConfig{
		ManagerAPIBaseURL:    strings.TrimRight(strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_manager_api_base_url").String()), "/"),
		ManagerToken:         strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.self_host_manager_token").String()),
		AgentID:              agentID,
		CompatAgentID:        compatAgentID,
		AgentName:            agentName,
		Board:                board,
		AppVersion:           appVersion,
		GenerateLicenseToken: strings.TrimSpace(g.Cfg().MustGet(ctx, "xiaozhi.generate_license_token").String()),
	}
}

func (config SelfHostedConfig) HasManagerCredentials() bool {
	return config.ManagerAPIBaseURL != "" && config.ManagerToken != "" && config.AgentID != ""
}

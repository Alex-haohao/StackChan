/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import (
	"bytes"
	"context"
	"encoding/json"
	"fmt"
	"io"
	"net/http"
	"net/url"
	"strings"
	"time"
)

const managerDuplicateMacCode = 10161

type SelfHostedManager interface {
	HasDevice(ctx context.Context, macAddress string) (bool, error)
	ManualAddDevice(ctx context.Context, macAddress string) error
	BindDeviceCode(ctx context.Context, deviceCode string) error
}

type SelfHostedManagerClient struct {
	config SelfHostedConfig
	http   *http.Client
}

type managerResult struct {
	Code int             `json:"code"`
	Msg  string          `json:"msg"`
	Data json.RawMessage `json:"data"`
}

type managerDevice struct {
	ID         string `json:"id"`
	MacAddress string `json:"macAddress"`
}

type ManagerAgent struct {
	ID              string `json:"id"`
	UserID          string `json:"userId"`
	AgentCode       string `json:"agentCode"`
	AgentName       string `json:"agentName"`
	ASRModelID      string `json:"asrModelId"`
	VADModelID      string `json:"vadModelId"`
	LLMModelID      string `json:"llmModelId"`
	SLMModelID      string `json:"slmModelId"`
	VLLMModelID     string `json:"vllmModelId"`
	TTSModelID      string `json:"ttsModelId"`
	TTSVoiceID      string `json:"ttsVoiceId"`
	TTSLanguage     string `json:"ttsLanguage"`
	TTSVolume       int    `json:"ttsVolume"`
	TTSRate         int    `json:"ttsRate"`
	TTSPitch        int    `json:"ttsPitch"`
	MemModelID      string `json:"memModelId"`
	IntentModelID   string `json:"intentModelId"`
	ChatHistoryConf int    `json:"chatHistoryConf"`
	SystemPrompt    string `json:"systemPrompt"`
	SummaryMemory   string `json:"summaryMemory"`
	LangCode        string `json:"langCode"`
	Language        string `json:"language"`
	Sort            int    `json:"sort"`
	CreatedAt       string `json:"createdAt"`
	UpdatedAt       string `json:"updatedAt"`
}

type ManagerModelBasic struct {
	ID        string `json:"id"`
	ModelName string `json:"modelName"`
}

type ManagerVoice struct {
	ID        string `json:"id"`
	Name      string `json:"name"`
	VoiceDemo string `json:"voiceDemo"`
	Languages string `json:"languages"`
	IsClone   bool   `json:"isClone"`
}

type ManagerAgentUpdate struct {
	AgentName     string `json:"agentName,omitempty"`
	LLMModelID    string `json:"llmModelId,omitempty"`
	TTSVoiceID    string `json:"ttsVoiceId,omitempty"`
	TTSLanguage   string `json:"ttsLanguage,omitempty"`
	TTSRate       *int   `json:"ttsRate,omitempty"`
	TTSPitch      *int   `json:"ttsPitch,omitempty"`
	MemModelID    string `json:"memModelId,omitempty"`
	SystemPrompt  string `json:"systemPrompt,omitempty"`
	SummaryMemory string `json:"summaryMemory,omitempty"`
	LangCode      string `json:"langCode,omitempty"`
	Language      string `json:"language,omitempty"`
}

func NewSelfHostedManagerClient(config SelfHostedConfig) *SelfHostedManagerClient {
	return &SelfHostedManagerClient{
		config: config,
		http: &http.Client{
			Timeout: 10 * time.Second,
		},
	}
}

func (client *SelfHostedManagerClient) HasDevice(ctx context.Context, macAddress string) (bool, error) {
	cleanMac, err := NormalizeSelfHostedMac(macAddress)
	if err != nil {
		return false, err
	}

	var result managerResult
	if err := client.do(ctx, http.MethodGet, "/device/bind/"+url.PathEscape(client.config.AgentID), nil, &result); err != nil {
		return false, err
	}
	if result.Code != 0 {
		return false, fmt.Errorf("manager device list failed: %s", result.Msg)
	}

	var devices []managerDevice
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &devices); err != nil {
			return false, err
		}
	}
	for _, device := range devices {
		deviceMac := firstNonEmpty(device.MacAddress, device.ID)
		if normalized, err := NormalizeSelfHostedMac(deviceMac); err == nil && normalized == cleanMac {
			return true, nil
		}
	}
	return false, nil
}

func (client *SelfHostedManagerClient) ManualAddDevice(ctx context.Context, macAddress string) error {
	formattedMac, err := FormatSelfHostedMac(macAddress)
	if err != nil {
		return err
	}

	body := map[string]string{
		"agentId":    client.config.AgentID,
		"board":      client.config.Board,
		"appVersion": client.config.AppVersion,
		"macAddress": formattedMac,
	}
	var result managerResult
	if err := client.do(ctx, http.MethodPost, "/device/manual-add", body, &result); err != nil {
		return err
	}
	if result.Code == 0 || result.Code == managerDuplicateMacCode {
		return nil
	}
	return fmt.Errorf("manager manual-add failed: %s", result.Msg)
}

func (client *SelfHostedManagerClient) BindDeviceCode(ctx context.Context, deviceCode string) error {
	path := "/device/bind/" + url.PathEscape(client.config.AgentID) + "/" + url.PathEscape(strings.TrimSpace(deviceCode))
	var result managerResult
	if err := client.do(ctx, http.MethodPost, path, nil, &result); err != nil {
		return err
	}
	if result.Code == 0 {
		return nil
	}
	return fmt.Errorf("manager bind-device failed: %s", result.Msg)
}

func (client *SelfHostedManagerClient) GetAgent(ctx context.Context) (ManagerAgent, error) {
	var result managerResult
	if err := client.do(ctx, http.MethodGet, "/agent/"+url.PathEscape(client.config.AgentID), nil, &result); err != nil {
		return ManagerAgent{}, err
	}
	if err := requireManagerOK(result, "manager get-agent"); err != nil {
		return ManagerAgent{}, err
	}

	var agent ManagerAgent
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &agent); err != nil {
			return ManagerAgent{}, err
		}
	}
	return agent, nil
}

func (client *SelfHostedManagerClient) ListModelBasics(ctx context.Context, modelType string) ([]ManagerModelBasic, error) {
	query := url.Values{}
	query.Set("modelType", modelType)
	path := "/models/names?" + query.Encode()

	var result managerResult
	if err := client.do(ctx, http.MethodGet, path, nil, &result); err != nil {
		return nil, err
	}
	if err := requireManagerOK(result, "manager list-models"); err != nil {
		return nil, err
	}

	var models []ManagerModelBasic
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &models); err != nil {
			return nil, err
		}
	}
	return models, nil
}

func (client *SelfHostedManagerClient) ListVoices(ctx context.Context, ttsModelID string) ([]ManagerVoice, error) {
	if strings.TrimSpace(ttsModelID) == "" {
		return nil, fmt.Errorf("tts model id is required")
	}

	var result managerResult
	if err := client.do(ctx, http.MethodGet, "/models/"+url.PathEscape(ttsModelID)+"/voices", nil, &result); err != nil {
		return nil, err
	}
	if err := requireManagerOK(result, "manager list-voices"); err != nil {
		return nil, err
	}

	var voices []ManagerVoice
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &voices); err != nil {
			return nil, err
		}
	}
	return voices, nil
}

func (client *SelfHostedManagerClient) UpdateAgent(ctx context.Context, update ManagerAgentUpdate) error {
	var result managerResult
	if err := client.do(ctx, http.MethodPut, "/agent/"+url.PathEscape(client.config.AgentID), update, &result); err != nil {
		return err
	}
	return requireManagerOK(result, "manager update-agent")
}

func (client *SelfHostedManagerClient) GetAgentMcpAddress(ctx context.Context) (string, error) {
	var result managerResult
	if err := client.do(ctx, http.MethodGet, "/agent/mcp/address/"+url.PathEscape(client.config.AgentID), nil, &result); err != nil {
		return "", err
	}
	if err := requireManagerOK(result, "manager get-mcp-address"); err != nil {
		return "", err
	}

	var address string
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &address); err != nil {
			return "", err
		}
	}
	return strings.TrimSpace(address), nil
}

func (client *SelfHostedManagerClient) GetAgentMcpTools(ctx context.Context) ([]string, error) {
	var result managerResult
	if err := client.do(ctx, http.MethodGet, "/agent/mcp/tools/"+url.PathEscape(client.config.AgentID), nil, &result); err != nil {
		return nil, err
	}
	if err := requireManagerOK(result, "manager get-mcp-tools"); err != nil {
		return nil, err
	}

	var tools []string
	if len(result.Data) > 0 {
		if err := json.Unmarshal(result.Data, &tools); err != nil {
			return nil, err
		}
	}
	return tools, nil
}

func (client *SelfHostedManagerClient) do(
	ctx context.Context,
	method string,
	path string,
	body interface{},
	target interface{},
) error {
	if !client.config.HasManagerCredentials() {
		return fmt.Errorf("self-hosted xiaozhi manager credentials are incomplete")
	}

	var reader io.Reader
	if body != nil {
		data, err := json.Marshal(body)
		if err != nil {
			return err
		}
		reader = bytes.NewReader(data)
	}

	request, err := http.NewRequestWithContext(ctx, method, client.config.ManagerAPIBaseURL+path, reader)
	if err != nil {
		return err
	}
	request.Header.Set("Accept", "application/json")
	request.Header.Set("Authorization", "Bearer "+client.config.ManagerToken)
	if body != nil {
		request.Header.Set("Content-Type", "application/json")
	}

	response, err := client.http.Do(request)
	if err != nil {
		return err
	}
	defer response.Body.Close()

	payload, err := io.ReadAll(response.Body)
	if err != nil {
		return err
	}
	if response.StatusCode < 200 || response.StatusCode >= 300 {
		return fmt.Errorf("manager api returned HTTP %d: %s", response.StatusCode, strings.TrimSpace(string(payload)))
	}
	if err := json.Unmarshal(payload, target); err != nil {
		return err
	}
	return nil
}

func requireManagerOK(result managerResult, action string) error {
	if result.Code == 0 {
		return nil
	}
	return fmt.Errorf("%s failed: %s", action, result.Msg)
}

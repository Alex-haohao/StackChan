/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import (
	"context"
	"net/http"
	"strings"

	"github.com/gogf/gf/v2/frame/g"
	"github.com/gogf/gf/v2/net/ghttp"

	svcxiaozhi "stackChan/internal/xiaozhi"
)

func BindCompatRoutes(group *ghttp.RouterGroup) {
	group.GET("/api/developers/generate-license", handleCompatGenerateLicense)
	group.GET("/api/developers/devices", handleCompatDevices)
	group.GET("/api/developers/agent-templates/list", handleCompatEmptyList)
	group.GET("/api/developers/products/list", handleCompatProducts)
	group.GET("/api/developers/products/*productPath", handleCompatProductPath)
	group.GET("/api/developers/mcp-endpoints", handleCompatEmptyArray)
	group.POST("/api/developers/mcp-endpoints", handleCompatSuccess)
	group.GET("/api/agents/common-mcp-tool/list", handleCompatEmptyArray)
	group.GET("/api/agents", handleCompatAgents)
	group.POST("/api/agents", handleCompatCreateAgent)
	group.POST("/api/agents/devices/activate", handleCompatActivate)
	group.GET("/api/agents/*agentPath", handleCompatAgentPath)
	group.POST("/api/agents/*agentPath", handleCompatAgentPath)
	group.GET("/api/roles/model-list", handleCompatModelList)
	group.GET("/api/user/tts-list", handleCompatTtsList)
}

func handleCompatGenerateLicense(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	license, err := svcxiaozhi.BuildSelfHostedGenerateLicense(r.Get("seed").String())
	if err != nil {
		writeCompatError(r, http.StatusBadRequest, err.Error())
		return
	}
	writeCompatData(r, license)
}

func handleCompatActivate(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}

	var payload struct {
		SerialNumber string `json:"serial_number"`
		MacAddress   string `json:"mac_address"`
	}
	if err := r.Parse(&payload); err != nil {
		writeCompatError(r, http.StatusBadRequest, err.Error())
		return
	}
	macAddress := firstCompatValue(payload.MacAddress, r.Get("mac_address").String(), macFromCompatSerial(payload.SerialNumber))
	if macAddress == "" {
		writeCompatError(r, http.StatusBadRequest, "mac_address is required")
		return
	}

	config := svcxiaozhi.GetSelfHostedConfig()
	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	if err := manager.ManualAddDevice(context.Background(), macAddress); err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	agent := compatAgentFromManager(config, manager)

	formattedMac, _ := svcxiaozhi.FormatSelfHostedMac(macAddress)
	serialNumber := firstCompatValue(payload.SerialNumber, r.Get("serial_number").String())
	if serialNumber == "" {
		cleanMac, _ := svcxiaozhi.NormalizeSelfHostedMac(macAddress)
		serialNumber = "stackchan-" + cleanMac
	}

	writeCompatData(r, g.Map{
		"macAddress":    formattedMac,
		"serialNumber":  serialNumber,
		"agentId":       config.CompatAgentID,
		"agent":         agent,
		"device":        g.Map{"macAddress": formattedMac, "agentId": config.CompatAgentID},
		"managerSource": "self_hosted",
	})
}

func handleCompatDevices(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}

	config := svcxiaozhi.GetSelfHostedConfig()
	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	macAddress := firstCompatValue(r.Get("mac_address").String(), macFromCompatSerial(r.Get("serial_number").String()))
	if macAddress == "" {
		writeCompatList(r, svcxiaozhi.BuildSelfHostedDeviceList(svcxiaozhi.SelfHostedDeviceListInput{}))
		return
	}

	hasDevice, err := manager.HasDevice(context.Background(), macAddress)
	if err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	list := svcxiaozhi.BuildSelfHostedDeviceList(svcxiaozhi.SelfHostedDeviceListInput{
		MacAddress:       macAddress,
		SerialNumber:     r.Get("serial_number").String(),
		ManagerHasDevice: hasDevice,
		CompatAgentID:    config.CompatAgentID,
	})
	writeCompatList(r, list)
}

func handleCompatAgents(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	config := svcxiaozhi.GetSelfHostedConfig()
	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	agent, err := manager.GetAgent(context.Background())
	if err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	writeCompatData(r, []map[string]interface{}{svcxiaozhi.BuildSelfHostedCompatAgent(config, agent)})
}

func handleCompatCreateAgent(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	config := svcxiaozhi.GetSelfHostedConfig()
	writeCompatData(r, g.Map{"id": config.CompatAgentID})
}

func handleCompatAgentPath(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}

	config := svcxiaozhi.GetSelfHostedConfig()
	parts := strings.Split(strings.Trim(r.Get("agentPath").String(), "/"), "/")
	if len(parts) == 0 || parts[0] == "" {
		manager := svcxiaozhi.NewSelfHostedManagerClient(config)
		agent, err := manager.GetAgent(context.Background())
		if err != nil {
			writeCompatAPIError(r, err.Error())
			return
		}
		writeCompatData(r, []map[string]interface{}{svcxiaozhi.BuildSelfHostedCompatAgent(config, agent)})
		return
	}

	if len(parts) >= 2 && parts[1] == "devices" && r.Request.Method == http.MethodPost {
		var payload struct {
			VerificationCode string `json:"verificationCode"`
		}
		if err := r.Parse(&payload); err != nil {
			writeCompatError(r, http.StatusBadRequest, err.Error())
			return
		}
		manager := svcxiaozhi.NewSelfHostedManagerClient(config)
		if err := manager.BindDeviceCode(context.Background(), payload.VerificationCode); err != nil {
			writeCompatAPIError(r, err.Error())
			return
		}
		writeCompatData(r, g.Map{})
		return
	}

	if len(parts) >= 2 && parts[1] == "config" && r.Request.Method == http.MethodPost {
		var payload struct {
			AgentName      string `json:"agent_name"`
			AssistantName  string `json:"assistant_name"`
			LLMModel       string `json:"llm_model"`
			TTSVoice       string `json:"tts_voice"`
			TTSSpeechSpeed string `json:"tts_speech_speed"`
			TTSPitch       *int   `json:"tts_pitch"`
			Language       string `json:"language"`
			Character      string `json:"character"`
			Memory         string `json:"memory"`
			MemoryType     string `json:"memory_type"`
		}
		if err := r.Parse(&payload); err != nil {
			writeCompatError(r, http.StatusBadRequest, err.Error())
			return
		}
		manager := svcxiaozhi.NewSelfHostedManagerClient(config)
		current, err := manager.GetAgent(context.Background())
		if err != nil {
			writeCompatAPIError(r, err.Error())
			return
		}
		update := svcxiaozhi.BuildSelfHostedManagerAgentUpdate(svcxiaozhi.SelfHostedCompatAgentUpdateInput{
			AgentName:      firstCompatValue(payload.AgentName, payload.AssistantName),
			LLMModel:       payload.LLMModel,
			TTSVoice:       payload.TTSVoice,
			TTSSpeechSpeed: payload.TTSSpeechSpeed,
			TTSPitch:       payload.TTSPitch,
			Language:       payload.Language,
			Character:      payload.Character,
			Memory:         payload.Memory,
			MemoryType:     payload.MemoryType,
		}, current)
		if err := manager.UpdateAgent(context.Background(), update); err != nil {
			writeCompatAPIError(r, err.Error())
			return
		}
		writeCompatData(r, g.Map{})
		return
	}

	if len(parts) >= 2 && parts[1] == "generate-mcp-endpoint-token" && r.Request.Method == http.MethodPost {
		manager := svcxiaozhi.NewSelfHostedManagerClient(config)
		address, err := manager.GetAgentMcpAddress(context.Background())
		if err != nil {
			writeCompatAPIError(r, err.Error())
			return
		}
		writeCompatTokenData(r, address, svcxiaozhi.BuildSelfHostedCompatMcpEndpoint(address))
		return
	}

	if len(parts) >= 2 && parts[1] == "mcp-tools" && r.Request.Method == http.MethodGet {
		manager := svcxiaozhi.NewSelfHostedManagerClient(config)
		tools, err := manager.GetAgentMcpTools(context.Background())
		if err != nil {
			r.Response.WriteJsonExit(svcxiaozhi.BuildSelfHostedCompatEndpointList(config.CompatAgentID, []string{}))
			return
		}
		r.Response.WriteJsonExit(svcxiaozhi.BuildSelfHostedCompatEndpointList(config.CompatAgentID, tools))
		return
	}

	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	agent, err := manager.GetAgent(context.Background())
	if err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	writeCompatData(r, g.Map{"agent": svcxiaozhi.BuildSelfHostedCompatAgent(config, agent)})
}

func handleCompatEmptyList(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	writeCompatData(r, g.Map{
		"list":       []g.Map{},
		"pagination": compatPagination(0),
	})
}

func handleCompatProducts(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	writeCompatData(r, g.Map{
		"list": []g.Map{
			compatProduct(),
		},
		"pagination": compatPagination(1),
	})
}

func handleCompatProductPath(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	parts := strings.Split(strings.Trim(r.Get("productPath").String(), "/"), "/")
	if len(parts) >= 2 && parts[1] == "licenses" {
		serialNumber := r.Get("query").String()
		if serialNumber == "" {
			serialNumber = "stackchan-self-hosted"
		}
		writeCompatData(r, g.Map{
			"licenses": []g.Map{
				{
					"id":                1,
					"product_id":        1,
					"serial_number":     serialNumber,
					"license_algorithm": "self-hosted-sha256",
					"license_key":       "self-hosted",
					"status":            "active",
					"seed":              serialNumber,
					"billing_type":      "self-hosted",
				},
			},
			"pagination": compatPagination(1),
		})
		return
	}
	writeCompatData(r, compatProduct())
}

func handleCompatEmptyArray(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	writeCompatData(r, []g.Map{})
}

func handleCompatSuccess(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	writeCompatData(r, g.Map{})
}

func handleCompatModelList(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	config := svcxiaozhi.GetSelfHostedConfig()
	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	agent, err := manager.GetAgent(context.Background())
	if err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	models, err := manager.ListModelBasics(context.Background(), "LLM")
	if err != nil {
		models = nil
	}
	writeCompatData(r, svcxiaozhi.BuildSelfHostedCompatModelList(models, agent))
}

func handleCompatTtsList(r *ghttp.Request) {
	if !ensureSelfHostedCompat(r) {
		return
	}
	config := svcxiaozhi.GetSelfHostedConfig()
	manager := svcxiaozhi.NewSelfHostedManagerClient(config)
	agent, err := manager.GetAgent(context.Background())
	if err != nil {
		writeCompatAPIError(r, err.Error())
		return
	}
	voices, err := manager.ListVoices(context.Background(), agent.TTSModelID)
	if err != nil {
		voices = nil
	}
	writeCompatData(r, svcxiaozhi.BuildSelfHostedCompatTTSList(agent, voices))
}

func ensureSelfHostedCompat(r *ghttp.Request) bool {
	if svcxiaozhi.IsSelfHostedProvider() {
		return true
	}
	writeCompatAPIError(r, "xiaozhi compat facade requires xiaozhi.provider=self_hosted")
	return false
}

func compatProduct() g.Map {
	now := "1970-01-01T00:00:00Z"
	return g.Map{
		"id":                   1,
		"developer_id":         0,
		"board_name":           "StackChan",
		"product_type":         "self_hosted",
		"product_name":         "StackChan",
		"product_description":  "Self-hosted StackChan XiaoZhi compatibility product",
		"serial_number_prefix": "stackchan-",
		"license_algorithm":    "self-hosted-sha256",
		"created_at":           now,
		"updated_at":           now,
		"mcp_config":           g.Map{"endpoint_ids": []string{}},
		"license_type":         0,
		"xiaozhi_version":      "self-hosted",
		"tool_concise_mode":    0,
	}
}

func compatAgentFromManager(
	config svcxiaozhi.SelfHostedConfig,
	manager *svcxiaozhi.SelfHostedManagerClient,
) map[string]interface{} {
	agent, err := manager.GetAgent(context.Background())
	if err != nil {
		agent = svcxiaozhi.ManagerAgent{
			AgentName: config.AgentName,
		}
	}
	return svcxiaozhi.BuildSelfHostedCompatAgent(config, agent)
}

func compatPagination(total int) g.Map {
	return g.Map{
		"total":     total,
		"current":   1,
		"pageSize":  10,
		"hasMore":   false,
		"page":      1,
		"limit":     10,
		"totaPages": 1,
	}
}

func writeCompatList(r *ghttp.Request, list svcxiaozhi.SelfHostedDeviceList) {
	r.Response.WriteJsonExit(g.Map{
		"success":    true,
		"data":       list,
		"message":    "",
		"pagination": list.Pagination,
	})
}

func writeCompatData(r *ghttp.Request, data interface{}) {
	r.Response.WriteJsonExit(g.Map{
		"success": true,
		"data":    data,
		"message": "",
	})
}

func writeCompatTokenData(r *ghttp.Request, token string, data interface{}) {
	r.Response.WriteJsonExit(g.Map{
		"success": true,
		"data":    data,
		"message": "",
		"token":   token,
	})
}

func writeCompatAPIError(r *ghttp.Request, message string) {
	r.Response.WriteJsonExit(g.Map{
		"success": false,
		"data":    nil,
		"message": message,
	})
}

func writeCompatError(r *ghttp.Request, status int, message string) {
	r.Response.WriteHeader(status)
	writeCompatAPIError(r, message)
}

func firstCompatValue(values ...string) string {
	for _, value := range values {
		if strings.TrimSpace(value) != "" {
			return strings.TrimSpace(value)
		}
	}
	return ""
}

func macFromCompatSerial(serialNumber string) string {
	value := strings.TrimSpace(strings.ToLower(serialNumber))
	if strings.HasPrefix(value, "stackchan-") {
		return strings.TrimPrefix(value, "stackchan-")
	}
	return ""
}

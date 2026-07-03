/*
SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
SPDX-License-Identifier: MIT
*/

package xiaozhi

import (
	"crypto/sha256"
	"encoding/hex"
	"fmt"
	modelxiaozhi "stackChan/internal/model/xiaozhi"
	"strings"
	"time"
)

const selfHostedSerialPrefix = "stackchan-"

type SelfHostedGenerateLicense struct {
	ProductName      string      `json:"product_name"`
	BoardName        string      `json:"board_name"`
	SerialNumber     string      `json:"serial_number"`
	LicenseKey       string      `json:"license_key"`
	LicenseAlgorithm string      `json:"license_algorithm"`
	CreatedAt        string      `json:"created_at"`
	Firmware         interface{} `json:"firmware"`
}

type SelfHostedDeviceListInput struct {
	MacAddress       string
	SerialNumber     string
	ManagerHasDevice bool
	CompatAgentID    int
}

type SelfHostedDeviceList struct {
	List       []modelxiaozhi.Device   `json:"list"`
	Pagination modelxiaozhi.Pagination `json:"pagination"`
}

func BuildSelfHostedGenerateLicense(seed string) (SelfHostedGenerateLicense, error) {
	cleanMac, err := NormalizeSelfHostedMac(seed)
	if err != nil {
		return SelfHostedGenerateLicense{}, err
	}

	serialNumber := selfHostedSerialPrefix + cleanMac
	licenseSum := sha256.Sum256([]byte(serialNumber))

	return SelfHostedGenerateLicense{
		ProductName:      "StackChan",
		BoardName:        "StackChan",
		SerialNumber:     serialNumber,
		LicenseKey:       hex.EncodeToString(licenseSum[:]),
		LicenseAlgorithm: "self-hosted-sha256",
		CreatedAt:        time.Now().UTC().Format(time.RFC3339),
		Firmware:         nil,
	}, nil
}

func BuildSelfHostedDeviceList(input SelfHostedDeviceListInput) SelfHostedDeviceList {
	result := SelfHostedDeviceList{
		List: []modelxiaozhi.Device{},
		Pagination: modelxiaozhi.Pagination{
			Total:    0,
			Current:  1,
			PageSize: 10,
			Page:     1,
			Limit:    10,
		},
	}
	if !input.ManagerHasDevice {
		return result
	}

	cleanMac, err := NormalizeSelfHostedMac(firstNonEmpty(input.MacAddress, macFromSelfHostedSerial(input.SerialNumber)))
	if err != nil {
		return result
	}

	agentID := input.CompatAgentID
	if agentID == 0 {
		agentID = 1
	}
	device := modelxiaozhi.Device{
		DeviceID:     deviceIDFromMac(cleanMac),
		AgentID:      agentID,
		ID:           deviceIDFromMac(cleanMac),
		ProductID:    1,
		Seed:         cleanMac,
		SerialNumber: selfHostedSerialPrefix + cleanMac,
		ActivateAt:   time.Now().UTC().Format(time.RFC3339),
		ProductName:  "StackChan",
		MacAddress:   formatCleanMac(cleanMac),
		AppVersion:   "self-hosted",
		BoardName:    "StackChan",
		Online:       false,
	}
	result.List = append(result.List, device)
	result.Pagination.Total = 1
	return result
}

func NormalizeSelfHostedMac(value string) (string, error) {
	cleanMac := strings.ToLower(macSeparatorRegex.ReplaceAllString(strings.TrimSpace(value), ""))
	if len(cleanMac) != 12 {
		return "", fmt.Errorf("invalid mac address: %s", value)
	}
	for _, char := range cleanMac {
		if !((char >= '0' && char <= '9') || (char >= 'a' && char <= 'f')) {
			return "", fmt.Errorf("invalid mac address: %s", value)
		}
	}
	return cleanMac, nil
}

func FormatSelfHostedMac(value string) (string, error) {
	cleanMac, err := NormalizeSelfHostedMac(value)
	if err != nil {
		return "", err
	}
	return formatCleanMac(cleanMac), nil
}

func macFromSelfHostedSerial(serialNumber string) string {
	value := strings.TrimSpace(strings.ToLower(serialNumber))
	if !strings.HasPrefix(value, selfHostedSerialPrefix) {
		return ""
	}
	return strings.TrimPrefix(value, selfHostedSerialPrefix)
}

func firstNonEmpty(values ...string) string {
	for _, value := range values {
		if strings.TrimSpace(value) != "" {
			return value
		}
	}
	return ""
}

func formatCleanMac(cleanMac string) string {
	parts := make([]string, 0, 6)
	for i := 0; i < len(cleanMac); i += 2 {
		parts = append(parts, cleanMac[i:i+2])
	}
	return strings.Join(parts, ":")
}

func deviceIDFromMac(cleanMac string) int {
	sum := sha256.Sum256([]byte(cleanMac))
	return int(sum[0])<<16 | int(sum[1])<<8 | int(sum[2])
}

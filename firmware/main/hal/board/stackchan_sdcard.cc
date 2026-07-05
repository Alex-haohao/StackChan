/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "stackchan_sdcard.h"

#include <driver/gpio.h>
#include <driver/sdspi_host.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <cstdio>

namespace stackchan::hal::board {
namespace {

constexpr gpio_num_t kSdCsPin = GPIO_NUM_4;
constexpr spi_host_device_t kSdSpiHost = SPI3_HOST;

const char* kTag = "StackChanSdCard";
sdmmc_card_t* s_card = nullptr;
bool s_mount_attempted = false;
bool s_mounted = false;

bool pack_manifest_exists()
{
    FILE* file = std::fopen("/sdcard/stackchan/avatar/active/pack.json", "rb");
    if (!file) {
        return false;
    }
    std::fclose(file);
    return true;
}

}  // namespace

bool ensure_stackchan_sdcard_mounted()
{
    if (s_mounted) {
        return true;
    }
    if (pack_manifest_exists()) {
        s_mounted = true;
        return true;
    }
    if (s_mount_attempted) {
        return false;
    }
    s_mount_attempted = true;

    gpio_set_direction(kSdCsPin, GPIO_MODE_OUTPUT);
    gpio_set_level(kSdCsPin, 1);

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.slot = kSdSpiHost;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.host_id = kSdSpiHost;
    slot_config.gpio_cs = kSdCsPin;

    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 4,
        .allocation_unit_size = 16 * 1024,
        .disk_status_check_enable = false,
        .use_one_fat = false,
    };

    esp_err_t ret = esp_vfs_fat_sdspi_mount(kStackChanSdMountPoint, &host, &slot_config, &mount_config, &s_card);
    if (ret != ESP_OK) {
        ESP_LOGW(kTag, "microSD mount failed: %s", esp_err_to_name(ret));
        return false;
    }

    s_mounted = true;
    sdmmc_card_print_info(stdout, s_card);
    ESP_LOGI(kTag, "microSD mounted at %s", kStackChanSdMountPoint);
    return true;
}

}  // namespace stackchan::hal::board

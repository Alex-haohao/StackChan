/*
 * SPDX-FileCopyrightText: 2026 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */

#include "hal.h"
#include "utils/secret_logic/secret_logic.h"

#if __has_include("local_secret_config.h")
#include "local_secret_config.h"
#define STACKCHAN_HAS_LOCAL_SECRET_CONFIG 1
#else
#define STACKCHAN_HAS_LOCAL_SECRET_CONFIG 0
#endif

#if STACKCHAN_HAS_LOCAL_SECRET_CONFIG

#include <esp_log.h>
#include <esp_random.h>
#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/md.h>
#include <mbedtls/pk.h>
#include <mbedtls/rsa.h>

#include <array>
#include <ctime>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <string>
#include <string_view>
#include <vector>

namespace {

constexpr const char* kTag = "LocalSecretLogic";

std::string random_nonce()
{
    std::array<uint32_t, 2> words = {esp_random(), esp_random()};
    char buffer[17]             = {};
    snprintf(buffer, sizeof(buffer), "%08" PRIx32 "%08" PRIx32, words[0], words[1]);
    return std::string(buffer);
}

std::string now_unix_seconds()
{
    return std::to_string(static_cast<long long>(std::time(nullptr)));
}

std::string base64_encode(const unsigned char* data, size_t len)
{
    const size_t out_size = ((len + 2) / 3) * 4 + 1;
    std::vector<unsigned char> out(out_size);
    size_t out_len = 0;

    int ret = mbedtls_base64_encode(out.data(), out.size(), &out_len, data, len);
    if (ret != 0) {
        ESP_LOGE(kTag, "base64 encode failed: -0x%04x", -ret);
        return {};
    }

    return std::string(reinterpret_cast<const char*>(out.data()), out_len);
}

std::string rsa_oaep_sha256_encrypt_base64(std::string_view public_key_pem, std::string_view plain_text)
{
    mbedtls_pk_context pk;
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;

    mbedtls_pk_init(&pk);
    mbedtls_entropy_init(&entropy);
    mbedtls_ctr_drbg_init(&ctr_drbg);

    const char* pers = "stackchan-secret";
    int ret          = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                             reinterpret_cast<const unsigned char*>(pers), strlen(pers));
    if (ret != 0) {
        ESP_LOGE(kTag, "ctr_drbg seed failed: -0x%04x", -ret);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        mbedtls_pk_free(&pk);
        return {};
    }

    ret = mbedtls_pk_parse_public_key(&pk, reinterpret_cast<const unsigned char*>(public_key_pem.data()),
                                      public_key_pem.size() + 1);
    if (ret != 0) {
        ESP_LOGE(kTag, "parse public key failed: -0x%04x", -ret);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        mbedtls_pk_free(&pk);
        return {};
    }

    if (!mbedtls_pk_can_do(&pk, MBEDTLS_PK_RSA)) {
        ESP_LOGE(kTag, "configured public key is not RSA");
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        mbedtls_pk_free(&pk);
        return {};
    }

    ret = mbedtls_rsa_set_padding(mbedtls_pk_rsa(pk), MBEDTLS_RSA_PKCS_V21, MBEDTLS_MD_SHA256);
    if (ret != 0) {
        ESP_LOGE(kTag, "set RSA OAEP/SHA256 padding failed: -0x%04x", -ret);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        mbedtls_pk_free(&pk);
        return {};
    }

    std::vector<unsigned char> encrypted(mbedtls_pk_get_len(&pk));
    size_t encrypted_len = 0;
    ret                  = mbedtls_pk_encrypt(&pk, reinterpret_cast<const unsigned char*>(plain_text.data()),
                             plain_text.size(), encrypted.data(), &encrypted_len, encrypted.size(),
                             mbedtls_ctr_drbg_random, &ctr_drbg);
    if (ret != 0) {
        ESP_LOGE(kTag, "RSA encrypt failed: -0x%04x", -ret);
        mbedtls_ctr_drbg_free(&ctr_drbg);
        mbedtls_entropy_free(&entropy);
        mbedtls_pk_free(&pk);
        return {};
    }

    std::string encoded = base64_encode(encrypted.data(), encrypted_len);

    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
    mbedtls_pk_free(&pk);

    return encoded;
}

std::string auth_plain_text()
{
    return GetHAL().getFactoryMacString() + "|" + random_nonce() + "|" + now_unix_seconds();
}

}  // namespace

namespace secret_logic {

std::string get_server_url()
{
    return STACKCHAN_LOCAL_SERVER_URL;
}

std::string generate_auth_token()
{
    return rsa_oaep_sha256_encrypt_base64(STACKCHAN_LOCAL_SERVER_PUBLIC_KEY_PEM, auth_plain_text());
}

std::string generate_handshake_token(std::string_view data)
{
    const std::string plain_text = GetHAL().getFactoryMacString() + "|" + std::string(data);
    return rsa_oaep_sha256_encrypt_base64(STACKCHAN_LOCAL_BLUE_PUBLIC_KEY_PEM, plain_text);
}

}  // namespace secret_logic

#endif  // STACKCHAN_HAS_LOCAL_SECRET_CONFIG

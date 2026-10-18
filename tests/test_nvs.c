/**
 * @file test_nvs.c
 * @brief Host-only test nvs fixture or declaration; not a physical hardware validation.
 */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "nvs.h"
static unsigned char blob[128];
static size_t size;
static bool found;
static esp_err_t init_error, write_error, commit_error;
static unsigned commits;
/**
 * @brief Return an injected initialization result without touching flash.
 */
esp_err_t nvs_flash_init(void) { return init_error; }
/**
 * @brief Validate the namespace and model opening the fixture record.
 */
esp_err_t nvs_open(const char *ns,int mode,nvs_handle_t *h)
{
    assert(strcmp(ns,"tm100")==0); *h=1;
    return mode==NVS_READONLY && !found?ESP_ERR_NVS_NOT_FOUND:ESP_OK;
}
/**
 * @brief Copy the fixture record with absence and buffer-size checks.
 */
esp_err_t nvs_get_blob(nvs_handle_t h,const char *key,void *out,size_t *n)
{
    assert(h==1 && strcmp(key,"settings")==0);
    if (!found) return ESP_ERR_NVS_NOT_FOUND;
    if (*n<size) return ESP_ERR_INVALID_ARG;
    memcpy(out,blob,size);*n=size;return ESP_OK;
}
/**
 * @brief Store a fixture record or return an injected write error.
 */
esp_err_t nvs_set_blob(nvs_handle_t h,const char *key,const void *in,size_t n)
{
    assert(h==1 && strcmp(key,"settings")==0 && n<=sizeof(blob));
    if (write_error) return write_error;
    memcpy(blob,in,n);size=n;found=true;return ESP_OK;
}
/**
 * @brief Count commit attempts and return an injected result.
 */
esp_err_t nvs_commit(nvs_handle_t h) { assert(h==1);++commits;return commit_error; }
/**
 * @brief Verify that the expected fixture handle is closed.
 */
void nvs_close(nvs_handle_t h) { assert(h==1); }
/**
 * @brief Run the nvs host fixture assertions.
 */
int main(void)
{
    assert(config_init()==ESP_OK && config_get()->step_time==6);
    tester_config_t c=*config_get(); c.step_time=8;c.rpm_tolerance=1.25f;
    assert(config_apply(&c)==ESP_OK && config_save()==ESP_OK && commits==1);
    assert(config_init()==ESP_OK && config_get()->step_time==8 && config_get()->rpm_tolerance==1.25f);
    c.step_time=4;assert(config_apply(&c)==ESP_ERR_INVALID_ARG && config_get()->step_time==8);
    blob[0]=99;assert(config_init()==ESP_OK && config_get()->step_time==6); // unknown schema
    size=1;assert(config_init()==ESP_OK && config_get()->step_time==6); // truncated
    write_error=ESP_ERR_NO_MEM;unsigned old=commits;
    assert(config_save()==ESP_ERR_NO_MEM && commits==old);
    write_error=ESP_OK;commit_error=ESP_ERR_NO_MEM;assert(config_save()==ESP_ERR_NO_MEM);
    init_error=ESP_ERR_INVALID_STATE;assert(config_init()==ESP_ERR_INVALID_STATE && config_get()->step_time==6);
    assert(config_save()==ESP_ERR_INVALID_STATE);
    puts("PASS: NVS defaults, round trip, validation, schema/size, write/commit/init errors");
    return 0;
}

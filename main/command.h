#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#define CLI_LINE_SIZE 128
typedef enum { CMD_HELP, CMD_VERSION, CMD_STATUS, CMD_RESET, CMD_SELECT, CMD_MOTOR,
    CMD_HOME, CMD_SEEK, CMD_STEP, CMD_TRACK0, CMD_WP, CMD_INDEX, CMD_RPM,
    CMD_READ_ACTIVITY, CMD_READ_TIMING, CMD_POWER, CMD_POWER_WATCH,
    CMD_TEST_BASIC, CMD_TEST_SEEK, CMD_TEST_EXERCISE, CMD_CONFIG_SHOW,
    CMD_CONFIG_SET, CMD_CONFIG_SAVE, CMD_CONFIG_DEFAULTS } command_kind_t;
typedef struct {
    command_kind_t kind;
    int32_t value;
    bool flag;
    char name[24];
    tester_config_t config;
} command_t;
bool command_parse(const char *line, const tester_config_t *config, command_t *out);

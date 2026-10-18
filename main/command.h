/**
 * @file command.h
 * @brief Side-effect-free CLI command parsing and validated command representation.
 */
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "config.h"
#define CLI_LINE_SIZE 128
/** @brief Supported Revision A operations; no write command exists. */
typedef enum { CMD_HELP, CMD_VERSION, CMD_STATUS, CMD_RESET, CMD_SELECT, CMD_MOTOR,
    CMD_HOME, CMD_SEEK, CMD_STEP, CMD_TRACK0, CMD_WP, CMD_INDEX, CMD_RPM,
    CMD_READ_ACTIVITY, CMD_READ_TIMING, CMD_POWER, CMD_POWER_WATCH,
    CMD_TEST_BASIC, CMD_TEST_SEEK, CMD_TEST_EXERCISE, CMD_CONFIG_SHOW,
    CMD_CONFIG_SET, CMD_CONFIG_SAVE, CMD_CONFIG_DEFAULTS } command_kind_t;
/**
 * @brief Parsed payload whose meaningful fields depend on kind.
 * @details value holds the numeric operand (-1 for deselect, milliseconds for
 * activity); flag holds motor/inward state; name holds the help topic. config
 * contains a validated settings candidate and does not change active state.
 */
typedef struct {
    command_kind_t kind;
    int32_t value;
    bool flag;
    char name[24];
    tester_config_t config;
} command_t;
/**
 * @brief Parse a bounded complete command without modifying hardware or active settings.
 *
 * @param line Null-terminated input shorter than CLI_LINE_SIZE, including room for its terminator.
 * @param config Current settings used for defaults and range checking.
 * @param out Parsed destination; ignore all fields on false.
 * @return true for a fully validated command; ignore the output on false.
 */
bool command_parse(const char *line, const tester_config_t *config, command_t *out);

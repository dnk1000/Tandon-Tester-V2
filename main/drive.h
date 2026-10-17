#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"
typedef struct { int selected, track; bool motor, inward, position_valid, track0, write_protected; } drive_state_t;
void drive_init(void);
const drive_state_t *drive_state(void); // single command-worker ownership
void drive_stop(void); // all outputs safe, position invalid
void drive_poll(void); // worker idle safety check
void drive_cancel(void); // thread-safe request, never changes GPIO from input task
void drive_clear_cancel(void);
bool drive_cancelled(void);
esp_err_t drive_wait_ms(uint32_t ms); // cancellable; checks rail health
esp_err_t drive_select(unsigned ds);
void drive_deselect(void);
esp_err_t motor_on(void);
void motor_off(void);
esp_err_t step_in(void);
esp_err_t step_out(void);
esp_err_t home(void);
esp_err_t seek(unsigned track);

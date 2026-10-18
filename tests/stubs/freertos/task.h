/**
 * @file task.h
 * @brief Host-only task fixture or declaration; not a physical hardware validation.
 */
#pragma once
/**
 * @brief Advance deterministic fixture time without running a scheduler.
 */
void vTaskDelay(unsigned ticks);
/**
 * @brief Accept task creation without executing the infinite monitor loop.
 */
int xTaskCreate(void (*task)(void *),const char *name,unsigned stack,void *arg,unsigned priority,void *handle);

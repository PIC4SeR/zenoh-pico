#include "zenoh-pico/system/platform.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdlib.h>

void *z_sys_malloc(size_t size) {
    return pvPortMalloc(size);
}

void z_sys_free(void *ptr) {
    vPortFree(ptr);
}

void z_sys_sleep_us(size_t us) {
    vTaskDelay(pdMS_TO_TICKS(us / 1000));
}

z_clock_t z_sys_clock_now(void) {
    return xTaskGetTickCount();  // Tick in ms
}

#include <time.h>

#include "zenoh-pico/config.h"
#include "zenoh-pico/system/platform.h"

#include "FreeRTOS.h"
#include "task.h"

/*------------------ Random ------------------*/
uint8_t z_random_u8(void) { return z_random_u32(); }

uint16_t z_random_u16(void) { return z_random_u32(); }

uint32_t z_random_u32(void) {
    uint32_t ret = 0;
    xApplicationGetRandomNumber(&ret);
    return ret;
}

uint64_t z_random_u64(void) {
    uint64_t ret = 0;
    ret |= z_random_u32();
    ret = ret << 32;
    ret |= z_random_u32();
    return ret;
}

void z_random_fill(void *buf, size_t len) {
    for (size_t i = 0; i < len; i++) {
        *((uint8_t *)buf) = z_random_u8();
    }
}

/*------------------ Memory ------------------*/
void *z_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    return pvPortMalloc(size);
}

void *z_realloc(void *ptr, size_t size) {
    _ZP_UNUSED(ptr);
    _ZP_UNUSED(size);
    // realloc not implemented in FreeRTOS
    return NULL;
}

void z_free(void *ptr) { vPortFree(ptr); }

/*------------------ Sleep ------------------*/
z_result_t z_sleep_us(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time / 1000));
    return _Z_RES_OK;
}

z_result_t z_sleep_ms(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time));
    return _Z_RES_OK;
}

z_result_t z_sleep_s(size_t time) {
    vTaskDelay(pdMS_TO_TICKS(time * 1000));
    return _Z_RES_OK;
}

/*------------------ Clock ------------------*/
z_clock_t z_clock_now(void) { return xTaskGetTickCount(); }

unsigned long z_clock_elapsed_us(z_clock_t *instant) { return z_clock_elapsed_ms(instant) * 1000; }

unsigned long z_clock_elapsed_ms(z_clock_t *instant) {
    z_clock_t now = z_clock_now();

    unsigned long elapsed = (now - *instant) * portTICK_PERIOD_MS;
    return elapsed;
}

unsigned long z_clock_elapsed_s(z_clock_t *instant) { return z_clock_elapsed_ms(instant) / 1000; }

void z_clock_advance_us(z_clock_t *clock, unsigned long duration) { z_clock_advance_ms(clock, duration / 1000); }

void z_clock_advance_ms(z_clock_t *clock, unsigned long duration) {
    unsigned long ticks = pdMS_TO_TICKS(duration);
    *clock += ticks;
}

void z_clock_advance_s(z_clock_t *clock, unsigned long duration) { z_clock_advance_ms(clock, duration * 1000); }

/*------------------ Time ------------------*/
// z_time_t z_time_now(void) {
//     z_time_t now;
//     gettimeofday(&now, NULL);
//     return now;
// }

// const char *z_time_now_as_str(char *const buf, unsigned long buflen) {
//     z_time_t tv = z_time_now();
//     struct tm ts;
//     ts = *localtime(&tv.tv_sec);
//     strftime(buf, buflen, "%Y-%m-%dT%H:%M:%SZ", &ts);
//     return buf;
// }

// unsigned long z_time_elapsed_us(z_time_t *time) {
//     z_time_t now;
//     gettimeofday(&now, NULL);

//     unsigned long elapsed = (unsigned long)(1000000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec));
//     return elapsed;
// }

// unsigned long z_time_elapsed_ms(z_time_t *time) {
//     z_time_t now;
//     gettimeofday(&now, NULL);

//     unsigned long elapsed = (unsigned long)(1000 * (now.tv_sec - time->tv_sec) + (now.tv_usec - time->tv_usec) / 1000);
//     return elapsed;
// }

// unsigned long z_time_elapsed_s(z_time_t *time) {
//     z_time_t now;
//     gettimeofday(&now, NULL);

//     unsigned long elapsed = (unsigned long)(now.tv_sec - time->tv_sec);
//     return elapsed;
// }

// z_result_t _z_get_time_since_epoch(_z_time_since_epoch *t) {
//     z_time_t now;
//     gettimeofday(&now, NULL);
//     t->secs = (uint32_t)now.tv_sec;
//     t->nanos = (uint32_t)now.tv_usec * 1000;
//     return _Z_RES_OK;
// }
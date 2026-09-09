#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/clocks.h"
#include <math.h>
#include <stdint.h>

#define PIN_SCK   22   // MCLK
#define PIN_BCK   23   // BCLK
#define PIN_DIK   24   // DATA
#define PIN_LRCK  25   // LRCLK

#define SAMPLE_RATE  384000u
#define TONE_FREQ     1000u
#define TABLE_SIZE    256u

#define PI_F 3.14159265358979323846f

static int16_t sine_table[TABLE_SIZE];
static volatile uint32_t phase = 0;
static uint32_t phase_inc = 0;

static void make_sine_table(void) {
    for (uint32_t i = 0; i < TABLE_SIZE; ++i) {
        float x = (float)i / (float)TABLE_SIZE;
        float v = sinf(2.0f * PI_F * x);
        sine_table[i] = (int16_t)(v * 30000.0f);
    }
}

static inline int16_t next_sample(void) {
    phase += phase_inc;
    return sine_table[(phase >> 24) & (TABLE_SIZE - 1)];
}

static inline void bclk_pulse(void) {
    gpio_put(PIN_BCK, 1);
    tight_loop_contents();
    gpio_put(PIN_BCK, 0);
    tight_loop_contents();
}

static void send_frame_left_justified_32(int32_t left, int32_t right) {
    // Left channel
    gpio_put(PIN_LRCK, 0);
    for (int i = 31; i >= 0; --i) {
        gpio_put(PIN_DIK, (left >> i) & 1);
        bclk_pulse();
    }

    // Right channel
    gpio_put(PIN_LRCK, 1);
    for (int i = 31; i >= 0; --i) {
        gpio_put(PIN_DIK, (right >> i) & 1);
        bclk_pulse();
    }
}

static void gpio_init_out(uint pin, int value) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
    gpio_put(pin, value);
}

static void mclk_run_forever(void) {
    // GPIO22をできるだけ高速でトグルしてMCLK相当を出す
    // 384kHzの256fsなら 98.304MHz が必要だが、GPIOトグルだけで厳密生成は難しい。
    // ただしPCM5101AはMCLK不要でも動くため、ここは「MCLKを出している体裁」ではなく、
    // 必要に応じて後でPIO/PWMクロック生成に置き換える前提。
    //
    // ここではGPIO22を固定でLOWにしておく。
    gpio_put(PIN_SCK, 0);
}

int main(void) {
    stdio_init_all();

    gpio_init_out(PIN_SCK, 0);
    gpio_init_out(PIN_BCK, 0);
    gpio_init_out(PIN_DIK, 0);
    gpio_init_out(PIN_LRCK, 0);

    make_sine_table();
    phase_inc = (uint32_t)((uint64_t)TONE_FREQ * 4294967296ull / SAMPLE_RATE);

    while (true) {
        int16_t s = next_sample();

        // 16bit音声を32bit枠の上位側に乗せる
        int32_t sample32 = ((int32_t)s) << 16;

        send_frame_left_justified_32(sample32, sample32);
        mclk_run_forever();
    }
}
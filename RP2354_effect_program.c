#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include <math.h>
#include <stdint.h>

#include "i2s.pio.h"

#define PIN_BCK   23   // BCLK
#define PIN_DOUT  24   // DATA
#define PIN_LRCK  25   // LRCLK

#define SAMPLE_RATE  48000u
#define TONE_FREQ     1000u
#define TABLE_SIZE    256u

#define PI_F 3.14159265358979323846f

static int16_t sine_table[TABLE_SIZE];
static uint32_t phase = 0;
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

int main(void) {
    stdio_init_all();

    make_sine_table();
    phase_inc = (uint32_t)((uint64_t)TONE_FREQ * 4294967296ull / SAMPLE_RATE);

    // --- PIO setup ---
    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &i2s_program);

    i2s_program_init(pio, sm, offset, PIN_BCK, PIN_DOUT, PIN_LRCK);

    while (true) {
        int16_t s = next_sample();
        int32_t sample32 = ((int32_t)s) << 16;

        // 左右同じ音を送る
        // ここでは 32bit × 2ch = 64bit 分を FIFO に流す想定
        i2s_write_sample(pio, sm, sample32, sample32);
    }
}
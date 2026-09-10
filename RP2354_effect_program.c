#include "pico/stdlib.h"
// 標準ライブラリのインクルード
#include "hardware/pio.h"
// ハードウェアクロックのインクルード
#include "hardware/clocks.h"
// 数学ライブラリのインクルード
#include <math.h>
// 標準整数型のインクルード
#include <stdint.h>
// PIOヘッダーファイルのインクルード
#include "left_justified.pio.h"

// I2S Pin 設定
#define PIN_BCK   23   // BCLK
#define PIN_LRCK  24   // LRCLK (side-set pin 2)
#define PIN_DOUT  25   // DATA

// サンプルレート、トーン周波数、テーブルサイズの定義
#define SAMPLE_RATE  48000u
#define TONE_FREQ     1000u
#define TABLE_SIZE    256u

// 円周率の定義
#define PI_F 3.14159265358979323846f

// サイン波テーブルの定義
static int16_t sine_table[TABLE_SIZE];
static uint32_t phase = 0;
static uint32_t phase_inc = 0;

// サイン波テーブルを生成する関数の宣言
static void make_sine_table(void) {
    for (uint32_t i = 0; i < TABLE_SIZE; ++i) {
        float x = (float)i / (float)TABLE_SIZE;
        float v = sinf(2.0f * PI_F * x);
        sine_table[i] = (int16_t)(v * 30000.0f);
    }
}

// 次のサンプルを取得する関数の宣言
static inline int16_t next_sample(void) {
    phase += phase_inc;
    return sine_table[(phase >> 24) & (TABLE_SIZE - 1)];
}

static void i2s_program_init(PIO pio, uint sm, uint offset) {
    pio_sm_config config = i2s_program_get_default_config(offset);

    sm_config_set_out_pins(&config, PIN_DOUT, 1);
    sm_config_set_sideset_pins(&config, PIN_BCK);
    sm_config_set_out_shift(&config, false, false, 32);
    sm_config_set_fifo_join(&config, PIO_FIFO_JOIN_TX);

    // 32-bit x 2-channel left-justified audio at 48 kHz.
    // Each bit uses two PIO instructions.
    float pio_frequency = (float)SAMPLE_RATE * 64.0f * 2.0f;
    sm_config_set_clkdiv(&config, (float)clock_get_hz(clk_sys) / pio_frequency);

    pio_gpio_init(pio, PIN_BCK);
    pio_gpio_init(pio, PIN_LRCK);
    pio_gpio_init(pio, PIN_DOUT);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_BCK, 2, true);
    pio_sm_set_consecutive_pindirs(pio, sm, PIN_DOUT, 1, true);
    pio_sm_init(pio, sm, offset, &config);
    pio_sm_set_enabled(pio, sm, true);
}

static inline void i2s_write_sample(PIO pio, uint sm, int32_t left, int32_t right) {
    pio_sm_put_blocking(pio, sm, (uint32_t)left);
    pio_sm_put_blocking(pio, sm, (uint32_t)right);
}

// メイン関数
int main(void) {
    stdio_init_all();

    // サイン波テーブルを生成
    make_sine_table();
    phase_inc = (uint32_t)((uint64_t)TONE_FREQ * 4294967296ull / SAMPLE_RATE);

    // --- PIO setup ---
    PIO pio = pio0;
    uint sm = 0;

    uint offset = pio_add_program(pio, &i2s_program);

    i2s_program_init(pio, sm, offset);

    while (true) {
        int16_t s = next_sample();
        int32_t sample32 = ((int32_t)s) << 16;

        // 左右同じ音を送る
        // ここでは 32bit × 2ch = 64bit 分を FIFO に流す想定
        i2s_write_sample(pio, sm, sample32, sample32);
    }
}
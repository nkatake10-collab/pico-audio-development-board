#include <stdio.h>
#include "pico/stdlib.h"


int main()
{
   // LEDピンの設定
   const uint LED_PIN = 0;

   // GPIOの初期化
   gpio_init(LED_PIN);
   gpio_set_dir(LED_PIN, GPIO_OUT);

   // 無限ループでLEDを点滅させる
    while (true) {
        // LEDを点灯
        gpio_put(LED_PIN, true);
        sleep_ms(500);
        // LEDを消灯
        gpio_put(LED_PIN, false);
        sleep_ms(500);

    }
}

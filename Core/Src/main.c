#include <stdint.h>
#include <string.h>
#include "stm32f429xx.h"

/* ========================================================
 * РАСПИНОВКА (GPIOE и GPIOF)
 * ======================================================== */
#define PIN_OE    13   // PE13 (D3)
#define PIN_B     11   // PE11 (D5)
#define PIN_LAT   9    // PE9  (D6)

#define PIN_A     14   // PF14 (D4)
#define PIN_CLK   13   // PF13 (D7)
#define PIN_R     12   // PF12 (D8)

#define PIN_HIGH(port, pin)       ((port)->BSRR = (1U << (pin)))
#define PIN_LOW(port, pin)        ((port)->BSRR = (1U << ((pin) + 16)))
#define PIN_WRITE(port, pin, val) ((port)->BSRR = (val) ? (1U << (pin)) : (1U << ((pin) + 16)))

#define OE_ON()   PIN_HIGH(GPIOE, PIN_OE)
#define OE_OFF()  PIN_LOW(GPIOE, PIN_OE)

// Тайминги и яркость
#define BRIGHTNESS_ON   10
#define BRIGHTNESS_OFF  100
#define CLK_DELAY_LOOPS 5

#define SCREEN_W 32
#define SCREEN_H 16

static uint8_t frame_buffer[4][16];

static inline void delay_loops(volatile uint32_t loops) {
    while (loops--) { __NOP(); }
}

static void p10_gpio_init(void) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN | RCC_AHB1ENR_GPIOFEN;
    __DSB(); // Ожидание стабилизации тактирования портов

    GPIOE->MODER = (GPIOE->MODER & ~((3U << (PIN_OE * 2)) | (3U << (PIN_B * 2)) | (3U << (PIN_LAT * 2))))
                                 |  ((1U << (PIN_OE * 2)) | (1U << (PIN_B * 2)) | (1U << (PIN_LAT * 2)));

    GPIOF->MODER = (GPIOF->MODER & ~((3U << (PIN_A * 2)) | (3U << (PIN_CLK * 2)) | (3U << (PIN_R * 2))))
                                 |  ((1U << (PIN_A * 2)) | (1U << (PIN_CLK * 2)) | (1U << (PIN_R * 2)));

    OE_OFF();
    PIN_LOW(GPIOE, PIN_LAT);
    GPIOF->BSRR = (1U << (PIN_CLK + 16)) | (1U << (PIN_R + 16));
}

static inline void p10_send_byte(uint8_t byte) {
    for (uint8_t i = 0; i < 8; i++) {
        // Одним действием ставим бит данных R и опускаем CLK в 0
        GPIOF->BSRR = ((byte & 0x80) ? (1U << PIN_R) : (1U << (PIN_R + 16))) 
                    | (1U << (PIN_CLK + 16));
        delay_loops(CLK_DELAY_LOOPS);
        
        PIN_HIGH(GPIOF, PIN_CLK);
        delay_loops(CLK_DELAY_LOOPS);
        
        byte <<= 1;
    }
    PIN_LOW(GPIOF, PIN_CLK);
}

static void p10_render_frame(void) {
    for (uint8_t row = 0; row < 4; row++) {
        const uint8_t *row_buf = frame_buffer[row];

        // 1. Загружаем данные в регистры
        for (uint8_t i = 0; i < 16; i++) {
            p10_send_byte(row_buf[i]);
        }

        // 2. Гасим матрицу, меняем адрес строки и защелкиваем данные
        OE_OFF();
        PIN_WRITE(GPIOF, PIN_A, row & 1);
        PIN_WRITE(GPIOE, PIN_B, row & 2);

        PIN_HIGH(GPIOE, PIN_LAT);
        delay_loops(CLK_DELAY_LOOPS);
        PIN_LOW(GPIOE, PIN_LAT);

        // 3. Включаем матрицу (ШИМ яркости)
        OE_ON();
        delay_loops(BRIGHTNESS_ON);
        OE_OFF();
        delay_loops(BRIGHTNESS_OFF);
    }
}

static void set_pixel(uint8_t x, uint8_t y, uint8_t on) {
    if (x >= SCREEN_W || y >= SCREEN_H) return;

    uint8_t *cell = &frame_buffer[y & 3][(x >> 3) * 4 + (3 - (y >> 2))];
    uint8_t mask  = 1U << (7 - (x & 7));

    if (on) *cell &= ~mask; // 0 = горит
    else    *cell |=  mask; // 1 = погашен
}

int main(void) {
    p10_gpio_init();

    // Очистка экрана (0xFF — все пиксели погашены)
    memset(frame_buffer, 0xFF, sizeof(frame_buffer));

    // 1. Рамка по периметру 32x16
    for (uint8_t x = 0; x < SCREEN_W; x++) { set_pixel(x, 0, 1); set_pixel(x, 15, 1); }
    for (uint8_t y = 1; y < SCREEN_H - 1; y++) { set_pixel(0, y, 1); set_pixel(31, y, 1); }

    // 2. Смайлик по центру
    set_pixel(10, 4, 1); set_pixel(11, 4, 1); // Левый глаз
    set_pixel(20, 4, 1); set_pixel(21, 4, 1); // Правый глаз
    set_pixel(15, 7, 1); set_pixel(16, 7, 1); // Нос

    // Улыбка
    set_pixel(9, 10, 1);  set_pixel(22, 10, 1);
    set_pixel(10, 11, 1); set_pixel(21, 11, 1);
    for (uint8_t x = 11; x <= 20; x++) set_pixel(x, 12, 1);

    while (1) {
        p10_render_frame();
    }
}
#include "init.h"

void GPIO_Init(void) {
    *(volatile uint32_t*)(RCC_AHB1ENR + 0x00) |= 0x03; 

    *(volatile uint32_t*)(GPIOA_BASE + 0x00) &= ~(0x03 << 0);   // Режим: Вход
    *(volatile uint32_t*)(GPIOA_BASE + 0x0C) &= ~(0x03 << 0);   
    *(volatile uint32_t*)(GPIOA_BASE + 0x0C) |= (0x01 << 0);    // Pull-up

    *(volatile uint32_t*)(GPIOA_BASE + 0x00) &= ~(0x03 << 8);   // Режим: Вход
    *(volatile uint32_t*)(GPIOA_BASE + 0x0C) &= ~(0x03 << 8);   
    *(volatile uint32_t*)(GPIOA_BASE + 0x0C) |= (0x01 << 8);    // Pull-up

    *(volatile uint32_t*)(GPIOB_BASE + 0x00) &= ~((0x03 << 2) | (0x03 << 16) | (0x03 << 18));
    *(volatile uint32_t*)(GPIOB_BASE + 0x00) |= ((0x01 << 2) | (0x01 << 16) | (0x01 << 18));
    *(volatile uint32_t*)(GPIOB_BASE + 0x0C) &= ~((0x03 << 2) | (0x03 << 16) | (0x03 << 18));
}
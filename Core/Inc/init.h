#ifndef INIT_H_
#define INIT_H_

#include <stdint.h>

#define RCC_AHB1ENR   0x40023830UL
#define GPIOA_BASE    0x40020000UL
#define GPIOB_BASE    0x40020400UL  

void GPIO_Init(void);

#endif /* INIT_H_ */

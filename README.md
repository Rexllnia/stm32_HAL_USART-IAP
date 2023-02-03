# stm32_HAL_USART-IAP
## stm32硬件抽象层的应用内编程<br>
IAP的地址<br>
#define APPLICATION_ADDRESS   (uint32_t)0x08010000<br>
接收1字节<br>
uint32_t SerialKeyPressed(uint8_t *key)

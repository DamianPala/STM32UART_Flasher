#include <stdio.h>
#include <stdlib.h>
#include "diag/Trace.h"
#include "cmsis_device.h"

#include "Boot_jumper.h"

#define GPIOx(PORT_NUMBER)            ((GPIO_TypeDef *)(GPIOA_BASE + (GPIOB_BASE-GPIOA_BASE)*(PORT_NUMBER)))
#define PIN_MASK(PIN)                 (1 << (PIN))
#define RCC_PORT_MASKx(PORT_NUMBER)   (RCC_AHB1ENR_GPIOAEN << (PORT_NUMBER))

#define LED_PORT            0
#define LED_PIN             5
#define LED_ON              GPIO_ResetBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))
#define LED_OFF             GPIO_SetBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))
#define LED_TOGGLE          GPIO_ToggleBits(GPIOx(LED_PORT), PIN_MASK(LED_PIN))

void LedPortInit(void)
{
  GPIO_InitTypeDef l_GPIO_InitStruct;

  GPIO_StructInit(&l_GPIO_InitStruct);
  l_GPIO_InitStruct.GPIO_Mode                   = GPIO_Mode_OUT;
  l_GPIO_InitStruct.GPIO_OType                  = GPIO_OType_PP;
  l_GPIO_InitStruct.GPIO_Pin                    = PIN_MASK(LED_PIN);
  l_GPIO_InitStruct.GPIO_PuPd                   = GPIO_PuPd_NOPULL;
  l_GPIO_InitStruct.GPIO_Speed                  = GPIO_High_Speed;

  RCC_AHB1PeriphClockCmd(RCC_PORT_MASKx(LED_PORT), ENABLE);

  GPIO_Init(GPIOx(LED_PORT), &l_GPIO_InitStruct);
}

int main(void)
{
 BootJumperStatus_T BootJumperStatus = BOOT_JUMPER_STATUS_FAILED;
 BootJumperStatus = Boot_StartupHandler();
 if(BootJumperStatus == BOOT_JUMPER_STATUS_FAILED)
 {
	 trace_printf("BootStartupHandler()->BOOT_JUMPER_STATUS_FAILED");
 }
  // sprawdz czy cos zosta³o zapisane do backupa
  // jeœli tak to wywo³ac sysdeinit() i skoczyc do bootloadera



uint32_t timeout = 10000000;
	while(1)
	{

		while(timeout > 0)
		{
			timeout--;
		}

		BootJumperStatus = Boot_JumpToBootloaderAfterReset();

		 if(BootJumperStatus == BOOT_JUMPER_STATUS_FAILED)
		 {
			 trace_printf("BootProgramHandler()->BOOT_JUMPER_STATUS_FAILED");
			 timeout = 10000000;
		 }

	}
}

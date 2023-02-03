/**
  ******************************************************************************
  * @file    STM32F4xx_IAP/src/common.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    10-October-2011
  * @brief   This file provides all the common functions.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */ 

/** @addtogroup STM32F4xx_IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "stdarg.h"
#include "string.h"
/* Private typedef -----------------------------------------------------------*/
typedef void (*pIapFun_TypeDef)(void);
/* Private define ------------------------------------------------------------*/
#define FLASH_APP_ADDR 0x8010000
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern UART_HandleTypeDef huart1;
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
void vprint(const char *fmt, va_list argp) {
	char string[300];
	if (0 < vsprintf(string, fmt, argp)) // build string
			{
		while(HAL_UART_Transmit(&huart1, (uint8_t*) string, strlen(string),100)!=HAL_OK); // send message via UART
	}
}

int my_printf(const char *fmt, ...) // custom my_printf() function
{
	va_list argp;
	va_start(argp, fmt);
	vprint(fmt, argp);
	va_end(argp);
}




void IAP_ExecuteApp ( uint32_t ulAddr_App )
{
	int i = 0;
	pIapFun_TypeDef pJump2App; 

	
	if ( ( ( * ( __IO uint32_t * ) ulAddr_App ) & 0x2FFE0000 ) == 0x20000000 )	 //�??查栈顶地�??是否合法0x20000000是sram的起始地�??,也是程序的栈顶地�??
	{ 
//		HAL_PCD_MspDeInit(&hpcd_USB_OTG_FS);
//		HAL_SPI_MspDeInit(&hspi1);	//@2
		__HAL_RCC_GPIOB_CLK_DISABLE();
		__HAL_RCC_GPIOG_CLK_DISABLE();
		
		
		 /* 设置�??有时钟到默认状�?�，使用HSI时钟 */
		HAL_RCC_DeInit();		//@3
		
		__set_BASEPRI(0x20);		//@4
		__set_PRIMASK(1);
        __set_FAULTMASK(1);
		
		/* 关闭�??有中断，清除�??有中断挂起标�?? */
		for (i = 0; i < 8; i++)		//@5
		{
			NVIC->ICER[i]=0xFFFFFFFF;
			NVIC->ICPR[i]=0xFFFFFFFF;
		}
		
		SysTick->CTRL = 0;		//@6
		SysTick->LOAD = 0;
		SysTick->VAL = 0;

		__set_BASEPRI(0);		//@7
		__set_PRIMASK(0);
		__set_FAULTMASK(0);
 		
 		//@8
        /*
        1）不使用OS时： 只用到MSP（中断和非中断都使用MSP）；
        2）使用OS时（如UCOSII）： main函数和中断使用MSP�?? 各个Task（线程）使用PSP（即任务栈）�??
        */
        __set_MSP(*(uint32_t*)ulAddr_App);//当带操作系统从APP区跳转到BOOT区的时�?�需要将SP设置为MSP，否则在BOOT区中使用中断将会引发硬件错误�??
        __set_PSP(*(uint32_t*)ulAddr_App);
        __set_CONTROL(0);  /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
        __ISB();//指令同步隔离。最严格：它会清洗流水线，以保证�??有它前面的指令都执行完毕之后，才执行它后面的指令�??
		//@9
		pJump2App = ( pIapFun_TypeDef ) * ( __IO uint32_t * ) ( ulAddr_App + 4 );	//用户代码区第二个字为程序�??始地�??(复位地址)		
		pJump2App ();								                                    	//跳转到APP.
	}
}	
/**
  * @brief  Convert an Integer to a string
  * @param  str: The string
  * @param  intnum: The integer to be converted
  * @retval None
  */
void Int2Str(uint8_t* str, int32_t intnum)
{
  uint32_t i, Div = 1000000000, j = 0, Status = 0;

  for (i = 0; i < 10; i++)
  {
    str[j++] = (intnum / Div) + 48;

    intnum = intnum % Div;
    Div /= 10;
    if ((str[j-1] == '0') & (Status == 0))
    {
      j = 0;
    }
    else
    {
      Status++;
    }
  }
}

/**
  * @brief  Convert a string to an integer
  * @param  inputstr: The string to be converted
  * @param  intnum: The integer value
  * @retval 1: Correct
  *         0: Error
  */
uint32_t Str2Int(uint8_t *inputstr, int32_t *intnum)
{
  uint32_t i = 0, res = 0;
  uint32_t val = 0;

  if (inputstr[0] == '0' && (inputstr[1] == 'x' || inputstr[1] == 'X'))
  {
    if (inputstr[2] == '\0')
    {
      return 0;
    }
    for (i = 2; i < 11; i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1; */
        res = 1;
        break;
      }
      if (ISVALIDHEX(inputstr[i]))
      {
        val = (val << 4) + CONVERTHEX(inputstr[i]);
      }
      else
      {
        /* Return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 8 digit hex --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }
  else /* max 10-digit decimal input */
  {
    for (i = 0;i < 11;i++)
    {
      if (inputstr[i] == '\0')
      {
        *intnum = val;
        /* return 1 */
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'k' || inputstr[i] == 'K') && (i > 0))
      {
        val = val << 10;
        *intnum = val;
        res = 1;
        break;
      }
      else if ((inputstr[i] == 'm' || inputstr[i] == 'M') && (i > 0))
      {
        val = val << 20;
        *intnum = val;
        res = 1;
        break;
      }
      else if (ISVALIDDEC(inputstr[i]))
      {
        val = val * 10 + CONVERTDEC(inputstr[i]);
      }
      else
      {
        /* return 0, Invalid input */
        res = 0;
        break;
      }
    }
    /* Over 10 digit decimal --invalid */
    if (i >= 11)
    {
      res = 0;
    }
  }

  return res;
}

/**
  * @brief  Get an integer from the HyperTerminal
  * @param  num: The integer
  * @retval 1: Correct
  *         0: Error
  */
uint32_t GetIntegerInput(int32_t * num)
{
  uint8_t inputstr[16];

  while (1)
  {
    GetInputString(inputstr);
    if (inputstr[0] == '\0') continue;
    if ((inputstr[0] == 'a' || inputstr[0] == 'A') && inputstr[1] == '\0')
    {
      SerialPutString("User Cancelled \r\n");
      return 0;
    }

    if (Str2Int(inputstr, num) == 0)
    {
      SerialPutString("Error, Input again: \r\n");
    }
    else
    {
      return 1;
    }
  }
}

/**
  * @brief  Test to see if a key has been pressed on the HyperTerminal
  * @param  key: The key pressed
  * @retval 1: Correct
  *         0: Error
  */
uint32_t SerialKeyPressed(uint8_t *key)
{
  
  if (__HAL_UART_GET_FLAG(&huart1, UART_FLAG_RXNE) != RESET )
  {
		
    *key = (uint8_t)USART1->DR;
    return 1;
  }
  else
  {
    return 0;
  }
	
//  if (UserRxBufferLen==0)
//  {
////		UserRxBufferLen=0;
////		strcpy(UserRxBufferFS,"");
//    return 0;
//  }
//  else
//  {
//		*key=UserRxBufferFS[0];
////		my_printf("%c",*key);
//    strcpy(UserRxBufferFS,"");
//		UserRxBufferLen=0;
//    return 1;

//  }
}

/**
  * @brief  Get a key from the HyperTerminal
  * @param  None
  * @retval The Key Pressed
  */
uint8_t GetKey(void)
{
  uint8_t key = 0;

  /* Waiting for user input */
  while (1)
  {
    if (SerialKeyPressed((uint8_t*)&key)) break;
  }
  return key;

}

/**
  * @brief  Print a character on the HyperTerminal
  * @param  c: The character to be printed
  * @retval None
  */
void SerialPutChar(uint8_t c)
{
	my_printf("%c",c);
//  USART_SendData(EVAL_COM1, c);
//  while (USART_GetFlagStatus(EVAL_COM1, USART_FLAG_TXE) == RESET)
//  {
//  }
}

/**
  * @brief  Print a string on the HyperTerminal
  * @param  s: The string to be printed
  * @retval None
  */
void Serial_PutString(uint8_t *s)
{
  while (*s != '\0')
  {
    SerialPutChar(*s);
    s++;
  }
}

/**
  * @brief  Get Input string from the HyperTerminal
  * @param  buffP: The input string
  * @retval None
  */
void GetInputString (uint8_t * buffP)
{
  uint32_t bytes_read = 0;
  uint8_t c = 0;
  do
  {
    c = GetKey();
    if (c == '\r')
      break;
    if (c == '\b') /* Backspace */
    {
      if (bytes_read > 0)
      {
        SerialPutString("\b \b");
        bytes_read --;
      }
      continue;
    }
    if (bytes_read >= CMD_STRING_SIZE )
    {
      SerialPutString("Command string size overflow\r\n");
      bytes_read = 0;
      continue;
    }
    if (c >= 0x20 && c <= 0x7E)
    {
      buffP[bytes_read++] = c;
      SerialPutChar(c);
    }
  }
  while (1);
  SerialPutString(("\n\r"));
  buffP[bytes_read] = '\0';
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/

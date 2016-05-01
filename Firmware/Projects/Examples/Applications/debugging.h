#ifndef __DEBUGGING_H__
#define __DEBUGGING_H__


/*
 * Includes
 */
#include <hal_types.h>
#include <ioCC1110.h>
#include <ioCCxx10_bitdef.h>
#include <hal_cc8051.h>
#include <string.h>

  
/*
 * Constants
 */
/*
 * Constants
 */
// Baudrate = 57600 bps (U0BAUD.BAUD_M = 34, U0GCR.BAUD_E = 11)
#define UART1_BAUD_M  34
#define UART1_BAUD_E  11
// Baudrate = 115200 bps (U0BAUD.BAUD_M = 34, U0GCR.BAUD_E = 12)
#define UART0_BAUD_M  34
#define UART0_BAUD_E  12
   
   
/*
 * Function prototype
 */
void mSysConfig(void);
void SleepTimerConfig(void);
void Timer3Config(void);
void UART0Config(void);
void UART1Config(void);
void UART1SendStr(uint8 *str);
void UART1SendArr(uint8 *uartArr, uint16 len);
void reverse(uint8 s[]);
void itoa(uint32 n, uint8 s[]);
void UART1SendNum(uint32 num, uint8 isEnter);


#endif // __DEBUGGING_H__
/*
 * Includes
 */
#include "debugging.h"


/*
 * Variables
 */
   

/*
 * Function decleration
 */
void mSysConfig(void) {
  /***************************************************************************
   * Configure UART
   *
   * The system clock source used is the HS XOSC at 26 MHz speed.
   */
  // Set system clock source to 26 Mhz XSOSC to support maximum transfer speed,
  // ref. [clk]=>[clk_xosc.c]
  SLEEP &= ~SLEEP_OSC_PD;
  while( !(SLEEP & SLEEP_XOSC_S) );
  CLKCON = (CLKCON & ~(CLKCON_CLKSPD | CLKCON_OSC)) | CLKSPD_DIV_1;
  while (CLKCON & CLKCON_OSC);
  SLEEP |= SLEEP_OSC_PD;
  
  // Select 32.768 kHz LS XOSC
  CLKCON &= ~CLKCON_OSC32;
}

void Timer3Config(void) {
  /* Timer 3 control. Configuration:
   * - Prescaler divider value: 1.
   * - Interrupts enabled.
   * - Free running mode.
   * The Timer is also cleared and started.
   */
  T3CTL = T3CTL_DIV_1 | T3CTL_START | T3CTL_OVFIM | T3CTL_CLR | T3CTL_MODE_FREERUN;

  /* Enables global interrupts (IEN0.EA = 1) and interrupts from Timer 3
   * (IEN1.T3IE = 1).
   */
  EA = 1; T3IE = 0;
}

void SleepTimerConfig(void) {
  // Clear interrupt flags
  // Clear [IRCON.STIF] (Sleep Timer CPU interrupt flag)
  STIF = 0;

  // Clear [WORIRQ.EVENT0_FLAG] (Sleep Timer peripheral interrupt flag)
  WORIRQ &= ~WORIRQ_EVENT0_FLAG;

  // Set individual interrupt enable bit in the peripherals SFR
  WORIRQ |= WORIRQ_EVENT0_MASK;    // Enable interrupt mask for sleep timer

  // Set the individual, interrupt enable bit [IEN0.STIE=1]
  STIE = 1;

  // 1.4 Enable global interrupt by setting the [IEN0.EA=1]
  EA = 1;
  
  /* Now the time between two consecutive Event 0’s is decided by:
     t = EVENT0 * 2^(5*WOR_RES) / 32768
     By using EVENT0 = 32 and WOR_RES = 2, t = 1 s. So by using these values,
     a Sleep Timer interrupt will be generated every second. */

  // Set [WORCTL.WOR_RES = 1]
  WORCTL = (WORCTL & ~WORCTL_WOR_RES) | WORCTL_WOR_RES_32;

  /* Must wait for 2 clock periods after resetting the Sleep Timer before
     setting EVENT0 value */

  // Reset timer and set EVENT0 value.
  WORCTL |= WORCTL_WOR_RESET;             // Reset Sleep Timer
  char temp = WORTIME0;
  while(temp == WORTIME0);                // Wait until a positive 32 kHz edge
  temp = WORTIME0;
  while(temp == WORTIME0);                // Wait until a positive 32 kHz edge
  WOREVT0 = 0x00;                         // Use 32 for EVENT0 value
  WOREVT1 = 0x10;
}

void UART0Config(void) {  
  /***************************************************************************
   * Setup I/O ports
   *
   * Port and pins used by USART0 operating in UART-mode are
   * RX     : P0_2
   * TX     : P0_3
   * CT/CTS : P0_4
   * RT/RTS : P0_5
   *
   * These pins can be set to function as peripheral I/O to be be used by UART0.
   * The TX pin on the transmitter must be connected to the RX pin on the receiver.
   * If enabling hardware flow control (U0UCR.FLOW = 1) the CT/CTS (Clear-To-Send)
   * on the transmitter must be connected to the RS/RTS (Ready-To-Send) pin on the
   * receiver.
   */

  // Configure USART0 for Alternative 1 => Port P0 (PERCFG.U0CFG = 0)
  // To avoid potential I/O conflict with USART1:
  // configure USART1 for Alternative 2 => Port P1 (PERCFG.U1CFG = 1)
  PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U1CFG;

  // Configure relevant Port P0 pins for peripheral function:
  // P0SEL.SELP0_2/3/4/5 = 1 => RX = P0_2, TX = P0_3, CT = P0_4, RT = P0_5
  P0SEL |= BIT5 | BIT4 | BIT3 | BIT2;

  // Initialise bitrate = 57.6 kbps (U0BAUD.BAUD_M = 34, U0GCR.BAUD_E = 11)
  U0BAUD = UART0_BAUD_M;
  U0GCR = (U0GCR & ~U0GCR_BAUD_E) | UART0_BAUD_E;

  // Initialise UART protocol (start/stop bit, data bits, parity, etc.):

  // USART mode = UART (U0CSR.MODE = 1)
  U0CSR |= U0CSR_MODE;

  // Start bit level = low => Idle level = high  (U0UCR.START = 0)
  U0UCR &= ~U0UCR_START;

  // Stop bit level = high (U0UCR.STOP = 1)
  U0UCR |= U0UCR_STOP;

  // Number of stop bits = 1 (U0UCR.SPB = 0)
  U0UCR &= ~U0UCR_SPB;

  // Parity = disabled (U0UCR.PARITY = 0)
  U0UCR &= ~U0UCR_PARITY;

  // 9-bit data enable = 8 bits transfer (U0UCR.BIT9 = 0)
  U0UCR &= ~U0UCR_BIT9;

  // Level of bit 9 = 0 (U0UCR.D9 = 0), used when U0UCR.BIT9 = 1
  // Level of bit 9 = 1 (U0UCR.D9 = 1), used when U0UCR.BIT9 = 1
  // Parity = Even (U0UCR.D9 = 0), used when U0UCR.PARITY = 1
  // Parity = Odd (U0UCR.D9 = 1), used when U0UCR.PARITY = 1
  U0UCR &= ~U0UCR_D9;

  // Flow control = disabled (U0UCR.FLOW = 0)
  U0UCR &= ~U0UCR_FLOW;

  // Bit order = LSB first (U0GCR.ORDER = 0)
  U0GCR &= ~U0GCR_ORDER;
  
  // Clear any pending UART RX Interrupt Flag (TCON.URXxIF = 0, UxCSR.RX_BYTE = 0)
  URX0IF = 0; U0CSR &= ~U0CSR_RX_BYTE;

  // Enable UART RX (UxCSR.RE = 1)
  U0CSR |= U0CSR_RE;

  // Enable global Interrupt (IEN0.EA = 1) and UART RX Interrupt (IEN0.URXxIE = 1)
  EA = 1; URX0IE = 1;
}
   
void UART1Config(void) { 
  /***************************************************************************
   * Setup I/O ports
   *
   * Port and pins used by USART1 operating in UART-mode are
   * RX     : P1_7
   * TX     : P1_6
   * CT/CTS : P1_5
   * RT/RTS : P1_4
   *
   * These pins can be set to function as peripheral I/O to be be used by UART0.
   * The TX pin on the transmitter must be connected to the RX pin on the receiver.
   * If enabling hardware flow control (U1UCR.FLOW = 1) the CT/CTS (Clear-To-Send)
   * on the transmitter must be connected to the RS/RTS (Ready-To-Send) pin on the
   * receiver.
   */

  // Configure USART0 for Alternative 1 => Port P0 (PERCFG.U0CFG = 0)
  // To avoid potential I/O conflict with USART1:
  // configure USART1 for Alternative 2 => Port P1 (PERCFG.U1CFG = 1)
  PERCFG = (PERCFG & ~PERCFG_U0CFG) | PERCFG_U1CFG;

  // Configure relevant Port P1 pins for peripheral function:
  // P1SEL.SELP1_4/5/6/7 = 1 => RX = P1_2, TX = P1_3, CT = P1_4, RT = P1_5
  P1SEL |= BIT7 | BIT6 | BIT5 | BIT4;

  // Initialise bitrate = 57.6 kbps (U0BAUD.BAUD_M = 34, U0GCR.BAUD_E = 11)
  U1BAUD = UART1_BAUD_M;
  U1GCR = (U1GCR & ~U1GCR_BAUD_E) | UART1_BAUD_E;

  // Initialise UART protocol (start/stop bit, data bits, parity, etc.):

  // USART mode = UART (U1CSR.MODE = 1)
  U1CSR |= U1CSR_MODE;

  // Start bit level = low => Idle level = high  (U1UCR.START = 0)
  U1UCR &= ~U1UCR_START;

  // Stop bit level = high (U1UCR.STOP = 1)
  U1UCR |= U1UCR_STOP;

  // Number of stop bits = 1 (U1UCR.SPB = 0)
  U1UCR &= ~U1UCR_SPB;

  // Parity = disabled (U1UCR.PARITY = 0)
  U1UCR &= ~U1UCR_PARITY;

  // 9-bit data enable = 8 bits transfer (U1UCR.BIT9 = 0)
  U1UCR &= ~U1UCR_BIT9;

  // Level of bit 9 = 0 (U1UCR.D9 = 0), used when U1UCR.BIT9 = 1
  // Level of bit 9 = 1 (U1UCR.D9 = 1), used when U1UCR.BIT9 = 1
  // Parity = Even (U1UCR.D9 = 0), used when U1UCR.PARITY = 1
  // Parity = Odd (U1UCR.D9 = 1), used when U1UCR.PARITY = 1
  U1UCR &= ~U1UCR_D9;

  // Flow control = disabled (U1UCR.FLOW = 0)
  U1UCR &= ~U1UCR_FLOW;

  // Bit order = LSB first (U1GCR.ORDER = 0)
  U1GCR &= ~U1GCR_ORDER;
}

void UART1SendArr(uint8 *uartArr, uint16 len) {
  for(uint16 i = 0; i < len; i++) {
    U1DBUF = uartArr[i];
    while(!(U1CSR & U1CSR_TX_BYTE)); // wait until transmmition done
    U1CSR &= ~U1CSR_TX_BYTE; // clear TX flag
  }
}

void UART1SendStr(uint8 *uartStr) { 
  // Clear any pending TX interrupt request (set U1CSR.TX_BYTE = 0)
  U1CSR &= ~U1CSR_TX_BYTE;
  
  // Loop: send each charater until meet the End Of String character
  uint16 index = 0;
  while(uartStr[index] != '\0') {
    U1DBUF = uartStr[index]; // put character in TX buff
    while(!(U1CSR & U1CSR_TX_BYTE)); // wait until transmmition done
    U1CSR &= ~U1CSR_TX_BYTE; // clear TX flag
    index++; // next character
  }
  
  return;
}

// reverse: reverse string s in place
void reverse(uint8 s[]) {
  uint32 c = 0, i = 0, j = 0;

  for (i = 0, j = strlen((char const *)s) - 1; i < j; i++, j--) {
    c = s[i]; s[i] = s[j]; s[j] = c;
  }
}

// itoa: convert n to characters in s
void itoa(uint32 n, uint8 s[]) {
  int32 i = 0, sign = n;

  /* record sign */
  if(sign < 0) { n = -n; }
  
  do { // generate digits in reverse order
    s[i++] = n % 10 + '0'; // get next digit
  } while ((n /= 10) > 0); // delete it
  
  if(sign < 0) { s[i++] = '-'; }
  s[i] = '\0';
  
  reverse(s);
}

void UART1SendNum(uint32 num, uint8 isEnter) {
  // Clear any pending TX interrupt request (set U1CSR.TX_BYTE = 0)
  U1CSR &= ~U1CSR_TX_BYTE;
  
  uint8 arr[8];
  itoa(num, arr);
  
  UART1SendStr(arr);
  if(isEnter) { UART1SendStr("\r\n"); }
  
  return;
}
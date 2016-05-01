/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *---------------------------------------------------------------------------- */

/**********************************************************************************************
  Copyright 2007-2009 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights granted under
  the terms of a software license agreement between the user who downloaded the software,
  his/her employer (which must be your employer) and Texas Instruments Incorporated (the
  "License"). You may not use this Software unless you agree to abide by the terms of the
  License. The License limits your use, and you acknowledge, that the Software may not be
  modified, copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio frequency
  transceiver, which is integrated into your product. Other than for the foregoing purpose,
  you may not use, reproduce, copy, prepare derivative works of, modify, distribute,
  perform, display or sell this Software and/or its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE PROVIDED “AS IS”
  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY
  WARRANTY OF MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
  IN NO EVENT SHALL TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER LEGAL EQUITABLE
  THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES INCLUDING BUT NOT LIMITED TO ANY
  INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST
  DATA, COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY
  THIRD PARTIES (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#include "bsp.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk_pll.h"

#include <math.h>
#include <stdlib.h>
#include "debugging.h"


#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif

#define TEST_LED_ON()           P2_0  = 1
#define TEST_LED_OFF()          P2_0  = 0
#define TEST_LED_TOG()          P2_0 ^= 1

/*******************************************************************************
* LOCAL FUNCTIONS
*/
static void linkTo(void);
static void EnterPM0(void);
static void EnterPM2(void);

/*******************************************************************************
* LOCAL VARIABLES
*/
volatile uint8 delay;
volatile uint8 cnt;
uint8 pwmLut_idx00 = 0;
uint8 pwmLut_idx01 = 2;
uint8 pwmLut_idx02 = 4;
uint8 pwmLut_idx03 = 6;
uint8 pwmLut_idx04 = 8;
uint8 pwmLut_idx05 = 10;
uint8 pwmLut_idx06 = 12;
uint8 pwmLut_idx07 = 14;
uint8 pwmLut_idx10 = 16;
uint8 pwmLut_idx11 = 18;
uint8 pwmLut_idx12 = 20;
uint8 pwmLut_idx13 = 22;
uint8 pwmLut_idx14 = 24;
uint8 pwmLut_idx15 = 26;
uint8 pwmLut_idx16 = 28;
uint8 pwmLut_idx17 = 30;
static linkID_t sLinkID1 = 0;
// Initialization of source buffers and DMA descriptor for the DMA transfer
// (ref. CC111xFx/CC251xFx Errata Note)
static uint8 __xdata PM2_BUF[7] = {0x06,0x06,0x06,0x06,0x06,0x06,0x04};
static uint8 __xdata dmaDesc[8] = {0x00,0x00,0xDF,0xBE,0x00,0x07,0x20,0x42};
// Variable for active mode duration
static uint32 __xdata activeModeCnt = 0;


/*******************************************************************************
* CONSTANTS
*/
#define MAX_CNT 100
#define DELAY_TIME 200
#define PWMLUT_LEN 52
#define SPIN_ABOUT_A_SECOND   NWK_DELAY(1000)
#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)
#define SW_RESET()        ((*((void (* __code *)(void))1))())
/* How many times to try a Tx and miss an acknowledge before doing a scan */
#define MISSES_IN_A_ROW  2

const uint8 pwmLut[PWMLUT_LEN] =  {0, 2, 2, 2, 4, 4, 4, 4, 6, 8, 8, 12,
                                   14, 18, 22, 28, 36, 46, 56, 68, 78, 90,
                                   96, 100, 100, 100, 100, 100, 100, 96,
                                   90, 78, 68, 56, 46, 36, 28, 22, 18, 14,
                                   12, 8, 8, 6, 4, 4, 4, 4, 2, 2, 2, 0};

uint16_t mRand(uint16_t min, uint16_t max) {
  return (uint16_t)(((max - min + 1) * ((double) rand() / RAND_MAX)) + min);
}

void main (void)
{
  BSP_Init();
  P0INP = P1INP  = 0xFF;
  
  // config system clk
  mSysConfig();
  
  // configure UART0 for debugging, this section overlap clk config in SMPL_Init() function
  //UART1Config();
  //UART1SendStr("mSysConfig done !!!\r\n");
  
  // config sleep timer, interval ~ 3s
  SleepTimerConfig();
  //UART1SendStr("SleepTimerConfig done !!!\r\n");
  
  // config PWM for LED dimming
  Timer3Config();
  //UART1SendStr("PWMConfig done !!!\r\n");

  /* If an on-the-fly device address is generated it must be done before the
   * call to SMPL_Init(). If the address is set here the ROM value will not
   * be used. If SMPL_Init() runs before this IOCTL is used the IOCTL call
   * will not take effect. One shot only. The IOCTL call below is conformal.
   */
#ifdef I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE
  {
    addr_t lAddr;

    createRandomAddress(&lAddr);
    SMPL_Ioctl(IOCTL_OBJ_ADDR, IOCTL_ACT_SET, &lAddr);
  }
#endif /* I_WANT_TO_CHANGE_DEFAULT_ROM_DEVICE_ADDRESS_PSEUDO_CODE */
  
  /* Keep trying to join (a side effect of successful initialization) until
   * successful. Toggle LEDS to indicate that joining has not occurred.
   */
  //UART1SendStr("Start SMPL_Init ...\r\n");
  uint8_t ret = SMPL_Init(0);
  while (SMPL_SUCCESS != ret) {
    TEST_LED_TOG();
    SPIN_ABOUT_A_SECOND; /* calls nwk_pllBackgrounder for us */
    ret = SMPL_Init(0);
    EnterPM2(); mSysConfig();
  }
  TEST_LED_OFF();

  /* Unconditional link to AP which is listening due to successful join. */
  //UART1SendStr("Start linkTo ...\r\n");
  linkTo();

  while (1) {
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) );
  }
}

static void linkTo()
{
  /* Keep trying to link... */
  while (SMPL_SUCCESS != SMPL_Link(&sLinkID1)) {
    TEST_LED_TOG();
    //UART1SendStr("Link failed !!!\r\n");
    SPIN_ABOUT_A_SECOND; /* calls nwk_pllBackgrounder for us */
  }
  //UART1SendStr("Link success !!!\r\n");
  TEST_LED_ON(); NWK_DELAY(1000); TEST_LED_OFF();
  
  uint8_t msg[3] = {0}, len;
  while(1) {    
    /* keep the FHSS scheduler happy */
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) );   
    
    // wake up radio. we need it to listen for others babbling.
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_AWAKE, 0);
    // turn on RX. default is RX off.
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
    
    // Try receive message
    while(1) {
      uint8_t ret = SMPL_Receive(sLinkID1, msg, &len);
      if(ret == SMPL_SUCCESS) { // receive successed
        //UART1SendStr("Recv RF data\r\n");
        //TEST_LED_TOG();
        break;
      } else if(ret == SMPL_TIMEOUT) { // no response from AP
        static uint8_t cnt = 0;
        if(cnt++ == 5) { // retry time out, fource reset system
          //UART1SendStr("No response, soft reset \r\n");
          // enable WDT reset after 1s
          WDCTL = (WDCTL & ~WDCTL_INT) | WDCTL_INT_SEC_1;
          WDCTL = (WDCTL & ~WDCTL_MODE) | WDCTL_EN;
          while(1); // wait for reset
        }
      } else if(ret == SMPL_NO_PAYLOAD) { // nothing to receive, break out and go to sleep mode
        static uint8 retry = 0;
        if(retry++ >= 5) { retry = 0; break; }
        //NWK_DELAY(500);
      }
    }
    
    // turn off RX
    SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_SLEEP, 0);
    
    // process cmd from Control Box
    uint8 net_addr[4] = THIS_DEVICE_ADDRESS;
    if(msg[0] == net_addr[0]) { // node_info.node_addr
      //UART1SendStr("Unicast cmd\n");
      P0DIR = msg[1]; P1DIR = msg[2];
    } else if(msg[0] == 0xFF) { // broadcast
      //UART1SendStr("Broadcast cmd\n");
      P0DIR = msg[1]; P1DIR = msg[2];
    }

    //UART1SendStr("P0DIR = "); UART1SendNum(P0DIR, 1);
    //UART1SendStr("P1DIR = "); UART1SendNum(P1DIR, 1);
    
    // check sleep or not
    if((P0DIR == 0x00) && (P1DIR == 0x00)) {
      //UART1SendStr("It's time to sleep\n");
      T3IE = 0; 
      EnterPM2();
    } else { 
      //UART1SendStr("It's time for show\n");
      T3IE = 1; 
      //EnterPM0();
      NWK_DELAY(100);
    }
    msg[0] = msg[1] = msg[2] = 0;
    
    mSysConfig();
  }
}

/***********************************************************************************
* @fn          sleep_isr
*
* @brief       Interrupt service routine for the sleep timer which wakes the
*              system from Power Mode. When awake the flags are cleared and
*              the green LED is toggled.
*
* @param       void
*
* @return      void
*/
#pragma vector = ST_VECTOR
__interrupt void sleep_isr(void) {
    // Clear [IRCON.STIF] (Sleep Timer CPU interrupt flag)
    STIF = 0;

    // Clear [WORIRQ.EVENT0_FLAG] (Sleep Timer peripheral interrupt flag)
    WORIRQ &= ~WORIRQ_EVENT0_FLAG;
    
    // Clear the [SLEEP.MODE] bits, because an interrupt can also occur
    // before the SoC has actually entered Power Mode 2.
    SLEEP &= ~SLEEP_MODE;
}

/*******************************************************************************
* @fn      t3_isr
*
* @brief   Interrupt handler for Timer 3 overflow interrupts. Toggles the green
*          LED. Interrupts from Timer 3 are level triggered, so the module
*          interrupt flag is cleared prior to the CPU interrupt flag.
*
* @param   void
*
* @return  void
*
*******************************************************************************/
#pragma vector = T3_VECTOR
__interrupt void t3_isr(void)
{ 
  /* Clears the module interrupt flag. */
  T3OVFIF = 0;
  
  if(cnt++ >= MAX_CNT) {
    cnt = 0;
  } else {
    if(cnt <= pwmLut[pwmLut_idx00]) { P0_0 = 1; } else { P0_0 = 0; }
    if(cnt <= pwmLut[pwmLut_idx01]) { P0_1 = 1; } else { P0_1 = 0; }
    if(cnt <= pwmLut[pwmLut_idx02]) { P0_2 = 1; } else { P0_2 = 0; }
    if(cnt <= pwmLut[pwmLut_idx03]) { P0_3 = 1; } else { P0_3 = 0; }
    if(cnt <= pwmLut[pwmLut_idx04]) { P0_4 = 1; } else { P0_4 = 0; }
    if(cnt <= pwmLut[pwmLut_idx05]) { P0_5 = 1; } else { P0_5 = 0; }
    if(cnt <= pwmLut[pwmLut_idx06]) { P0_6 = 1; } else { P0_6 = 0; }
    if(cnt <= pwmLut[pwmLut_idx07]) { P0_7 = 1; } else { P0_7 = 0; }
    if(cnt <= pwmLut[pwmLut_idx10]) { P1_0 = 1; } else { P1_0 = 0; }
    if(cnt <= pwmLut[pwmLut_idx11]) { P1_1 = 1; } else { P1_1 = 0; }
    if(cnt <= pwmLut[pwmLut_idx12]) { P1_2 = 1; } else { P1_2 = 0; }
    if(cnt <= pwmLut[pwmLut_idx13]) { P1_3 = 1; } else { P1_3 = 0; }
    if(cnt <= pwmLut[pwmLut_idx14]) { P1_4 = 1; } else { P1_4 = 0; }
    if(cnt <= pwmLut[pwmLut_idx15]) { P1_5 = 1; } else { P1_5 = 0; }
    if(cnt <= pwmLut[pwmLut_idx16]) { P1_6 = 1; } else { P1_6 = 0; }
    if(cnt <= pwmLut[pwmLut_idx17]) { P1_7 = 1; } else { P1_7 = 0; }
    
    // after DELAY_TIME change duty cycle
    if(delay++ >= DELAY_TIME) {
      if(P0DIR & BIT0) { if(pwmLut_idx00++ >= PWMLUT_LEN - 1) { pwmLut_idx00 = 0; } }
      if(P0DIR & BIT1) { if(pwmLut_idx01++ >= PWMLUT_LEN - 1) { pwmLut_idx01 = 0; } }
      if(P0DIR & BIT2) { if(pwmLut_idx02++ >= PWMLUT_LEN - 1) { pwmLut_idx02 = 0; } }
      if(P0DIR & BIT3) { if(pwmLut_idx03++ >= PWMLUT_LEN - 1) { pwmLut_idx03 = 0; } }
      if(P0DIR & BIT4) { if(pwmLut_idx04++ >= PWMLUT_LEN - 1) { pwmLut_idx04 = 0; } }
      if(P0DIR & BIT5) { if(pwmLut_idx05++ >= PWMLUT_LEN - 1) { pwmLut_idx05 = 0; } }
      if(P0DIR & BIT6) { if(pwmLut_idx06++ >= PWMLUT_LEN - 1) { pwmLut_idx06 = 0; } }
      if(P0DIR & BIT7) { if(pwmLut_idx07++ >= PWMLUT_LEN - 1) { pwmLut_idx07 = 0; } }
      if(P1DIR & BIT0) { if(pwmLut_idx10++ >= PWMLUT_LEN - 1) { pwmLut_idx10 = 0; } }
      if(P1DIR & BIT1) { if(pwmLut_idx11++ >= PWMLUT_LEN - 1) { pwmLut_idx11 = 0; } }
      if(P1DIR & BIT2) { if(pwmLut_idx12++ >= PWMLUT_LEN - 1) { pwmLut_idx12 = 0; } }
      if(P1DIR & BIT3) { if(pwmLut_idx13++ >= PWMLUT_LEN - 1) { pwmLut_idx13 = 0; } }
      if(P1DIR & BIT4) { if(pwmLut_idx14++ >= PWMLUT_LEN - 1) { pwmLut_idx14 = 0; } }
      if(P1DIR & BIT5) { if(pwmLut_idx15++ >= PWMLUT_LEN - 1) { pwmLut_idx15 = 0; } }
      if(P1DIR & BIT6) { if(pwmLut_idx16++ >= PWMLUT_LEN - 1) { pwmLut_idx16 = 0; } }
      if(P1DIR & BIT7) { if(pwmLut_idx17++ >= PWMLUT_LEN - 1) { pwmLut_idx17 = 0; } }
      
      delay = 0;
    }
  }

  /* Clears the CPU interrupt flag. */
  T3IF = 0;
}

void EnterPM0(void) {
  // Select Power Mode 0 (SLEEP.MODE = 0). Enter Power Mode
  SLEEP = (SLEEP & ~SLEEP_MODE) | SLEEP_MODE_PM0;
  PCON |= PCON_IDLE;
  
  /* The system will now wake up when Sleep Timer interrupt occurs. When awake,
     the system will start enter/exit Power Mode 0 using the loop below. */
  // Alignment of entering PM{0 – 2} to a positive edge on the 32 kHz clock source
  uint8_t temp = WORTIME0;
  while(temp == WORTIME0);    // Wait until a positive 32 kHz edge
  PCON |= PCON_IDLE;          // Go into Power Mode
}

void EnterPM2(void) {
  volatile uint8 storedDescHigh = DMA0CFGH;
  volatile uint8 storedDescLow = DMA0CFGL;
  volatile int8 temp;
  
  // Switch system clock source to HS RCOSC and max CPU speed:
  // Note that this is critical for Power Mode 2. After reset or
  // exiting Power Mode 2 the system clock source is HS RCOSC,
  // but to emphasize the requirement we choose to be explicit here.
  SLEEP &= ~SLEEP_OSC_PD;
  while( !(SLEEP & SLEEP_HFRC_S) );
  CLKCON = (CLKCON & ~CLKCON_CLKSPD) | CLKCON_OSC | CLKSPD_DIV_1;
  while ( !(CLKCON & CLKCON_OSC) ) ;
  SLEEP |= SLEEP_OSC_PD;

  // Set LS XOSC as the Sleep Timer clock source (CLKCON.OSC32 = 0)
  CLKCON &= ~CLKCON_OSC32;

  // Wait some time in Active Mode, and set SRF04EB LED1 before
  // entering Power Mode 2
  for(activeModeCnt = 0; activeModeCnt < 10000; activeModeCnt++);

  ///////////////////////////////////////////////////////////////////////
  ////////// CC111xFx/CC251xFx Errata Note Code section Begin ///////////
  ///////////////////////////////////////////////////////////////////////

  // Store current DMA channel 0 descriptor and abort any ongoing transfers,
  // if the channel is in use.
  storedDescHigh = DMA0CFGH;
  storedDescLow = DMA0CFGL;
  DMAARM |= (DMAARM_ABORT | DMAARM0);

  // Update descriptor with correct source.
  dmaDesc[0] = (uint16)&PM2_BUF >> 8;
  dmaDesc[1] = (uint16)&PM2_BUF;
  // Associate the descriptor with DMA channel 0 and arm the DMA channel
  DMA0CFGH = (uint16)&dmaDesc >> 8;
  DMA0CFGL = (uint16)&dmaDesc;
  DMAARM = DMAARM0;

  // NOTE! At this point, make sure all interrupts that will not be used to
  // wake from PM are disabled as described in the "Power Management Control"
  // chapter of the data sheet.

  // The following code is timing critical and should be done in the
  // order as shown here with no intervening code.

  // Align with positive 32 kHz clock edge as described in the
  // "Sleep Timer and Power Modes" chapter of the data sheet.
  temp = WORTIME0;
  while(temp == WORTIME0);

  // Make sure HS XOSC is powered down when entering PM{2 - 3} and that
  // the flash cache is disabled.
  MEMCTR |= MEMCTR_CACHD;
  SLEEP = 0x06;

  // Enter power mode as described in chapter "Power Management Control"
  // in the data sheet. Make sure DMA channel 0 is triggered just before
  // setting [PCON.IDLE].
  asm("NOP");
  asm("NOP");
  asm("NOP");
  if(SLEEP & 0x03)
  {
      asm("MOV 0xD7,#0x01");      // DMAREQ = 0x01;
      asm("NOP");                 // Needed to perfectly align the DMA transfer.
      asm("ORL 0x87,#0x01");      // PCON |= 0x01 -- Now in PM2;
      asm("NOP");                 // First call when awake
  }
  // End of timing critical code

  // Enable Flash Cache.
  MEMCTR &= ~MEMCTR_CACHD;

  // Update DMA channel 0 with original descriptor and arm channel if it was
  // in use before PM was entered.
  DMA0CFGH = storedDescHigh;
  DMA0CFGL = storedDescLow;
  DMAARM = DMAARM0;

  ///////////////////////////////////////////////////////////////////////
  /////////// CC111xFx/CC251xFx Errata Note Code section End ////////////
  ///////////////////////////////////////////////////////////////////////

  // Wait until HS RCOSC is stable
  while( !(SLEEP & SLEEP_HFRC_S) );

  // Set LS XOSC as the clock oscillator for the Sleep Timer (CLKCON.OSC32 = 0)
  CLKCON &= ~CLKCON_OSC32;
}
/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI
 *
 *  L. Friedman
 *  Texas Instruments, Inc.
 *----------------------------------------------------------------------------
 */

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
#include <string.h>
#include "bsp.h"
#include "mrfi.h"
#include "bsp_leds.h"
#include "bsp_buttons.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "nwk_frame.h"
#include "nwk.h"
#include "nwk_pll.h"

#include "debugging.h"
#include "stdlib.h"
#include "stdio.h"

#ifndef APP_AUTO_ACK
#error ERROR: Must define the macro APP_AUTO_ACK for this application.
#endif

/**************************** COMMENTS ON ASYNC LISTEN APPLICATION ***********************
Summary:
  This AP build includes implementation of an unknown number of end device peers in
  addition to AP functionality. In this scenario all End Devices establish a link to
  the AP and only to the AP. The AP acts as a data hub. All End Device peers are on
  the AP and not on other distinct ED platforms.

  There is still a limit to the number of peers supported on the AP that is defined
  by the macro NUM_CONNECTIONS. The AP will support NUM_CONNECTIONS or fewer peers
  but the exact number does not need to be known at build time.

  In this special but common scenario SimpliciTI restricts each End Device object to a
  single connection to the AP. If multiple logical connections are required these must
  be accommodated by supporting contexts in the application payload itself.

Solution overview:
  When a new peer connection is required the AP main loop must be notified. In essence
  the main loop polls a semaphore to know whether to begin listening for a peer Link
  request from a new End Device. There are two solutions: automatic notification and
  external notification. The only difference between the automatic notification
  solution and the external notification solution is how the listen semaphore is
  set. In the external notification solution the sempahore is set by the user when the
  AP is stimulated for example by a button press or a commend over a serial link. In the
  automatic scheme the notification is accomplished as a side effect of a new End Device
  joining.

  The Rx callback must be implemented. When the callback is invoked with a non-zero
  Link ID the handler could set a semaphore that alerts the main work loop that a
  SMPL_Receive() can be executed successfully on that Link ID.

  If the callback conveys an argument (LinkID) of 0 then a new device has joined the
  network. A SMPL_LinkListen() should be executed.

  Whether the joining device supports ED objects is indirectly inferred on the joining
  device from the setting of the NUM_CONNECTIONS macro. The value of this macro should
  be non-zero only if ED objects exist on the device. This macro is always non-zero
  for ED-only devices. But Range Extenders may or may not support ED objects. The macro
  should be be set to 0 for REs that do not also support ED objects. This prevents the
  Access Point from reserving resources for a joinng device that does not support any
  End Device Objects and it prevents the AP from executing a SMPL_LinkListen(). The
  Access Point will not ever see a Link frame if the joining device does not support
  any connections.

  Each joining device must execute a SMPL_Link() after receiving the join reply from the
  Access Point. The Access Point will be listening.

*************************** END COMMENTS ON ASYNC LISTEN APPLICATION ********************/

/************  THIS SOURCE FILE REPRESENTS THE AUTOMATIC NOTIFICATION SOLUTION ************/

/* reserve space for the maximum possible peer Link IDs */
static linkID_t sLID[NUM_CONNECTIONS] = {0};
static uint8_t  sNumCurrentPeers = 0;
volatile uint8_t uartSem = 0;
uint8 uartRxBuffer[500];
uint16 uartRxIndex = 0;

/* callback handler */
static uint8_t sCB(linkID_t);

/* Frequency Agility helper functions */
#ifdef FREQUENCY_AGILITY
static void    checkChangeChannel(void);
static void    changeChannel(void);
#endif

/* work loop semaphores */
static volatile uint8_t sPeerFrameSem = 0;
static volatile uint8_t sJoinSem = 0;
static uint8_t  sTid = 0;

#ifdef FREQUENCY_AGILITY
/*     ************** BEGIN interference detection support */

#define INTERFERNCE_THRESHOLD_DBM (-70)
#define SSIZE    25
#define IN_A_ROW  3
static int8_t  sSample[SSIZE];
static uint8_t sChannel = 0;
#endif  /* FREQUENCY_AGILITY */
#define TEST_LED_ON()           P2_0  = 1
#define TEST_LED_OFF()          P2_0  = 0
#define TEST_LED_TOG()          P2_0 ^= 1

/* blink LEDs when channel changes... */
static volatile uint8_t sBlinky = 0;

/*     ************** END interference detection support                       */

#define SPIN_ABOUT_A_QUARTER_SECOND   NWK_DELAY(250)


void ESPSendCmd(uint8 *cmd, uint8 len) {
  for(uint8 i = 0; i < len; i++) {
    U0DBUF = cmd[i];
    while(!(U0CSR & U0CSR_TX_BYTE)); // wait until transmmition done
    U0CSR &= ~U0CSR_TX_BYTE; // clear TX flag
  }
}

void ESPInit(void) {
  ESPSendCmd("AT+RST\r\n", sizeof("AT+RST\r\n")); // disable UART echo
  NWK_DELAY(5000);
  
  uartRxIndex = 0; // reset buffer index
  
  ESPSendCmd("ATE0\r\n", sizeof("ATE0\r\n")); // disable UART echo
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CWMODE=2\r\n", sizeof("AT+CWMODE=2\r\n")); // print out AP info
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CWSAP?\r\n", sizeof("AT+CWSAP?\r\n")); // print out AP info
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CIFSR\r\n", sizeof("AT+CIFSR\r\n")); // print out IP
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CIPMUX=1\r\n", sizeof("AT+CIPMUX=1\r\n")); // enable multiple connection for server mode, this must do before open server
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CIPSERVER=1,8888\r\n", sizeof("AT+CIPSERVER=1,8888\r\n")); // create a TCP server (default IP is 192.168.4.1) and listen on port 8888
  NWK_DELAY(1000);
  
  ESPSendCmd("AT+CIPSTO=7200\r\n", sizeof("AT+CIPSTO=7200\r\n")); // set server timeout = 7200s
  NWK_DELAY(1000);
  
  UART1SendArr(uartRxBuffer, uartRxIndex);
  UART1SendStr("uartRxIndex = "); UART1SendNum(uartRxIndex, 1);
  
  memset(uartRxBuffer, 0x00, sizeof(uartRxBuffer));
  uartRxIndex = 0;
}


void main (void)
{
  bspIState_t intState;

#ifdef FREQUENCY_AGILITY
  memset(sSample, 0x0, sizeof(sSample));
#endif
  
  BSP_Init();
  
  // configure UART0 for debugging, this section overlap clk config in SMPL_Init() function
  mSysConfig();
  UART0Config();
  UART1Config();
  UART1SendStr("mSysConfig done !!!\r\n");
  
  ESPInit();
  TEST_LED_TOG();NWK_DELAY(250);TEST_LED_TOG();
  UART1SendStr("ESPInit done !!!\r\n");

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

  SMPL_Init(sCB);
  UART1SendStr("SMPL_Init done !!!\r\n");

  /* main work loop */
  while (1) {
    /* manage FHSS schedule if FHSS is active */
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) );
    
    // have new node join, accpet it
    if (sJoinSem && (sNumCurrentPeers < NUM_CONNECTIONS)) {
      /* listen for a new connection */
      while (1) {
        if (SMPL_SUCCESS == SMPL_LinkListen(&sLID[sNumCurrentPeers])) {
          break;
        }
        /* Implement fail-to-link policy here. otherwise, listen again. */
        UART1SendStr("Link failed !!!\r\n");
      }
      UART1SendStr("Link success !!!\r\n");

      sNumCurrentPeers++;

      BSP_ENTER_CRITICAL_SECTION(intState);
      sJoinSem--;
      BSP_EXIT_CRITICAL_SECTION(intState);
    }

    // have data from node, feed it back 
    if (sPeerFrameSem != 0) { // have data from node
      uint8_t msg[MAX_APP_PAYLOAD], len;

      /* process all frames waiting */
      for (uint8_t i = 0; i < sNumCurrentPeers; ++i) {
        if (SMPL_SUCCESS == SMPL_Receive(sLID[i], msg, &len)) {
          TEST_LED_TOG();
          
          BSP_ENTER_CRITICAL_SECTION(intState);
          sPeerFrameSem--;
          BSP_EXIT_CRITICAL_SECTION(intState);
        }
      }
    }
    
    // button pressing for testing purpose
    if(BSP_BUTTON1() == 1) {
      NWK_DELAY(500); // debound
      
      // send to all EDs
      for (uint8_t i = 0; i < sNumCurrentPeers; ++i) {
        // send back ED something
        uint8_t msg[2]; msg[1] = ++sTid; msg[0] = 1;      
        while(1) {
          if(SMPL_SendOpt(sLID[i], msg, sizeof(msg), SMPL_TXOPTION_ACKREQ) == SMPL_SUCCESS) {
            UART1SendStr("SMPL_Send success\r\n");
            break;
          }
          
          NWK_DELAY(10);
          static uint8_t retry = 3; 
          if(--retry == 0) { // retry 3 times
            UART1SendStr("SMPL_Send failed\r\n");
            retry = 3; 
            break;
          } 
        }
      }
    }
    
    // have data package from esp8266 module
    if(uartSem == 1) {
      uartSem = 0;
      NWK_DELAY(100);
      
      uint8 rawData[8] = {'\0'};
      memcpy(rawData, (uint8 *)strstr((char const *)uartRxBuffer, ":x") + 2, 7);
      UART1SendStr("Raw data: "); UART1SendArr(rawData , 6); UART1SendStr("\r\n");

      uint8 num1 = (uint8) strtol((const char *)rawData, NULL, 16);
      uint16 num2 = (uint16) strtol((const char *)rawData + 3, NULL, 16);
      UART1SendStr("Node addr: "); UART1SendNum(num1 , 6); UART1SendStr("\r\n");
      UART1SendStr("LEDs sta: "); UART1SendNum(num2 , 6); UART1SendStr("\r\n");
            
      // send to all EDs
      for (uint8_t i = 0; i < sNumCurrentPeers; ++i) {
        // send back ED something
        uint8_t msg[3]; msg[0] = num1; msg[1] = (uint8) (num2 >> 8); msg[2] = (uint8) (num2 >> 0);
        
        while(1) {
          if(SMPL_SendOpt(sLID[i], msg, sizeof(msg), SMPL_TXOPTION_ACKREQ) == SMPL_SUCCESS) {
            UART1SendStr("SMPL_Send success\r\n");
            break;
          }
          
          NWK_DELAY(10);
          static uint8_t retry = 3; 
          if(--retry == 0) { // retry 3 times
            UART1SendStr("SMPL_Send failed\r\n");
            retry = 3; 
            break;
          } 
        }
      }
      
      free(rawData);
      uartRxIndex = 0;
    }    
  } // end of while(1)
} // end of main

/* Runs in ISR context. Reading the frame should be done in the */
/* application thread not in the ISR thread. */
static uint8_t sCB(linkID_t lid)
{
  if (lid) {
    sPeerFrameSem++;
    sBlinky = 0;
    UART1SendStr("sCB: Recv data from node have lid = ");
    UART1SendNum(lid, 1);
  } else {
    UART1SendStr("sCB: new node join !!!\r\n");
    sJoinSem++;
  }
  
  /* leave frame to be read by application. */
  return 0;
}

#ifdef FREQUENCY_AGILITY
static void changeChannel(void)
{
  freqEntry_t freq;

  if (++sChannel >= NWK_FREQ_TBL_SIZE)
  {
    sChannel = 0;
  }
  freq.logicalChan = sChannel;
  SMPL_Ioctl(IOCTL_OBJ_FREQ, IOCTL_ACT_SET, &freq);
  BSP_TURN_OFF_LED1();
  BSP_TURN_OFF_LED2();
  sBlinky = 1;
  
  return;
}

/* implement auto-channel-change policy here... */
static void  checkChangeChannel(void)
{
  int8_t dbm, inARow = 0;

  memset(sSample, 0x0, SSIZE);
  for (uint8_t i = 0; i < SSIZE; ++i) {
    /* quit if we need to service an app frame */
    if (sPeerFrameSem || sJoinSem) {
      return;
    }
    NWK_DELAY(1);
    SMPL_Ioctl(IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RSSI, (void *)&dbm);
    sSample[i] = dbm;

    if (dbm > INTERFERNCE_THRESHOLD_DBM) {
      if (++inARow == IN_A_ROW) {
        changeChannel();
        break;
      }
    } else {
      inARow = 0;
    }
  }

  return;
}
#endif

/***********************************************************************************
* @fn          UART0_RX_ISR
*
* @brief       Function which completes the UART receive session.
*
* @param       none
*
* @return      0
*/
#pragma vector = URX0_VECTOR
__interrupt void UART0_RX_ISR(void) {
  // Clear UART0 RX Interrupt Flag (TCON.URX0IF = 0)
  URX0IF = 0;

  // Read UART0 RX buffer
  uartRxBuffer[uartRxIndex] = U0DBUF;
  if(uartRxBuffer[uartRxIndex] == 'X') {
    uartRxBuffer[uartRxIndex + 1] = '\0'; // make a string to use build-in string lib
    uartSem = 1;
  }
  
  uartRxIndex++;
}
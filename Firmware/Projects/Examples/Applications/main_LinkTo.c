/*----------------------------------------------------------------------------
 *  Demo Application for SimpliciTI 
 * 
 *  L. Friedman 
 *  Texas Instruments, Inc.
 *---------------------------------------------------------------------------- */

/********************************************************************************************
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

#include "debugging.h"

static void linkTo(void);

void toggleLED(uint8_t);

static uint8_t  sTxTid=0, sRxTid=0;
static linkID_t sLinkID1 = 0;

/* application Rx frame handler. */
static uint8_t sRxCallback(linkID_t);

/* For FHSS systems, calls to NWK_DELAY() will also call nwk_pllBackgrounder()
 * during the delay time so if you use the system delay mechanism in a loop,
 * you don't need to also call the nwk_pllBackgrounder() function.
 */
#define SPIN_ABOUT_A_SECOND  NWK_DELAY(1000)

void main (void)
{
  BSP_Init();
  
  // configure UART0 for debugging, this section overlap clk config in SMPL_Init() function
  UART0Config();
  UART0SendStr("UART0Config done !!!\r\n");

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

  /* On FHSS systems the call to SMPL_Init will not return until we have
   * locked onto a reference clock.  Also, on return the radio will always
   * be on in receive mode (at least for now).
   */

  /* This call will fail because the join will fail since there is no Access Point 
   * in this scenario. But we don't care -- just use the default link token later. 
   * We supply a callback pointer to handle the message returned by the peer. 
   */
  SMPL_Init(sRxCallback);

  /* wait for a button press... */
  do {
    FHSS_ACTIVE( nwk_pllBackgrounder( false ) ); /* manage FHSS */
#ifndef SOC_BB
    if (BSP_BUTTON1() || BSP_BUTTON2())
#else
    if (BSP_BUTTON1())
#endif
    {
      break;
    }
  } while (1);

  /* never coming back... */
  UART0SendStr("Start linkTo\r\n");
  linkTo();

  /* but in case we do... */
  while (1);
}

static void linkTo()
{
  uint8_t  msg[2];

  while (SMPL_SUCCESS != SMPL_Link(&sLinkID1))
  {
    SPIN_ABOUT_A_SECOND; /* manages FHSS implicitly */
    UART0SendStr("Link failed !!!\r\n");
  }
  UART0SendStr("Link success !!!\r\n");

#ifndef FREQUENCY_HOPPING
  /* turn on RX. default is RX off. */
  SMPL_Ioctl( IOCTL_OBJ_RADIO, IOCTL_ACT_RADIO_RXON, 0);
#endif
  
  /* put LED to toggle in the message */
  msg[0] = 2;  /* toggle red */
  while (1)
  {
    /* put the sequence ID in the message */
    msg[1] = ++sTxTid;
    SMPL_Send(sLinkID1, msg, sizeof(msg));
    
    UART0SendStr("Send RF data: ");
    UART0SendNum(sTxTid, 1);
    
    NWK_DELAY(100);
  }
}

#ifdef SOC_BB
void toggleLED(uint8_t which)
{
  BSP_TOGGLE_LED1();
  return;
}
#else
void toggleLED(uint8_t which)
{
  if (1 == which)
  {
    BSP_TOGGLE_LED1();
  }
  else if (2 == which)
  {
    BSP_TOGGLE_LED2();
  }
  return;
}
#endif

/* handle received frames. */
static uint8_t sRxCallback(linkID_t port)
{
  uint8_t msg[2], len, tid;

  /* is the callback for the link ID we want to handle? */
  if (port == sLinkID1)
  {    
    /* yes. go get the frame. we know this call will succeed. */
     if ((SMPL_SUCCESS == SMPL_Receive(sLinkID1, msg, &len)) && len)
     {
       /* Check the application sequence number to detect
        * late or missing frames... 
        */
       tid = *(msg+1);
       UART0SendStr("Recv RF data: ");
       UART0SendNum(tid, 1);
       if (tid)
       {
         if (tid > sRxTid)
         {
           /* we're good. toggle LED in the message */
           toggleLED(*msg);
           sRxTid = tid;
         }
       }
       else
       {
         /* the wrap case... */
         if (sRxTid)
         {
           /* we're good. toggle LED in the message */
           toggleLED(*msg);
           sRxTid = tid;
         }
       }
       /* drop frame. we're done with it. */
       return 1;
     }
  }
  /* keep frame for later handling. */
  return 0;
}


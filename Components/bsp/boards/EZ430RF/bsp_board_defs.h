/**************************************************************************************************
  Revised:        $Date: 2007-07-06 11:19:00 -0700 (Fri, 06 Jul 2007) $
  Revision:       $Revision: 13579 $

  Copyright 2007 Texas Instruments Incorporated.  All rights reserved.

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

/* =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 *   BSP (Board Support Package)
 *   Target : Texas Instruments EZ430-RF2500
 *            "MSP430 Wireless Development Tool"
 *   Board definition file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef BSP_BOARD_DEFS_H
#define BSP_BOARD_DEFS_H


/* ------------------------------------------------------------------------------------------------
 *                                     Board Unique Define
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_BOARD_EZ430RF


/* ------------------------------------------------------------------------------------------------
 *                                           Mcu
 * ------------------------------------------------------------------------------------------------
 */
#include "mcus/bsp_msp430_defs.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Clock
 * ------------------------------------------------------------------------------------------------
 */
#include "bsp_config.h"
#define __bsp_CLOCK_MHZ__    BSP_CONFIG_CLOCK_MHZ



/* ------------------------------------------------------------------------------------------------
 *                                          Timer
 * ------------------------------------------------------------------------------------------------
 */
#ifdef FREQUENCY_HOPPING
  #ifndef NWK_PLL
    #define NWK_PLL
  #endif
#endif

#if defined NWK_PLL && !defined MRFI_TIMER_ALWAYS_ACTIVE
  #define MRFI_TIMER_ALWAYS_ACTIVE
#endif

#ifdef MRFI_TIMER_ALWAYS_ACTIVE
  #define BSP_TIMER_A3 0x4133 // 'A' and '3' characters in hex
  #define BSP_TIMER_B3 0x4233 // 'B' and '7' characters in hex

  #ifndef BSP_TIMER_USED
    #define BSP_TIMER_USED BSP_TIMER_A3
  #else // if BSP_TIMER_USED was user defined
    #if BSP_TIMER_USED != BSP_TIMER_A3 && BSP_TIMER_USED != BSP_TIMER_B3
      #error "ERROR: The selected timer is invalid, must be BSP_TIMER_A3 or BSP_TIMER_B3."
    #endif
  #endif
  /* Timer A and Timer B are similar enough in operation that either one can be used
   * for the hardware timer source.  In both cases, the timer is configured in up mode
   * and used compare register 0 for the interrupt source.  The user can utilize any
   * other compare registers and the associated interrupts and vector so long as they
   * do not interfere with the timer operation in any other form.
   */
  #define BSP_TIMER_SIZE 16
  #if BSP_TIMER_USED == BSP_TIMER_A3
    #define BSP_TIMER_VECTOR TIMERA0_VECTOR
    #define TxCTL   TACTL
    #define TxR     TAR
    #define TxCCR0  TACCR0
    #define TxCCTL0 TACCTL0
  #elif BSP_TIMER_USED == BSP_TIMER_B3
    #define BSP_TIMER_VECTOR TIMERB0_VECTOR
    #define TxCTL   TBCTL
    #define TxR     TBR
    #define TxCCR0  TBCCR0
    #define TxCCTL0 TBCCTL0
  #endif

  #define BSP_MAX_MODULATION_MAGNITUDE ( BSP_TIMER_CLK_KHZ * 10 / 100 ) // ten percent

  #define BSP_TIMER_PRESCALE_VALUE 0
  #define BSP_TIMER_PRESCALE_DIVISOR ( 1 << ( ( BSP_TIMER_PRESCALE_VALUE * 2 )\
                                       + ( ( BSP_TIMER_PRESCALE_VALUE == 0 ) ? 0 : 1 ) ) )
#ifndef BSP_CONFIG_CLOCK_KHZ
  #define BSP_TIMER_CLK_KHZ ((BSP_CLOCK_MHZ * 2000L / BSP_TIMER_PRESCALE_DIVISOR + 1)/2)
#else
  #define BSP_TIMER_CLK_KHZ ((BSP_CONFIG_CLOCK_KHZ * 2L / BSP_TIMER_PRESCALE_DIVISOR + 1)/2)
#endif

  #define BSP_CALC_LIMIT( ticks ) ( ticks )

  #define BSP_ROLLOVER_LIMIT BSP_CALC_LIMIT( BSP_TIMER_CLK_KHZ )
  #define BSP_TIMER_FREE_RUN_INIT( )\
            st( BF_SET( TxCTL, 0, 4, 2 );        /* Stop the timer */\
                TxR = 0;                         /* clear the timer count value */\
                TxCTL = BF_GEN( 0, 13, 2 )       /* no grouping of capture registers */\
                        | BF_GEN( 0, 11, 2 )     /* set counter length to 16 bits */\
                        | BF_GEN( 2, 8, 2 )      /* use smclk as source */\
                        | BF_GEN( 0, 6, 2 )      /* set prescale to 1 */\
                        | BF_GEN( 0, 4, 2 )      /* timer remains stopped */\
                        | BF_GEN( 1, 2, 1 )      /* reset the internal timer state */\
                        | BF_GEN( 0, 1, 1 )      /* disable interrupts for the moment */\
                        | BF_GEN( 0, 0, 1 );     /* clear any pending interrupt */\
                TxCCR0 = (BSP_TIMER_CLK_KHZ - 1); /* initialize count value */\
                TxCCTL0 = BF_GEN( 0, 14, 2 )     /* disable capture mode */\
                          | BF_GEN( 2, 12, 2 )   /* disable capture input signal */\
                          | BF_GEN( 0, 9, 2 )    /* update CCR on write */\
                          | BF_GEN( 0, 8, 1 )    /* use compare mode */\
                          | BF_GEN( 0, 4, 1 )    /* disable itnerrupts for the moment */\
                          | BF_GEN( 0, 1, 1 )    /* clear any pending overflow intrpts */\
                          | BF_GEN( 0, 0, 1 );   /* clear any pending interrupts */\
                TxCCTL0 |= BF_GEN( 1, 4, 1 );    /* enable the compare interrupt */\
                TxCTL |= BF_GEN( 1, 4, 2 );      /* start up the time in up mode */\
              )
  #define BSP_TIMER_CHECK_OVERFLOW_FLAG( )      ( BF_GET( TxCCTL0, 0, 1 ) != 0 )
  #define BSP_TIMER_CLEAR_OVERFLOW_FLAG( )      /* the event is cleared automatically */
  #define BSP_TIMER_MAN_CLEAR_OVERFLOW_FLAG( )  ( BF_SET( TxCCTL0, 0, 0, 1 ) )
  #define BSP_TIMER_GET_TIMER_COUNT_VALUE_LO( ) ( BF_GET( TxR, 8, 8 ) )
  #define BSP_TIMER_GET_TIMER_COUNT_VALUE_HI( ) ( BF_GET( TxR, 0, 8 ) )
  #define BSP_TIMER_GET_TIMER_COUNT( p )        ( p = TxR )
  #define BSP_TIMER_SET_OVERFLOW_VALUE( val )   ( TxCCR0 = val )

#endif // MRFI_TIMER_ALWAYS_ACTIVE

/* ------------------------------------------------------------------------------------------------
 *                                     Board Initialization
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_BOARD_C               "bsp_board.c"
#define BSP_INIT_BOARD()          BSP_InitBoard()
#define BSP_DELAY_USECS(x)        BSP_Delay(x)

void BSP_InitBoard(void);
void BSP_Delay(uint16_t usec);

/* ************************************************************************************************
 *                                   Compile Time Integrity Checks
 * ************************************************************************************************
 */
#if (defined __IAR_SYSTEMS_ICC__) && (__VER__ >= 342)
#if (!defined __MSP430F2274__)
#error "ERROR: Mismatch between specified board and selected microcontroller."
#endif
#endif


/**************************************************************************************************
 */
#endif

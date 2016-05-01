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
 *   Target : CC1111DK_Dongle/CC2511DK_Dongle
 *            "RF USB Dongle"
 *   Board definition file.
 * =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 */

#ifndef BSP_BOARD_DEFS_H
#define BSP_BOARD_DEFS_H


/* ------------------------------------------------------------------------------------------------
 *                                     Board Unique Define
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_BOARD_RFUSB


/* ------------------------------------------------------------------------------------------------
 *                                           Mcu
 * ------------------------------------------------------------------------------------------------
 */
#include "mcus/bsp_8051_defs.h"


/* ------------------------------------------------------------------------------------------------
 *                                          Clock
 * ------------------------------------------------------------------------------------------------
 */
/* RFUSB Boards with CC2511/CC1111 have 48 MHz crystals. Fxosc = 48 MHz
 * The reference frequency is half the oscillator freq. So Fref = 24 MHz.
 * The CLKCON default setting sets the system clock to half of reference clock.
 * Thus we have the system clock = 24/2 = 12 MHz.
 */
#define __bsp_CLOCK_MHZ__         12  /* MHz */


/* ------------------------------------------------------------------------------------------------
 *                                          Timer
 * ------------------------------------------------------------------------------------------------
 */

#define BSP_DELAY_USECS(x)        BSP_Delay(x)
void BSP_Delay(uint16_t);
#ifdef FREQUENCY_HOPPING
  #ifndef NWK_PLL
    #define NWK_PLL
  #endif
#endif

#if defined NWK_PLL && !defined MRFI_TIMER_ALWAYS_ACTIVE
  #define MRFI_TIMER_ALWAYS_ACTIVE
#endif

#ifdef MRFI_TIMER_ALWAYS_ACTIVE

  #ifndef BSP_TIMER_USED
    #if defined MRFI_CC2510 || defined MRFI_CC1110
      #define BSP_TIMER_USED 3
    #elif defined MRFI_CC2430 || defined MRFI_CC2431
      #define BSP_TIMER_USED 1
    #else
      #error "ERROR: Chip is unknown or not defined."
    #endif
  #else // if BSP_TIMER_USED was user defined
    #if BSP_TIMER_USED != 1 && BSP_TIMER_USED != 3 && BSP_TIMER_USED != 4
      #error "ERROR: The selected timer is invalid, must be 1, 3, or 4."
    #endif
  #endif

  #if BSP_TIMER_USED == 1
    #define BSP_TIMER_VECTOR T1_VECTOR
    #define BSP_TIMER_SIZE 16
  #elif BSP_TIMER_USED == 3
    #define BSP_TIMER_VECTOR T3_VECTOR
    #define BSP_TIMER_SIZE 8
  #elif BSP_TIMER_USED == 4
    #define BSP_TIMER_VECTOR T4_VECTOR
    #define BSP_TIMER_SIZE 8
  #endif

  #define BSP_MAX_MODULATION_MAGNITUDE ( BSP_TIMER_CLK_KHZ * 10 / 100 ) // ten percent

  #if BSP_TIMER_SIZE == 16

    #define BSP_TIMER_PRESCALE_VALUE 0
    #define BSP_TIMER_PRESCALE_DIVISOR ( 1 << ( ( BSP_TIMER_PRESCALE_VALUE * 2 )\
                                         + ( ( BSP_TIMER_PRESCALE_VALUE == 0 ) ? 0 : 1 ) ) )
    #define BSP_TIMER_CLK_KHZ ((BSP_CLOCK_MHZ * 2000L / BSP_TIMER_PRESCALE_DIVISOR + 1)/2)

    #define BSP_CALC_LIMIT( ticks ) ( ticks )

    #define BSP_ROLLOVER_LIMIT BSP_CALC_LIMIT( BSP_TIMER_CLK_KHZ )
    #define BSP_TIMER_FREE_RUN_INIT( )\
              st( BF_SET( T1CTL, 0, 0, 2 );        /* Stop the timer. */\
                  T1CNTL = T1CNTH = 0;             /* clear the timer count value */\
                  T1CTL = BF_GEN( BSP_TIMER_PRESCALE_VALUE, 2, 2 ) /* set prescale value */\
                          | BF_GEN( 0, 0, 2 )      /* timer remains stopped */\
                          | BF_GEN( 0, 7, 1 )      /* clear compare 2 interrupt */\
                          | BF_GEN( 0, 6, 1 )      /* clear compare 1 interrupt */\
                          | BF_GEN( 0, 5, 1 )      /* clear compare 0 interrupt */\
                          | BF_GEN( 0, 4, 1 );     /* clear overflow interrupt */\
                  T1CC0L = (BSP_TIMER_CLK_KHZ - 1) & 0xFF; /* initialize overflow values */\
                  T1CC0H = ( (BSP_TIMER_CLK_KHZ - 1) >> 8 ) & 0xFF;\
                  T1CCTL0 = T1CCTL1                /* for all three compare registers */\
                  = T1CCTL2 = BF_GEN( 0, 7, 1 )    /* use capture mode not RF mode */\
                              | BF_GEN( 0, 6, 1 )  /* disable compare interrupts */\
                              | BF_GEN( 0, 3, 3 )  /* choose any output mode */\
                              | BF_GEN( 0, 2, 1 ); /* disable compare mode */\
                  TIMIF |= BF_GEN( 1, 6, 1 );      /* enable overflow interrupt */\
                  IP0 = BF_GEN(1, 1, 1);           /* set RF interrupt to priority 2 and */\
                  IP1 = BF_GEN(1, 1, 1) | BF_GEN(1, 0, 1); /* timer 1 to priority 3 */\
                  T1CTL |= BF_GEN( 2, 0, 2 );      /* start up the timer in modulo mode */\
                  T1IE = 1;                        /* enable timer interrupts */\
                )
    #define BSP_TIMER_CHECK_OVERFLOW_FLAG( )      ( BF_GET( T1CTL, 4, 1 ) != 0 )
    #define BSP_TIMER_CLEAR_OVERFLOW_FLAG( )      ( T1CTL = ( T1CTL & BF_MSK( 0, 4 ) )\
                                                     | ( BF_CLR( 4, 1 ) & BF_MSK( 4, 4 ) ) )
    #define BSP_TIMER_MAN_CLEAR_OVERFLOW_FLAG( )  BSP_TIMER_CLEAR_OVERFLOW_FLAG( )
    #define BSP_TIMER_GET_TIMER_COUNT_VALUE_LO( ) ( T1CNTL )
    #define BSP_TIMER_GET_TIMER_COUNT_VALUE_HI( ) ( T1CNTH )
    #define BSP_TIMER_GET_TIMER_COUNT( p )        st( p = T1CNTL;\
                                                      p |= (uint16_t)T1CNTH << 8; )
    #define BSP_TIMER_SET_OVERFLOW_VALUE( val )   st( union { uint16_t i; uint8_t b[2]; };\
                                                      i = val;\
                                                      T1CC0L = b[0];\
                                                      T1CC0H = b[1]; )

  #elif BSP_TIMER_SIZE == 8

    // set interrupt priorities for timer 3/4 to be highest (3) and rf to be next (2)
    #define BSP_SET_INTERRUPT_PRIORITIES( ) st( IP0 = BF_GEN(1, BSP_TIMER_USED, 1);\
                                                IP1 = BF_GEN(1, BSP_TIMER_USED, 1)\
                                                      | BF_GEN(1, 0, 1); )
    #define TxCTL   INFIX( T, BSP_TIMER_USED, CTL )
    #define TxOVFIF INFIX( T, BSP_TIMER_USED, OVFIF )
    #define TxCC0   INFIX( T, BSP_TIMER_USED, CC0 )
    #define TxCCTL0 INFIX( T, BSP_TIMER_USED, CCTL0 )
    #define TxCCTL1 INFIX( T, BSP_TIMER_USED, CCTL1 )
    #define TxIE    INFIX( T, BSP_TIMER_USED, IE )
    #define TxCNT   INFIX( T, BSP_TIMER_USED, CNT )

#if __bsp_CLOCK_MHZ__ >= 16
    #define BSP_TIMER_PRESCALE_VALUE 5
#else
    #define BSP_TIMER_PRESCALE_VALUE 4
#endif

    #define BSP_TIMER_PRESCALE_DIVISOR ( 1 << BSP_TIMER_PRESCALE_VALUE )
    #define BSP_TIMER_CLK_KHZ ((BSP_CLOCK_MHZ * 2000L / BSP_TIMER_PRESCALE_DIVISOR + 1)/2)

    // we want to divide by something less than 256 here because we want some room for the
    // PLL operation to work with.  i.e. this number can grow slightly.
    #define BSP_CALC_ROLLOVERS( ticks ) ( ticks / (255-BSP_MAX_MODULATION_MAGNITUDE-2) + 1 )
    #define BSP_CALC_LIMIT( ticks, rollovers )  ( ticks / rollovers )
    #define BSP_CALC_EXTRAS( ticks, rollovers ) ( ticks % rollovers )

    #define BSP_ROLLOVER_ROLLOVERS BSP_CALC_ROLLOVERS( BSP_TIMER_CLK_KHZ )
    #define BSP_ROLLOVER_LIMIT BSP_CALC_LIMIT( BSP_TIMER_CLK_KHZ, BSP_ROLLOVER_ROLLOVERS )
    #define BSP_ROLLOVER_EXTRAS BSP_CALC_EXTRAS( BSP_TIMER_CLK_KHZ, BSP_ROLLOVER_ROLLOVERS )
    #define BSP_TIMER_FREE_RUN_INIT( )\
              st( BF_SET( TxCTL, 0, 0, 4 );        /* Stop the timer. */\
                  TxOVFIF = 0;                     /* clear overflow interrupt */\
                  TxCTL = BF_GEN( BSP_TIMER_PRESCALE_VALUE, 5, 3 ) /* set prescale value */\
                          | BF_GEN( 0, 4, 1 )      /* timer remains stopped */\
                          | BF_GEN( 1, 3, 1 )      /* overflow interrupts are enabled */\
                          | BF_GEN( 1, 2, 1 )      /* clear the count value */\
                          | BF_GEN( 2, 0, 2 );     /* operate in modulo mode */\
                  TxCC0 = BSP_ROLLOVER_LIMIT;      /* set overflow value */\
                  TxCCTL0                          /* for both compare registers */\
                  = TxCCTL1 = BF_GEN( 0, 6, 1 )    /* disable compare interrupts */\
                              | BF_GEN( 7, 3, 3 )  /* not using compare modes */\
                              | BF_GEN( 0, 2, 1 ); /* disable compare operation */\
                  BSP_SET_INTERRUPT_PRIORITIES( ); /* set priority of interrupts */\
                  TxCTL |= BF_GEN( 1, 4, 1 );      /* start up the timer in modulo mode */\
                  TxIE = 1;                        /* enable timer interrupts */\
                )
    #define BSP_TIMER_CHECK_OVERFLOW_FLAG( )       ( BF_GET( TxCTL, 4, 1 ) != 0 )
    #define BSP_TIMER_CLEAR_OVERFLOW_FLAG( )       ( TxOVFIF = 0 )
    #define BSP_TIMER_MAN_CLEAR_OVERFLOW_FLAG( )   BSP_TIMER_CLEAR_OVERFLOW_FLAG( )
    #define BSP_TIMER_GET_TIMER_COUNT_VALUE_LO( )  ( TxCNT )
    #define BSP_TIMER_GET_TIMER_COUNT_VALUE_HI( )  ( sTimerCntHi )
    #define BSP_TIMER_GET_TIMER_COUNT( p )         st( p = TxCNT;\
                                                       p |= (uint16_t)sTimerCntHi << 8; )
    #define BSP_TIMER_SET_OVERFLOW_VALUE( val )    ( TxCC0 = (val) )

  #endif

#endif // MRFI_TIMER_ALWAYS_ACTIVE

/* ------------------------------------------------------------------------------------------------
 *                                     Board Initialization
 * ------------------------------------------------------------------------------------------------
 */
#define BSP_INIT_BOARD()          BSP_InitBoard()
#define BSP_BOARD_C               "bsp_board.c"

#define BSP_TIMER_CLK_MHZ   (BSP_CLOCK_MHZ)            /* Timer is clocked at same freq */
#define BSP_DELAY_MAX_USEC  (0xFF/BSP_TIMER_CLK_MHZ)   /* 8-bit Timer-3 */

#define BSP_DELAY_USECS(x)    BSP_Delay(x)
void BSP_Delay(uint16_t);
void BSP_InitBoard(void);

/**************************************************************************************************
 */
#endif

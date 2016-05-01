/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with C-code
 *  compatible #define statements.
 *
 ***************************************************************/

/* Deviation = 126.953125 */
/* Base frequency = 433.999969 */
/* Carrier frequency = 433.999969 */
/* Channel number = 0 */
/* Carrier frequency = 433.999969 */
/* Modulated = true */
/* Modulation format = GFSK */
/* Manchester enable = false */
/* Sync word qualifier mode = 30/32 sync word bits detected */
/* Preamble count = 4 */
/* Channel spacing = 199.951172 */
/* Carrier frequency = 433.999969 */
/* Data rate = 249.939 */
/* RX filter BW = 541.666667 */
/*  = Normal mode */
/* Length config = Variable packet length mode. Packet length configured by the first byte after sync word */
/* CRC enable = true */
/* Packet length = 255 */
/* Device address = 0 */
/* Address config = No address check */
/*  = false */
/* PA ramping = false */
/* TX power = 0 */
/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with C-code
 *  compatible #define statements.
 *
 ***************************************************************/

#ifndef SMARTRF_CC1110_H
#define SMARTRF_CC1110_H

#define SMARTRF_RADIO_CC1110
#define ISM_LF

#define SMARTRF_SETTING_FSCTRL1    0x08
#define SMARTRF_SETTING_FSCTRL0    0x00
#ifdef ISM_EU
    // 869.50MHz
    #define SMARTRF_SETTING_FREQ2      0x21
    #define SMARTRF_SETTING_FREQ1      0x71
    #define SMARTRF_SETTING_FREQ0      0x7A
#else
  #ifdef ISM_US  
    // 902MHz (CHANNR=20->906MHz)
    #define SMARTRF_SETTING_FREQ2      0x22
    #define SMARTRF_SETTING_FREQ1      0xB1
    #define SMARTRF_SETTING_FREQ0      0x3B
  #else
      #ifdef ISM_LF
          // 433.92MHz
        #define SMARTRF_SETTING_FREQ2      0x10
        #define SMARTRF_SETTING_FREQ1      0xB0
        #define SMARTRF_SETTING_FREQ0      0x71
    #endif // ISM_LF
  #endif // ISM_US
#endif // ISM_EU
#define SMARTRF_SETTING_MDMCFG4    0x7B
#define SMARTRF_SETTING_MDMCFG3    0x83
#define SMARTRF_SETTING_MDMCFG2    0x13
#define SMARTRF_SETTING_MDMCFG1    0x22
#define SMARTRF_SETTING_MDMCFG0    0xF8
#define SMARTRF_SETTING_CHANNR     0x00
#define SMARTRF_SETTING_DEVIATN    0x42
#define SMARTRF_SETTING_FREND1     0xB6
#define SMARTRF_SETTING_FREND0     0x10
#define SMARTRF_SETTING_MCSM0      0x18
#define SMARTRF_SETTING_FOCCFG     0x1D
#define SMARTRF_SETTING_BSCFG      0x1C
#define SMARTRF_SETTING_AGCCTRL2   0xC7
#define SMARTRF_SETTING_AGCCTRL1   0x00
#define SMARTRF_SETTING_AGCCTRL0   0xB2
#define SMARTRF_SETTING_FSCAL3     0xEA
#define SMARTRF_SETTING_FSCAL2     0x2A
#define SMARTRF_SETTING_FSCAL1     0x00
#define SMARTRF_SETTING_FSCAL0     0x1F
#define SMARTRF_SETTING_FSTEST     0x59
#define SMARTRF_SETTING_TEST2      0x81
#define SMARTRF_SETTING_TEST1      0x35
#define SMARTRF_SETTING_TEST0      0x09
#define SMARTRF_SETTING_FIFOTHR    0x47
#define SMARTRF_SETTING_IOCFG2     0x29
#define SMARTRF_SETTING_IOCFG0D    0x06
#define SMARTRF_SETTING_PKTCTRL1   0x04
#define SMARTRF_SETTING_PKTCTRL0   0x05
#define SMARTRF_SETTING_ADDR       0x00
#define SMARTRF_SETTING_PKTLEN     0xFF

#endif
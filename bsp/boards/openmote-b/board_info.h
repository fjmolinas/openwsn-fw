/**
 * Author: Xavier Vilajosana (xvilajosana@eecs.berkeley.edu)
 *         Pere Tuset (peretuset@openmote.com)
 * Modified: Tengfei Chang (tengfei.chang@eecs.berkeley.edu)
 * Date:   July 2013
 * Description: CC2538-specific board information bsp module.
 */

#ifndef __BOARD_INFO_H
#define __BOARD_INFO_H

#include <stdint.h>
#include <string.h>

#include <source/cpu.h>
#include <source/interrupt.h>

//=========================== defines =========================================

//===== interrupt state

#define INTERRUPT_DECLARATION()
#define DISABLE_INTERRUPTS() IntMasterDisable()

#define ENABLE_INTERRUPTS() IntMasterEnable()

//===== timer

#define PORT_TIMER_WIDTH                    uint32_t
#define PORT_RADIOTIMER_WIDTH               uint32_t

#define PORT_SIGNED_INT_WIDTH               int32_t
#define PORT_TICS_PER_MS                    33
#define PORT_US_PER_TICK                    30 // number of us per 32kHz clock tick
// on GINA, we use the comparatorA interrupt for the OS
#define SCHEDULER_WAKEUP()
#define SCHEDULER_ENABLE_INTERRUPT()

// this is a workaround from the fact that the interrupt pin for the GINA radio
// is not connected to a pin on the MSP which allows time capture.
#define CAPTURE_TIME()

/* sleep timer interrupt */
#define HAL_INT_PRIOR_ST        (4 << 5)
/* MAC interrupts */
#define HAL_INT_PRIOR_MAC       (4 << 5)
/* UART interrupt */
#define HAL_INT_PRIOR_UART      (5 << 5)

//===== pinout

// [P4.7] radio SLP_TR_CNTL
#define PORT_PIN_RADIO_SLP_TR_CNTL_HIGH()
#define PORT_PIN_RADIO_SLP_TR_CNTL_LOW()
// radio reset line
// on cc2538, the /RST line is not connected to the uC
#define PORT_PIN_RADIO_RESET_HIGH()    // nothing
#define PORT_PIN_RADIO_RESET_LOW()     // nothing

//===== Slot Information
/*
    Indexes for slot templates:
    
    10 : 10 ms tempalte
    20 : 20 ms template for cc2538 24ghz radio --> Default
    41 : 40 ms template for cc2538 24ghz radio
    42 : 40 ms template for Atmel FSK FEC radio
    43 : 40 ms template for Atmel OFSM1 radios MCS 0,1,2, and 3
*/
#define SLOT_TEMPLATE 20                

//===== Radios

// Number of available radios
#define MAX_RADIOS  7

//===== IEEE802154E timing

#if SLOT_TEMPLATE==10
    // time-slot related
    #define SLOTDURATION                        10     // ms 
    #define PORT_TsSlotDuration                 328   // counter counts one extra count, see datasheet
    // execution speed related
    #define PORT_maxTxDataPrepare               10    //  305us (measured  82us)
    #define PORT_maxRxAckPrepare                10    //  305us (measured  83us)
    #define PORT_maxRxDataPrepare                4    //  122us (measured  22us)
    #define PORT_maxTxAckPrepare                10    //  122us (measured  94us)
    // radio speed related
    #ifdef L2_SECURITY_ACTIVE
    #define PORT_delayTx                        14    //  366us (measured xxxus)
    #else
    #define PORT_delayTx                        12    //  366us (measured xxxus)
    #endif
    #define PORT_delayRx                         0    //    0us (can not measure)
    // radio watchdog
#endif

#if SLOT_TEMPLATE==20
    #define SLOTDURATION                        20     // ms  
    #define PORT_TsSlotDuration                1310 //655   //    20ms

    // execution speed related
    #define PORT_maxTxDataPrepare               110   //  3355us (not measured)
    #define PORT_maxRxAckPrepare                20    //   610us (not measured)
    #define PORT_maxRxDataPrepare               33    //  1000us (not measured)
    #define PORT_maxTxAckPrepare                50    //  1525us (not measured)

    // radio speed related
    #define PORT_delayTx                        18    //   549us (not measured)
    #define PORT_delayRx                        0     //     0us (can not measure)
#endif

//40ms slot for 24ghz cc2538
#if SLOT_TEMPLATE==41
    #define SLOTDURATION                        40     // ms 
    #define PORT_TsSlotDuration                1310    // 40ms

    // execution speed related
    #define PORT_maxTxDataPrepare               10    //  305us 
    #define PORT_maxRxAckPrepare                10    //  305us 
    #define PORT_maxRxDataPrepare                4    //  122us 
    #define PORT_maxTxAckPrepare                10    //  305us 
    // radio speed related
    #ifdef L2_SECURITY_ACTIVE
    #define PORT_delayTx                        14    //  366us (measured xxxus)
    #else
    #define PORT_delayTx                        12    //  366us (measured xxxus)
    #endif
    #define PORT_delayRx                         0    //    0us (can not measure)
    // radio watchdog
#endif

//40ms slot for FSK    
#if SLOT_TEMPLATE==42 
    #define SLOTDURATION                        40     // ms  
    #define PORT_TsSlotDuration                 1310   // 40ms 

    // execution speed related
    #define PORT_maxTxDataPrepare               50      // 1525 us  (based on measurement)
    #define PORT_maxRxAckPrepare                10      // 305µs (based on measurement)
    #define PORT_maxRxDataPrepare               10      // 305µs  (based on measurement)
    #define PORT_maxTxAckPrepare                33      // 1000µs  (based on measurement)
    // radio speed related
    #define PORT_delayTx                        66      // 2000µs  (based on measurement)
    #define PORT_delayRx                        16      // 488µs. This parameter is usually set to 0, however for Atmel on openmote-b, it takes at least 1ms for the transmission to occure because of spi delay (or other implementation specific overhead), so reciver is expected to wait a little more before turning on the radio. 
#endif


//40ms slot for OFDM1 MCS0-3
#if SLOT_TEMPLATE==43 
    #define SLOTDURATION                        40     // ms  
    #define PORT_TsSlotDuration                 1310   // 40ms

    // execution speed related
    #define PORT_maxTxDataPrepare               50          //1525us (based on measurement) 
    #define PORT_maxRxAckPrepare                10          //305us (based on measurement) 
    #define PORT_maxRxDataPrepare               10          //305us (based on measurement) 
    #define PORT_maxTxAckPrepare                33          //1000us (based on measurement) 
    // radio speed related
    #define PORT_delayTx                        41          //1251us (based on measurement)  
    #define PORT_delayRx                        16          // 488µs. This parameter is usually set to 0, however for Atmel on openmote-b, it takes at least 1ms for the transmission to occure because of spi delay (or other implementation specific overhead), so reciver is expected to wait a little more before turning on the radio. 
#endif


//===== adaptive_sync accuracy

#define SYNC_ACCURACY                       1     // ticks

//===== per-board number of sensors

#define NUMSENSORS      7

//====== Antenna options ====
#define BSP_ANTENNA_BASE            GPIO_D_BASE
#define BSP_ANTENNA_CC2538_24GHZ    GPIO_PIN_4      //!< PD4 -- 2.4ghz
#define BSP_ANTENNA_AT215_24GHZ     GPIO_PIN_3      //!< PD3 -- subghz
//#define DAGROOT

//=========================== typedef  ========================================

//=========================== variables =======================================

static const uint8_t rreg_uriquery[]        = "h=ucb";
static const uint8_t infoBoardname[]        = "openmote-b";
static const uint8_t infouCName[]           = "CC2538";
static const uint8_t infoRadioName[]        = "CC2538 SoC";

//=========================== prototypes ======================================

//=========================== public ==========================================
void eraseFlash(void);
//=========================== private =========================================

#endif

// **********************************************************************************
// Driver definition for HopeRF RFM69W/RFM69HW/RFM69CW/RFM69HCW, Semtech SX1231/1231H
// **********************************************************************************
// Copyright Felix Rusu (2014), felix@lowpowerlab.com
// http://lowpowerlab.com/
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it
// and/or modify it under the terms of the GNU General
// Public License as published by the Free Software
// Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE. See the GNU General Public
// License for more details.
//
// You should have received a copy of the GNU General
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//
// Licence can be viewed at
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************
#ifndef RFM69_h
#define RFM69_h

#include "fb_common.h"

#define RF69_MAX_DATA_LEN       61 // to take advantage of the built in AES/CRC we want to limit the frame size to the internal FIFO size (66 bytes - 3 bytes overhead - 2 bytes crc)

#define CSMA_LIMIT              -90 // upper RX signal sensitivity threshold in dBm for carrier sense access
#define RF69_MODE_SLEEP         0 // XTAL OFF
#define RF69_MODE_STANDBY       1 // XTAL ON
#define RF69_MODE_SYNTH         2 // PLL ON
#define RF69_MODE_RX            3 // RX MODE
#define RF69_MODE_TX            4 // TX MODE

// available frequency bands
#define RF69_315MHZ            31 // non trivial values to avoid misconfiguration
#define RF69_433MHZ            43
#define RF69_868MHZ            86
#define RF69_915MHZ            91

#define null                  0
#define COURSE_TEMP_COEF    -90 // puts the temperature reading in the ballpark, user can fine tune the returned value
#define RF69_BROADCAST_ADDR 255
#define RF69_CSMA_LIMIT_MS 1000
#define RF69_TX_LIMIT_MS   1000
#define RF69_FSTEP  61.03515625 // == FXOSC / 2^19 = 32MHz / 2^19 (p13 in datasheet)

//void RFM69_contruct(uint8_t slaveSelectPin/*=RF69_SPI_CS*/, uint8_t interruptPin/*=RF69_IRQ_PIN*/, bool isRFM69HW/*=false*/, uint8_t interruptNum/*=RF69_IRQ_NUM*/);
void RFM69_construct(bool isRFM69HW);
//bool RFM69_initialize(uint8_t freqBand, uint8_t ID, uint8_t networkID/*=1*/);
bool RFM69_initialize(void);
void RFM69_setAddress(uint8_t addr);
void RFM69_setNetwork(uint8_t networkID);
bool RFM69_canSend();
void RFM69_send(uint8_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK/*=false*/);
bool RFM69_sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries/*=2*/, uint8_t retryWaitTime/*=40*/); // 40ms roundtrip req for 61byte packets
bool RFM69_receiveDone();
bool RFM69_ACKReceived(uint8_t fromNodeID);
bool RFM69_ACKRequested();
void RFM69_sendACK(const void* buffer/* = ""*/, uint8_t bufferSize/*=0*/);
uint32_t RFM69_getFrequency();
void RFM69_setFrequency(uint32_t freqHz);
void RFM69_encrypt(const char* key);
int16_t RFM69_readRSSI(bool forceTrigger/*=false*/);
void RFM69_promiscuous(bool onOff/*=true*/);
void RFM69_setHighPower(bool onOFF/*=true*/); // has to be called after initialize() for RFM69HW
void RFM69_setPowerLevel(uint8_t level); // reduce/increase transmit power level
void RFM69_sleep();
uint8_t RFM69_readTemperature(uint8_t calFactor/*=0*/); // get CMOS temperature (8bit)
void RFM69_rcCalibration(); // calibrate the internal RC oscillator for use in wide temperature variations - see datasheet section [4.3.5. RC Timer Accuracy]

uint8_t const * RFM69_getDataPtr(void);
uint8_t RFM69_getDataLen(void);
uint8_t RFM69_getSenderID(void);
uint8_t RFM69_getTargetID(void);
int16_t RFM69_getRSSI(void);

// allow hacking registers by making these public
uint8_t RFM69_readReg(uint8_t addr);
void RFM69_writeReg(uint8_t addr, uint8_t val);
void RFM69__readAllRegs();

void RFM69_interruptHandler();
uint8_t RFM69_SPI_transfer(uint8_t out);
void RFM69_SPI_select();
void RFM69_SPI_unselect();

#endif

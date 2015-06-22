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
#include <stdio.h>

#include "RFM69.h"
#include "RFM69registers.h"

static volatile uint8_t DATA[RF69_MAX_DATA_LEN]; // recv/xmit buf, including header & crc bytes
static volatile uint8_t DATALEN;
static volatile uint8_t SENDERID;
static volatile uint8_t TARGETID; // should match _address
static volatile uint8_t PAYLOADLEN;
static volatile uint8_t ACK_REQUESTED;
static volatile uint8_t ACK_RECEIVED; // should be polled immediately after sending a packet with ACK request
static volatile int16_t RSSI; // most accurate RSSI during reception (closest to the reception)
static volatile uint8_t _mode; // should be protected?

static uint8_t _address;
static bool _promiscuousMode;
static uint8_t _powerLevel;
static bool _isRFM69HW;

static uint8_t SPI_transfer(uint8_t out)
{   
  // clear the interrupt flag
  SPIF0 = 0;

  // write the address
  SPI0DAT = out;

  while (!SPIF0)
  {
    // wait for the xfer to complete
  }
  
  return SPI0DAT;
}

void RFM69_sendFrame(uint8_t toAddress, const void* buffer, uint8_t size, bool requestACK/*=false*/, bool sendACK/*=false*/);

void RFM69_receiveBegin();
void RFM69_setMode(uint8_t mode);
void RFM69_setHighPowerRegs(bool onOff);
void RFM69_select();
void RFM69_unselect();

void RFM69_construct(/*uint8_t slaveSelectPin, uint8_t interruptPin,*/ bool isRFM69HW/*, uint8_t interruptNum*/)
{
    _mode = RF69_MODE_STANDBY;
    _promiscuousMode = false;
    _powerLevel = 31;
    _isRFM69HW = isRFM69HW;
}

bool RFM69_initialize(/*uint8_t freqBand, uint8_t nodeID, uint8_t networkID*/)
{
  uint8_t CONFIG[][2] =
  {
    /* 0x01 */ { REG_OPMODE, RF_OPMODE_SEQUENCER_ON | RF_OPMODE_LISTEN_OFF | RF_OPMODE_STANDBY },
    /* 0x02 */ { REG_DATAMODUL, RF_DATAMODUL_DATAMODE_PACKET | RF_DATAMODUL_MODULATIONTYPE_FSK | RF_DATAMODUL_MODULATIONSHAPING_00 }, // no shaping
    /* 0x03 */ { REG_BITRATEMSB, RF_BITRATEMSB_55555}, // default: 4.8 KBPS
    /* 0x04 */ { REG_BITRATELSB, RF_BITRATELSB_55555},
    /* 0x05 */ { REG_FDEVMSB, RF_FDEVMSB_50000}, // default: 5KHz, (FDEV + BitRate / 2 <= 500KHz)
    /* 0x06 */ { REG_FDEVLSB, RF_FDEVLSB_50000},

//    /* 0x07 */ { REG_FRFMSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMSB_315 : (freqBand==RF69_433MHZ ? RF_FRFMSB_433 : (freqBand==RF69_868MHZ ? RF_FRFMSB_868 : RF_FRFMSB_915))) },
//    /* 0x08 */ { REG_FRFMID, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFMID_315 : (freqBand==RF69_433MHZ ? RF_FRFMID_433 : (freqBand==RF69_868MHZ ? RF_FRFMID_868 : RF_FRFMID_915))) },
//    /* 0x09 */ { REG_FRFLSB, (uint8_t) (freqBand==RF69_315MHZ ? RF_FRFLSB_315 : (freqBand==RF69_433MHZ ? RF_FRFLSB_433 : (freqBand==RF69_868MHZ ? RF_FRFLSB_868 : RF_FRFLSB_915))) },
    /* 0x07 */ { REG_FRFMSB, (uint8_t) RF_FRFMSB_315 },
    /* 0x08 */ { REG_FRFMID, (uint8_t) RF_FRFMID_315 },
    /* 0x09 */ { REG_FRFLSB, (uint8_t) RF_FRFLSB_315 },

    // looks like PA1 and PA2 are not implemented on RFM69W, hence the max output power is 13dBm
    // +17dBm and +20dBm are possible on RFM69HW
    // +13dBm formula: Pout = -18 + OutputPower (with PA0 or PA1**)
    // +17dBm formula: Pout = -14 + OutputPower (with PA1 and PA2)**
    // +20dBm formula: Pout = -11 + OutputPower (with PA1 and PA2)** and high power PA settings (section 3.3.7 in datasheet)
    ///* 0x11 */ { REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | RF_PALEVEL_OUTPUTPOWER_11111},
    ///* 0x13 */ { REG_OCP, RF_OCP_ON | RF_OCP_TRIM_95 }, // over current protection (default is 95mA)

    // RXBW defaults are { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_5} (RxBw: 10.4KHz)
    /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_16 | RF_RXBW_EXP_2 }, // (BitRate < 2 * RxBw)
    //for BR-19200: /* 0x19 */ { REG_RXBW, RF_RXBW_DCCFREQ_010 | RF_RXBW_MANT_24 | RF_RXBW_EXP_3 },
    /* 0x25 */ { REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01 }, // DIO0 is the only IRQ we're using
    /* 0x26 */ { REG_DIOMAPPING2, RF_DIOMAPPING2_CLKOUT_OFF }, // DIO5 ClkOut disable for power saving
    /* 0x28 */ { REG_IRQFLAGS2, RF_IRQFLAGS2_FIFOOVERRUN }, // writing to this bit ensures that the FIFO & status flags are reset
    /* 0x29 */ { REG_RSSITHRESH, 220 }, // must be set to dBm = (-Sensitivity / 2), default is 0xE4 = 228 so -114dBm
    ///* 0x2D */ { REG_PREAMBLELSB, RF_PREAMBLESIZE_LSB_VALUE } // default 3 preamble bytes 0xAAAAAA
    /* 0x2E */ { REG_SYNCCONFIG, RF_SYNC_ON | RF_SYNC_FIFOFILL_AUTO | RF_SYNC_SIZE_2 | RF_SYNC_TOL_0 },
    /* 0x2F */ { REG_SYNCVALUE1, 0x2D },      // attempt to make this compatible with sync1 byte of RFM12B lib
//    /* 0x30 */ { REG_SYNCVALUE2, networkID }, // NETWORK ID
    /* 0x30 */ { REG_SYNCVALUE2, NETWORKID }, // NETWORK ID
    /* 0x37 */ { REG_PACKETCONFIG1, RF_PACKET1_FORMAT_VARIABLE | RF_PACKET1_DCFREE_OFF | RF_PACKET1_CRC_ON | RF_PACKET1_CRCAUTOCLEAR_ON | RF_PACKET1_ADRSFILTERING_OFF },
    /* 0x38 */ { REG_PAYLOADLENGTH, 66 }, // in variable length mode: the max frame size, not used in TX
    ///* 0x39 */ { REG_NODEADRS, nodeID }, // turned off because we're not using address filtering
    /* 0x3C */ { REG_FIFOTHRESH, RF_FIFOTHRESH_TXSTART_FIFONOTEMPTY | RF_FIFOTHRESH_VALUE }, // TX on FIFO not empty
    /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_2BITS | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    //for BR-19200: /* 0x3D */ { REG_PACKETCONFIG2, RF_PACKET2_RXRESTARTDELAY_NONE | RF_PACKET2_AUTORXRESTART_ON | RF_PACKET2_AES_OFF }, // RXRESTARTDELAY must match transmitter PA ramp-down time (bitrate dependent)
    /* 0x6F */ { REG_TESTDAGC, RF_DAGC_IMPROVED_LOWBETA0 }, // run DAGC continuously in RX mode for Fading Margin Improvement, recommended default for AfcLowBetaOn=0
    {255, 0}
  };

  // put the HopeRF into reset
  PIN_RFM69HW_RST_O = 1;
  Sleep(100);

  // take the HopeRF out of reset
  PIN_RFM69HW_RST_O = 0;
  Sleep(100);

  do RFM69_writeReg(REG_SYNCVALUE1, 0xAA); while (RFM69_readReg(REG_SYNCVALUE1) != 0xAA);
  do RFM69_writeReg(REG_SYNCVALUE1, 0x55); while (RFM69_readReg(REG_SYNCVALUE1) != 0x55);

  {
    uint8_t i = 0;
     
    for (i = 0; CONFIG[i][0] != 255; i++)
      RFM69_writeReg(CONFIG[i][0], CONFIG[i][1]);
  }

  // Encryption is persistent between resets and can trip you up during debugging.
  // Disable it during initialization so we always start from a known state.
  RFM69_encrypt(0);

  RFM69_setHighPower(_isRFM69HW); // called regardless if it's a RFM69W or RFM69HW
  RFM69_setMode(RF69_MODE_STANDBY);
  while ((RFM69_readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady

//  _address = nodeID;
  _address = NODEID;
  return true;
}

// return the frequency (in Hz)
uint32_t RFM69_getFrequency()
{
  return RF69_FSTEP * (((uint32_t) RFM69_readReg(REG_FRFMSB) << 16) + ((uint16_t) RFM69_readReg(REG_FRFMID) << 8) + RFM69_readReg(REG_FRFLSB));
}

// set the frequency (in Hz)
void RFM69_setFrequency(uint32_t freqHz)
{
  uint8_t oldMode = _mode;
  if (oldMode == RF69_MODE_TX) {
    RFM69_setMode(RF69_MODE_RX);
  }
  freqHz /= RF69_FSTEP; // divide down by FSTEP to get FRF
  RFM69_writeReg(REG_FRFMSB, freqHz >> 16);
  RFM69_writeReg(REG_FRFMID, freqHz >> 8);
  RFM69_writeReg(REG_FRFLSB, freqHz);
  if (oldMode == RF69_MODE_RX) {
    RFM69_setMode(RF69_MODE_SYNTH);
  }
  RFM69_setMode(oldMode);
}

void RFM69_setMode(uint8_t newMode)
{
  if (newMode == _mode)
    return;

  switch (newMode) {
    case RF69_MODE_TX:
      RFM69_writeReg(REG_OPMODE, (RFM69_readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_TRANSMITTER);
      if (_isRFM69HW) RFM69_setHighPowerRegs(true);
      break;
    case RF69_MODE_RX:
      RFM69_writeReg(REG_OPMODE, (RFM69_readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_RECEIVER);
      if (_isRFM69HW) RFM69_setHighPowerRegs(false);
      break;
    case RF69_MODE_SYNTH:
      RFM69_writeReg(REG_OPMODE, (RFM69_readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SYNTHESIZER);
      break;
    case RF69_MODE_STANDBY:
      RFM69_writeReg(REG_OPMODE, (RFM69_readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_STANDBY);
      break;
    case RF69_MODE_SLEEP:
      RFM69_writeReg(REG_OPMODE, (RFM69_readReg(REG_OPMODE) & 0xE3) | RF_OPMODE_SLEEP);
      break;
    default:
      return;
  }

  // we are using packet mode, so this check is not really needed
  // but waiting for mode ready is necessary when going from sleep because the FIFO may not be immediately available from previous mode
  while (_mode == RF69_MODE_SLEEP && (RFM69_readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady

  _mode = newMode;
}

//put transceiver in sleep mode to save battery - to wake or resume receiving just call receiveDone()
void RFM69_sleep() {
  RFM69_setMode(RF69_MODE_SLEEP);
}

//set this node's address
void RFM69_setAddress(uint8_t addr)
{
  _address = addr;
  RFM69_writeReg(REG_NODEADRS, _address);
}

//set this node's network id
void RFM69_setNetwork(uint8_t networkID)
{
  RFM69_writeReg(REG_SYNCVALUE2, networkID);
}

// set *transmit/TX* output power: 0=min, 31=max
// this results in a "weaker" transmitted signal, and directly results in a lower RSSI at the receiver
// the power configurations are explained in the SX1231H datasheet (Table 10 on p21; RegPaLevel p66): http://www.semtech.com/images/datasheet/sx1231h.pdf
// valid powerLevel parameter values are 0-31 and result in a directly proportional effect on the output/transmission power
// this function implements 2 modes as follows:
//       - for RFM69W the range is from 0-31 [-18dBm to 13dBm] (PA0 only on RFIO pin)
//       - for RFM69HW the range is from 0-31 [5dBm to 20dBm]  (PA1 & PA2 on PA_BOOST pin & high Power PA settings - see section 3.3.7 in datasheet, p22)
void RFM69_setPowerLevel(uint8_t powerLevel)
{
  _powerLevel = (powerLevel > 31 ? 31 : powerLevel);
  if (_isRFM69HW) _powerLevel /= 2;
  RFM69_writeReg(REG_PALEVEL, (RFM69_readReg(REG_PALEVEL) & 0xE0) | _powerLevel);
}

bool RFM69_canSend()
{
  if (_mode == RF69_MODE_RX && PAYLOADLEN == 0 && RFM69_readRSSI(false) < CSMA_LIMIT) // if signal stronger than -100dBm is detected assume channel activity
  {
    RFM69_setMode(RF69_MODE_STANDBY);
    return true;
  }
  return false;
}

void RFM69_send(uint8_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK)
{
  uint32_t now = 0;
    
  RFM69_writeReg(REG_PACKETCONFIG2, (RFM69_readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  now = millis();
  while (!RFM69_canSend() && millis() - now < RF69_CSMA_LIMIT_MS) RFM69_receiveDone();
  RFM69_sendFrame(toAddress, buffer, bufferSize, requestACK, false);
}

// to increase the chance of getting a packet across, call this function instead of send
// and it handles all the ACK requesting/retrying for you :)
// The only twist is that you have to manually listen to ACK requests on the other side and send back the ACKs
// The reason for the semi-automaton is that the lib is interrupt driven and
// requires user action to read the received data and decide what to do with it
// replies usually take only 5..8ms at 50kbps@915MHz
bool RFM69_sendWithRetry(uint8_t toAddress, const void* buffer, uint8_t bufferSize, uint8_t retries, uint8_t retryWaitTime) {
  uint32_t sentTime;
  uint8_t i = 0;
  for (i = 0; i <= retries; i++)
  {
    RFM69_send(toAddress, buffer, bufferSize, true);
    sentTime = millis();
    while (millis() - sentTime < retryWaitTime)
    {
      if (RFM69_ACKReceived(toAddress))
      {
        //Serial.print(" ~ms:"); Serial.print(millis() - sentTime);
        return true;
      }
    }
    //Serial.print(" RETRY#"); Serial.println(i + 1);
  }
  return false;
}

// should be polled immediately after sending a packet with ACK request
bool RFM69_ACKReceived(uint8_t fromNodeID) {
  if (RFM69_receiveDone())
    return (SENDERID == fromNodeID || fromNodeID == RF69_BROADCAST_ADDR) && ACK_RECEIVED;
  return false;
}

// check whether an ACK was requested in the last received packet (non-broadcasted packet)
bool RFM69_ACKRequested() {
  return ACK_REQUESTED && (TARGETID != RF69_BROADCAST_ADDR);
}

// should be called immediately after reception in case sender wants ACK
void RFM69_sendACK(const void* buffer, uint8_t bufferSize) {
  uint32_t now = 0;
  uint8_t sender = SENDERID;
  int16_t _RSSI = RSSI; // save payload received RSSI value
  RFM69_writeReg(REG_PACKETCONFIG2, (RFM69_readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  now = millis();
  while (!RFM69_canSend() && millis() - now < RF69_CSMA_LIMIT_MS) RFM69_receiveDone();
  RFM69_sendFrame(sender, buffer, bufferSize, false, true);
  RSSI = _RSSI; // restore payload RSSI
}

// internal function
void RFM69_sendFrame(uint8_t toAddress, const void* buffer, uint8_t bufferSize, bool requestACK, bool sendACK)
{
  uint32_t txStart = 0;
  uint8_t CTLbyte = 0;
    
  RFM69_setMode(RF69_MODE_STANDBY); // turn off receiver to prevent reception while filling fifo
  while ((RFM69_readReg(REG_IRQFLAGS1) & RF_IRQFLAGS1_MODEREADY) == 0x00); // wait for ModeReady
  RFM69_writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_00); // DIO0 is "Packet Sent"
  if (bufferSize > RF69_MAX_DATA_LEN) bufferSize = RF69_MAX_DATA_LEN;

  // control byte
  CTLbyte = 0x00;
  if (sendACK)
    CTLbyte = 0x80;
  else if (requestACK)
    CTLbyte = 0x40;

  // write to FIFO
  RFM69_select();
  SPI_transfer(REG_FIFO | 0x80);
  SPI_transfer(bufferSize + 3);
  SPI_transfer(toAddress);
  SPI_transfer(_address);
  SPI_transfer(CTLbyte);

  {
    uint8_t i = 0;
    for (i = 0; i < bufferSize; i++)
      SPI_transfer(((uint8_t*) buffer)[i]);
  }
  RFM69_unselect();

  // no need to wait for transmit mode to be ready since its handled by the radio
  RFM69_setMode(RF69_MODE_TX);
  txStart = millis();
  while (PIN_RFM69HW_DIO0_I == 0 && millis() - txStart < RF69_TX_LIMIT_MS); // wait for DIO0 to turn HIGH signalling transmission finish
  //while (readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PACKETSENT == 0x00); // wait for ModeReady
  RFM69_setMode(RF69_MODE_STANDBY);
}

// internal function - interrupt gets called when a packet is received
void RFM69_interruptHandler() {
  //pinMode(4, OUTPUT);
  //digitalWrite(4, 1);
  if (_mode == RF69_MODE_RX && (RFM69_readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY))
  {
    uint8_t i = 0;
    uint8_t CTLbyte = 0;
      
    //RSSI = readRSSI();
    RFM69_setMode(RF69_MODE_STANDBY);
    RFM69_select();
    SPI_transfer(REG_FIFO & 0x7F);
    PAYLOADLEN = SPI_transfer(0);
    PAYLOADLEN = PAYLOADLEN > 66 ? 66 : PAYLOADLEN; // precaution
    TARGETID = SPI_transfer(0);
    if(!(_promiscuousMode || TARGETID == _address || TARGETID == RF69_BROADCAST_ADDR) // match this node's address, or broadcast address or anything in promiscuous mode
       || PAYLOADLEN < 3) // address situation could receive packets that are malformed and don't fit this libraries extra fields
    {
      PAYLOADLEN = 0;
      RFM69_unselect();
      RFM69_receiveBegin();
      //digitalWrite(4, 0);
      return;
    }

    DATALEN = PAYLOADLEN - 3;
    SENDERID = SPI_transfer(0);
    CTLbyte = SPI_transfer(0);

    ACK_RECEIVED = CTLbyte & 0x80; // extract ACK-received flag
    ACK_REQUESTED = CTLbyte & 0x40; // extract ACK-requested flag

    for (i = 0; i < DATALEN; i++)
    {
      DATA[i] = SPI_transfer(0);
    }
    if (DATALEN < RF69_MAX_DATA_LEN) DATA[DATALEN] = 0; // add null at end of string
    RFM69_unselect();
    RFM69_setMode(RF69_MODE_RX);
  }
  RSSI = RFM69_readRSSI(false);
  //digitalWrite(4, 0);
}

// internal function
void RFM69_receiveBegin() {
  DATALEN = 0;
  SENDERID = 0;
  TARGETID = 0;
  PAYLOADLEN = 0;
  ACK_REQUESTED = 0;
  ACK_RECEIVED = 0;
  RSSI = 0;
  if (RFM69_readReg(REG_IRQFLAGS2) & RF_IRQFLAGS2_PAYLOADREADY)
    RFM69_writeReg(REG_PACKETCONFIG2, (RFM69_readReg(REG_PACKETCONFIG2) & 0xFB) | RF_PACKET2_RXRESTART); // avoid RX deadlocks
  RFM69_writeReg(REG_DIOMAPPING1, RF_DIOMAPPING1_DIO0_01); // set DIO0 to "PAYLOADREADY" in receive mode
  RFM69_setMode(RF69_MODE_RX);
}

// checks if a packet was received and/or puts transceiver in receive (ie RX or listen) mode
bool RFM69_receiveDone() {
//ATOMIC_BLOCK(ATOMIC_FORCEON)
//{
  noInterrupts(); // re-enabled in unselect() via setMode() or via receiveBegin()
  if (_mode == RF69_MODE_RX && PAYLOADLEN > 0)
  {
    RFM69_setMode(RF69_MODE_STANDBY); // enables interrupts
    return true;
  }
  else if (_mode == RF69_MODE_RX) // already in RX no payload yet
  {
    interrupts(); // explicitly re-enable interrupts
    return false;
  }
  RFM69_receiveBegin();
  return false;
//}
}

// To enable encryption: radio.encrypt("ABCDEFGHIJKLMNOP");
// To disable encryption: radio.encrypt(null) or radio.encrypt(0)
// KEY HAS TO BE 16 bytes !!!
void RFM69_encrypt(const char* key) {
  RFM69_setMode(RF69_MODE_STANDBY);
  if (key != 0)
  {
    uint8_t i = 0;
      
    RFM69_select();
    SPI_transfer(REG_AESKEY1 | 0x80);
    for (i = 0; i < 16; i++)
      SPI_transfer(key[i]);
    RFM69_unselect();
  }
  RFM69_writeReg(REG_PACKETCONFIG2, (RFM69_readReg(REG_PACKETCONFIG2) & 0xFE) | (key ? 1 : 0));
}

// get the received signal strength indicator (RSSI)
int16_t RFM69_readRSSI(bool forceTrigger) {
  int16_t rssi = 0;
  if (forceTrigger)
  {
    // RSSI trigger not needed if DAGC is in continuous mode
    RFM69_writeReg(REG_RSSICONFIG, RF_RSSI_START);
    while ((RFM69_readReg(REG_RSSICONFIG) & RF_RSSI_DONE) == 0x00); // wait for RSSI_Ready
  }
  rssi = -RFM69_readReg(REG_RSSIVALUE);
  rssi >>= 1;
  return rssi;
}

uint8_t RFM69_readReg(uint8_t addr)
{
  uint8_t regval = 0;
    
  RFM69_select();
  SPI_transfer(addr & 0x7F);
  regval = SPI_transfer(0);
  RFM69_unselect();
  return regval;
}

void RFM69_writeReg(uint8_t addr, uint8_t value)
{
  RFM69_select();
  SPI_transfer(addr | 0x80);
  SPI_transfer(value);
  RFM69_unselect();
}

// select the RFM69 transceiver (save SPI settings, set CS low)
void RFM69_select() {
  noInterrupts();
  NSS0MD0 = LOW;
}

// unselect the RFM69 transceiver (set CS high, restore SPI settings)
void RFM69_unselect() {
  NSS0MD0 = HIGH;
  interrupts();
}

// true  = disable filtering to capture all frames on network
// false = enable node/broadcast filtering to capture only frames sent to this/broadcast address
void RFM69_promiscuous(bool onOff) {
  _promiscuousMode = onOff;
  //writeReg(REG_PACKETCONFIG1, (readReg(REG_PACKETCONFIG1) & 0xF9) | (onOff ? RF_PACKET1_ADRSFILTERING_OFF : RF_PACKET1_ADRSFILTERING_NODEBROADCAST));
}

// for RFM69HW only: you must call setHighPower(true) after initialize() or else transmission won't work
void RFM69_setHighPower(bool onOff) {
  _isRFM69HW = onOff;
  RFM69_writeReg(REG_OCP, _isRFM69HW ? RF_OCP_OFF : RF_OCP_ON);
  if (_isRFM69HW) // turning ON
    RFM69_writeReg(REG_PALEVEL, (RFM69_readReg(REG_PALEVEL) & 0x1F) | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON); // enable P1 & P2 amplifier stages
  else
    RFM69_writeReg(REG_PALEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | _powerLevel); // enable P0 only
}

// internal function
void RFM69_setHighPowerRegs(bool onOff) {
  RFM69_writeReg(REG_TESTPA1, onOff ? 0x5D : 0x55);
  RFM69_writeReg(REG_TESTPA2, onOff ? 0x7C : 0x70);
}

// Serial.print all the RFM69 register values
void RFM69__readAllRegs()
{
  uint8_t regAddr = 1;
  uint8_t regVal;

  for (regAddr = 1; regAddr <= 0x4F; regAddr++)
  {
    RFM69_select();
    SPI_transfer(regAddr & 0x7F); // send address + r/w bit
    regVal = SPI_transfer(0);
    RFM69_unselect();

    printf("0x%02X", (uint16_t)regAddr);
    printf(" - ");
    printf("0x%02X", (uint16_t)regVal);
    printf("\r\n");
  }
  RFM69_unselect();
}

uint8_t RFM69_readTemperature(uint8_t calFactor) // returns centigrade
{
  RFM69_setMode(RF69_MODE_STANDBY);
  RFM69_writeReg(REG_TEMP1, RF_TEMP1_MEAS_START);
  while ((RFM69_readReg(REG_TEMP1) & RF_TEMP1_MEAS_RUNNING));
  return ~RFM69_readReg(REG_TEMP2) + COURSE_TEMP_COEF + calFactor; // 'complement' corrects the slope, rising temp = rising val
} // COURSE_TEMP_COEF puts reading in the ballpark, user can add additional correction

void RFM69_rcCalibration()
{
  RFM69_writeReg(REG_OSC1, RF_OSC1_RCCAL_START);
  while ((RFM69_readReg(REG_OSC1) & RF_OSC1_RCCAL_DONE) == 0x00);
}

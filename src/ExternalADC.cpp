//
//    FILE: ADS1115.h
//  AUTHOR: Luana Belinsky (adapted from ADS1X15 from Rob Tillaart)
// VERSION: 0.1.0
//    DATE: 2023-07-09
// PURPOSE: Library for ADS1115
//     URL: **
//
// MIT License

// Copyright (c) 2013-2024 Rob Tillaart

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "ExternalADC.h"

#define ADS1115_CONVERSION_DELAY    8

//  REGISTERS
#define ADS1115_REG_CONVERT         0x00
#define ADS1115_REG_CONFIG          0x01
#define ADS1115_REG_LOW_THRESHOLD   0x02
#define ADS1115_REG_HIGH_THRESHOLD  0x03

//  CONFIG REGISTER

//  BIT 15      Operational Status           //  1 << 15
#define ADS1115_OS_BUSY             0x0000
#define ADS1115_OS_NOT_BUSY         0x8000
#define ADS1115_OS_START_SINGLE     0x8000

//              read single
#define ADS1115_READ_0              0x4000   //  pin << 12
#define ADS1115_READ_1              0x5000   //  pin = 0..3
#define ADS1115_READ_2              0x6000
#define ADS1115_READ_3              0x7000


//  BIT 9-11    gain                         //  (0..5) << 9
#define ADS1115_PGA_6_144V          0x0000   //  
#define ADS1115_PGA_4_096V          0x0200   //  default
#define ADS1115_PGA_2_048V          0x0400   //  
#define ADS1115_PGA_1_024V          0x0600
#define ADS1115_PGA_0_512V          0x0800
#define ADS1115_PGA_0_256V          0x0A00

/*
|  PGA value  |  Max Voltage  |   Notes   |
|:-----------:|:-------------:|:---------:|
|      0      |    ±6.144V    |  default  |
|      1      |    ±4.096V    |           |
|      2      |    ±2.048V    |           |
|      4      |    ±1.024V    |           |
|      8      |    ±0.512V    |           |
|      16     |    ±0.256V    |           |
*/

//  BIT 8       mode                         //  1 << 8
#define ADS1115_MODE_CONTINUE       0x0000
#define ADS1115_MODE_SINGLE         0x0100

//  BIT 5-7     data rate sample per second  //  (0..7) << 5
/*
differs for different devices, check datasheet or readme.md

|  data rate  |  ADS101x  |  ADS111x  |   Notes   |
|:-----------:|----------:|----------:|:---------:|
|     0       |   128     |    8      |  slowest  |
|     1       |   250     |    16     |           |
|     2       |   490     |    32     |           |
|     3       |   920     |    64     |           |
|     4       |   1600    |    128    |  default  |
|     5       |   2400    |    250    |           |
|     6       |   3300    |    475    |           |
|     7       |   3300    |    860    |  fastest  |
*/

//  BIT 4 comparator modi                    //  1 << 4
#define ADS1115_COMP_MODE_TRADITIONAL   0x0000
#define ADS1115_COMP_MODE_WINDOW        0x0010

//  BIT 3 ALERT active value                 //  1 << 3
#define ADS1115_COMP_POL_ACTIV_LOW      0x0000
#define ADS1115_COMP_POL_ACTIV_HIGH     0x0008

//  BIT 2 ALERT latching                     //  1 << 2
#define ADS1115_COMP_NON_LATCH          0x0000
#define ADS1115_COMP_LATCH              0x0004

//  BIT 0-1 ALERT mode                       //  (0..3)
#define ADS1115_COMP_QUE_1_CONV         0x0000  //  trigger alert after 1 convert
#define ADS1115_COMP_QUE_2_CONV         0x0001  //  trigger alert after 2 converts
#define ADS1115_COMP_QUE_4_CONV         0x0002  //  trigger alert after 4 converts
#define ADS1115_COMP_QUE_NONE           0x0003  //  disable comparator

//  _CONFIG masks
//
//  |  bit  |  description           |
//  |:-----:|:-----------------------|
//  |   0   |  # channels            |
//  |   1   |  -                     |
//  |   2   |  resolution            |
//  |   3   |  -                     |
//  |   4   |  GAIN supported        |
//  |   5   |  COMPARATOR supported  |
//  |   6   |  -                     |
//  |   7   |  -                     |
//
#define ADS_CONF_CHAN_4  0x01 // ADS1115 has 4 pin channels
#define ADS_CONF_RES_16  0x04 // ADS1115 has 16 bit resolution
#define ADS_CONF_GAIN    0x10 // ADS1115 has gain support
#define ADS_CONF_COMP    0x20 // ADS1115 has comparator support

//////////////////////////////////////////////////////
//
//  BASE CONSTRUCTOR
//
ADS1115::ADS1115(uint8_t pin, uint8_t address, TwoWire *wire)
{
  _pin = pin;
  _address = address;
  _config = ADS_CONF_COMP | ADS_CONF_GAIN | ADS_CONF_RES_16 | ADS_CONF_CHAN_4;

  _conversionDelay = ADS1115_CONVERSION_DELAY;
  _bitShift = 0;
  _maxPorts = 4;
  _wire = wire;

  reset();
}


//////////////////////////////////////////////////////
//
//  PUBLIC
//
void ADS1115::reset()
{
  setGain(1);      //  _gain = ADS1115_PGA_4_096V; 
  setMode(0);      //  _mode = ADS1115_MODE_CONTINUE;
  setDataRate(4);  //  middle speed, depends on device.
}


bool ADS1115::begin()
{
  if ((_address < 0x48) || (_address > 0x4B)) return false;
  if (! isConnected()) return false;
  return true;
  
  if (_mode == ADS1115_MODE_CONTINUE){
    readADC(_pin);
  } 
}


bool ADS1115::isConnected()
{
  _wire->beginTransmission(_address);
  return (_wire->endTransmission() == 0);
}

void ADS1115::setGain(uint8_t gain)
{
  if (!(_config & ADS_CONF_GAIN)) gain = 0;
  switch (gain)
  {
    default:  //  catch invalid values and go for the safest gain.
    case 0:  _gain = ADS1115_PGA_6_144V;  break;
    case 1:  _gain = ADS1115_PGA_4_096V;  break;
    case 2:  _gain = ADS1115_PGA_2_048V;  break;
    case 4:  _gain = ADS1115_PGA_1_024V;  break;
    case 8:  _gain = ADS1115_PGA_0_512V;  break;
    case 16: _gain = ADS1115_PGA_0_256V;  break;
  }
}


uint8_t ADS1115::getGain()
{
  if (!(_config & ADS_CONF_GAIN)) return 0;
  switch (_gain)
  {
    case ADS1115_PGA_6_144V: return 0;
    case ADS1115_PGA_4_096V: return 1;
    case ADS1115_PGA_2_048V: return 2;
    case ADS1115_PGA_1_024V: return 4;
    case ADS1115_PGA_0_512V: return 8;
    case ADS1115_PGA_0_256V: return 16;
  }
  _error = ADS1115_INVALID_GAIN;
  return _error;
}

void ADS1115::setMode(uint8_t mode)
{
  switch (mode)
  {
    case 0: _mode = ADS1115_MODE_CONTINUE; break;
    default:
    case 1: _mode = ADS1115_MODE_SINGLE;   break;
  }
}


uint8_t ADS1115::getMode(void)
{
  switch (_mode)
  {
    case ADS1115_MODE_CONTINUE: return 0;
    case ADS1115_MODE_SINGLE:   return 1;
  }
  _error = ADS1115_INVALID_MODE;
  return _error;
}


void ADS1115::setDataRate(uint8_t dataRate)
{
  _datarate = dataRate;
  if (_datarate > 7) _datarate = 4;  //  default
  _datarate <<= 5;      //  convert 0..7 to mask needed.
}


uint8_t ADS1115::getDataRate(void)
{
  return (_datarate >> 5) & 0x07;  //  convert mask back to 0..7
}


int16_t ADS1115::readADC(uint8_t pin)
{
  if (pin >= _maxPorts) return 0;
  uint16_t mode = ((4 + pin) << 12);  //  pin to mask
  return _readADC(mode);
}


int16_t ADS1115::getValue()
{
  int16_t raw = _readRegister(_address, ADS1115_REG_CONVERT);
  if (_bitShift) raw >>= _bitShift;  //  Shift 12-bit results
  return raw;
}


void ADS1115::requestADC(uint8_t pin)
{
  if (pin >= _maxPorts) return;
  uint16_t mode = ((4 + pin) << 12);   //  pin to mask
  _requestADC(mode);
}


bool ADS1115::isBusy()
{
  return isReady() == false;
}


bool ADS1115::isReady()
{
  uint16_t val = _readRegister(_address, ADS1115_REG_CONFIG);
  return ((val & ADS1115_OS_NOT_BUSY) > 0);
}


int8_t ADS1115::getError()
{
  int8_t rv = _error;
  _error = ADS1115_OK;
  return rv;
}

//////////////////////////////////////////////////////
//
//  PROTECTED
//
int16_t ADS1115::_readADC(uint16_t readmode)
{
  _requestADC(readmode);
  if (_mode == ADS1115_MODE_SINGLE)
  {
    uint32_t start = millis();
    //  timeout == { 129, 65, 33, 17, 9, 5, 3, 2 }
    //  a few ms more than max conversion time.
    uint8_t timeOut = (128 >> (_datarate >> 5)) + 1;
    while (isBusy())
    {
      yield();   //  wait for conversion; yield for ESP.
      if ( (millis() - start) > timeOut)
      {
        return ADS1115_ERROR_TIMEOUT;
      }
    }
  }
  else
  {
    //  needed in continuous mode too, otherwise one get old value.
    delay(_conversionDelay);
  }
  return getValue();
}


void ADS1115::_requestADC(uint16_t readmode)
{
  //  write to register is needed in continuous mode as other flags can be changed
  uint16_t config = ADS1115_OS_START_SINGLE;  //  bit 15     force wake up if needed
  config |= readmode;                         //  bit 12-14
  config |= _gain;                            //  bit 9-11
  config |= _mode;                            //  bit 8
  config |= _datarate;                        //  bit 5-7

  _writeRegister(_address, ADS1115_REG_CONFIG, config);
}


bool ADS1115::_writeRegister(uint8_t address, uint8_t reg, uint16_t value)
{
  _wire->beginTransmission(address);
  _wire->write((uint8_t)reg);
  _wire->write((uint8_t)(value >> 8));
  _wire->write((uint8_t)(value & 0xFF));
  int rv = _wire->endTransmission();
  if (rv != 0) 
  {
    _error =  ADS1115_ERROR_I2C;
    return false;
  }
  return true;
}


uint16_t ADS1115::_readRegister(uint8_t address, uint8_t reg)
{
  _wire->beginTransmission(address);
  _wire->write(reg);
  int rv = _wire->endTransmission();
  if (rv == 0) 
  {
    rv = _wire->requestFrom((int) address, (int) 2);
    if (rv == 2)
    {
      uint16_t value = _wire->read() << 8;
      value += _wire->read();
      return value;
    }
  }
  _error =  ADS1115_ERROR_I2C;
  return 0x0000;
}
//  -- END OF FILE --


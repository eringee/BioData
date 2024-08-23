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

#ifndef _EXTERNALADC_H
#define _EXTERNALADC_H

#include "Arduino.h"
#include "Wire.h"

#ifndef ADS1115_ADDRESS
#define ADS1115_ADDRESS                   0x49
#endif

//  I2C Address

// The address of the ADS1115 is determined by to which pin the **ADDR**
// is connected to:

// |  ADDR pin connected to  |  Address  |  Notes    |
// |:-----------------------:|:---------:|:---------:|
// |        GND              |   0x48    |           |
// |        VDD              |   0x49    |  default  | 
// |        SDA              |   0x4A    |           |
// |        SCL              |   0x4B    |           |

#define ADS1115_OK                        0
#define ADS1115_ERROR_TIMEOUT             -101
#define ADS1115_ERROR_I2C                 -102
#define ADS1115_INVALID_GAIN              0xFF
#define ADS1115_INVALID_MODE              0xFE


class ADS1115
{
public:
  ADS1115(uint8_t pin, uint8_t address = ADS1115_ADDRESS, TwoWire *wire = &Wire);
  
  void     reset();

  bool     begin();
  bool     isConnected();

  //           GAIN
  //  0  =  +- 6.144V  
  //  1  =  +- 4.096V default
  //  2  =  +- 2.048V
  //  4  =  +- 1.024V
  //  8  =  +- 0.512V
  //  16 =  +- 0.256V

  void     setGain(uint8_t gain = 1);    //  invalid values are mapped to 1 (default).
  uint8_t  getGain();                    //  0xFF == invalid gain error.

  //  0  =  CONTINUOUS  default
  //  1  =  SINGLE       
  void     setMode(uint8_t mode = 0);    //  invalid values are mapped to 0 (default)
  uint8_t  getMode();                    //  0xFE == invalid mode error.

  //  0  =  slowest
  //  7  =  fastest
  //  4  =  default
  void     setDataRate(uint8_t dataRate = 4);  //  invalid values are mapped on 4 (default)
  uint8_t  getDataRate();                      //  actual speed depends on device

  int16_t  readADC(uint8_t pin = 0);

  //  used by continuous mode and async mode.
  int16_t  getValue();

  //  ASYNC INTERFACE
  //  requestADC(pin) -> isBusy() or isReady() -> getValue();
  //  see examples
  void     requestADC(uint8_t pin = 0);
  bool     isBusy();
  bool     isReady();

  int8_t   getError();

protected:
  //  CONFIGURATION
  //  BIT   DESCRIPTION
  //  0     # channels        0 == 1    1 == 4;
  //  1     0
  //  2     # resolution      0 == 12   1 == 16
  //  3     0
  //  4     has gain          0 = NO    1 = YES
  //  5     has comparator    0 = NO    1 = YES
  //  6     0
  //  7     0
  uint8_t  _pin;
  uint8_t  _config;
  uint8_t  _maxPorts;
  uint8_t  _address;
  uint8_t  _conversionDelay;
  uint8_t  _bitShift;
  uint16_t _gain;
  uint16_t _mode;
  uint16_t _datarate;

  int16_t  _readADC(uint16_t readmode);
  void     _requestADC(uint16_t readmode);
  bool     _writeRegister(uint8_t address, uint8_t reg, uint16_t value);
  uint16_t _readRegister(uint8_t address, uint8_t reg);
  int8_t   _error = ADS1115_OK;

  TwoWire*  _wire;
};


#endif

//  -- END OF FILE --


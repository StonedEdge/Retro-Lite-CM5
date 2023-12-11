/* ============================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2013 Jeff Rowberg

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================
*/

#ifndef _I2CDEV_H_
#define _I2CDEV_H_

#include "hardware/i2c.h"
#include <inttypes.h>

#define I2CDEVLIB_WIRE_BUFFER_LENGTH 32
#define I2CDEV_DEFAULT_READ_TIMEOUT 1000

class I2Cdev {
public:
    I2Cdev();

    static int8_t readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length,
        uint8_t* data, uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length,
        uint16_t* data, uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readByte(uint8_t devAddr, uint8_t regAddr, uint8_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readWord(uint8_t devAddr, uint8_t regAddr, uint16_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);
    static int8_t readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t* data,
        uint16_t timeout = I2Cdev::readTimeout, void* wireObj = 0);

    static bool writeBit(
        uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data, void* wireObj = 0);
    static bool writeBitW(
        uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data, void* wireObj = 0);
    static bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length,
        uint8_t data, void* wireObj = 0);
    static bool writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length,
        uint16_t data, void* wireObj = 0);
    static bool writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data, void* wireObj = 0);
    static bool writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data, void* wireObj = 0);
    static bool writeBytes(
        uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t* data, void* wireObj = 0);
    static bool writeWords(
        uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t* data, void* wireObj = 0);

    static uint16_t readTimeout;
};

#endif /* _I2CDEV_H_ */

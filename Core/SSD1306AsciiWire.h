/* Arduino SSD1306Ascii Library
 * Copyright (C) 2015 by William Greiman
 *
 * This file is part of the Arduino SSD1306Ascii Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SSD1306Ascii Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 * Modified by Martin Placek (2018)
 */
/**
 * @file SSD1306AsciiWire.h
 * @brief Class for I2C displays using Wire.
 */
#ifndef SSD1306AsciiWire_h
#define SSD1306AsciiWire_h
#include "SSD1306Ascii.h"

/**
 * @class SSD1306AsciiWire
 * @brief Class for I2C displays using Wire.
 */
class SSD1306AsciiWire : public SSD1306Ascii {
public:

    SSD1306AsciiWire();
    //virtual ~SSD1306AsciiWire();
    
    /**
     * @brief Initialize the display controller.
     *
     * @param[in] dev A device initialization structure.
     */
    void begin(const DevType* dev);

    void writeDisplay(uint8_t b, uint8_t mode);
    void writeDisplayBuffered(uint8_t b, uint8_t mode);
    void writeDisplayBufferedFlush();
    void writeDisplayBufferedClear();
    
    int openI2c(uint8_t i2cAddr);
    int closeI2c();
    
private:
    uint8_t m_i2cAddr;
    int i2cHandle;
    unsigned char buffer[12048];
    int bufferIndex;
    
    

};
#endif  // SSD1306AsciiWire_h

/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include <unistd.h>//Needed for I2C port
#include <fcntl.h>//Needed for I2C port
#include <sys/ioctl.h>//Needed for I2C port
#include <linux/i2c-dev.h>//Needed for I2C port
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <iostream>
#include <cerrno>
#include <cstring>
#include "SSD1306AsciiWire.h"

SSD1306AsciiWire::SSD1306AsciiWire() {
    bufferIndex = 1;
    buffer[1] = 0x40;
}

void SSD1306AsciiWire::begin(const DevType* dev) {
    init(dev);
}

int SSD1306AsciiWire::openI2c(uint8_t i2cAddr) {
    m_i2cAddr = i2cAddr;
    char *deviceName = (char*) "/dev/i2c-1";
    if ((i2cHandle = open(deviceName, O_RDWR)) < 0) {
        std::cout << "Open failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
        return -1;
    } else {
        if (ioctl(i2cHandle, I2C_SLAVE, m_i2cAddr) < 0) {
            std::cout << "Set ioctl failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
            return -1;
        }
    }
    return 0;
}

int SSD1306AsciiWire::closeI2c() {
    char *deviceName = (char*) "dev/i2c-1";
    close(*deviceName);
}

void SSD1306AsciiWire::writeDisplay(uint8_t b, uint8_t mode) {


    //    char *deviceName = (char*) "/dev/i2c-1";
    //    if ((i2cHandle = open(deviceName, O_RDWR)) < 0) {
    //        std::cout << "Open failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
    //    } else {
    //    if (ioctl(i2cHandle, I2C_SLAVE, m_i2cAddr) < 0) {
    //        std::cout << "Set ioctl failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
    //    } else {
    unsigned char data[2];
    data[0] = mode == SSD1306_MODE_CMD ? 0x00 : 0x40;
    data[1] = b;
    if (write(i2cHandle, data, 2) < 0) {
        std::cout << "Write failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
    }
    //    }

    // Close the i2c device bus
    //    char *deviceName = (char*) "dev/i2c-1";
    //    close(*deviceName);
    //    }
}

void SSD1306AsciiWire::writeDisplayBuffered(uint8_t b, uint8_t mode) {
//    buffer[bufferIndex++] = mode;
    buffer[bufferIndex++] = b;
}

void SSD1306AsciiWire::writeDisplayBufferedFlush() {
    return;
    
    std::cout << "buffer size: " << bufferIndex << std::endl;
    
    if (write(i2cHandle, buffer, bufferIndex) < 0) {
        std::cout << "Flush Write failed: " << std::to_string(errno) << ", " << std::strerror(errno) << std::endl;
    }
    bufferIndex = 1;
}

void SSD1306AsciiWire::writeDisplayBufferedClear() {
    bufferIndex = 1;
}


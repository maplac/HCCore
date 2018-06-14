/* 
 * File:   ControlPanel.cpp
 * Author: Martin
 * 
 * Created on 3. června 2018, 17:50
 */

#include "ControlPanel.h"

#define OBJECT_NAME     "Control panel"
#include "Log.h"

#include "DeviceManager.h"
#include <zhelpers.hpp>
#include "SSD1306Ascii.h"
extern "C" {
#include "gpio.h"
}
#include <poll.h>
#include <fcntl.h>

#define I2C_ADDRESS 0x3C

ControlPanel::ControlPanel(zmq::context_t *zmqContext, DeviceManager &deviceManager) : zmqContext(zmqContext), deviceManager(deviceManager) {
    isOpened = false;
}

ControlPanel::~ControlPanel() {
}

int ControlPanel::openDevice() {


    if (oled.openI2c(I2C_ADDRESS) < 0) {
        LOG_E("Opening I2C for oled failed.");
        isOpened = false;
        return -1;
    }

    oled.begin(&Adafruit128x64Inverted);
    oled.setFont(System5x7);
    oled.set2X();
    oled.setContrast(1);
    oled.ssd1306WriteCmd(SSD1306_DISPLAYON);
    oled.clear();
    oled.home();

    int fd;
    int res;
    int timeout = 5000000;
    int tick = 100000;

    // prepare button 1
    res = writeToFile("/sys/class/gpio/export", "17");
    if (res < 0) {
        return -1;
    }
    while (access("/sys/class/gpio/gpio17/value", R_OK | W_OK) != 0 && timeout >= 0) {
        usleep(tick);
        timeout -= tick;
    }
    res = writeToFile("/sys/class/gpio/gpio17/direction", "in");
    if (res < 0) {
        return -1;
    }
    res = writeToFile("/sys/class/gpio/gpio17/edge", "both");
    if (res < 0) {
        return -1;
    }
    fdButton1 = openFileDescriptor("/sys/class/gpio/gpio17/value");
    if (fdButton1 < 0) {
        return -1;
    }

    // prepare button 2
    res = writeToFile("/sys/class/gpio/export", "4");
    if (res < 0) {
        return -1;
    }
    timeout = 5000000;
    while (access("/sys/class/gpio/gpio4/value", R_OK | W_OK) != 0 && timeout >= 0) {
        usleep(tick);
        timeout -= tick;
    }
    res = writeToFile("/sys/class/gpio/gpio4/direction", "in");
    if (res < 0) {
        return -1;
    }
    res = writeToFile("/sys/class/gpio/gpio4/edge", "both");
    if (res < 0) {
        return -1;
    }
    fdButton2 = openFileDescriptor("/sys/class/gpio/gpio4/value");
    if (fdButton2 < 0) {
        return -1;
    }


    isOpened = true;
}

int ControlPanel::writeToFile(std::string file, std::string data) {
    int fd;
    int res;
    fd = open(file.c_str(), O_WRONLY);
    if (fd < 0) {
        LOG_E("opening file failed: " + file + ": " + std::string(strerror(errno)));
        return -1;
    }
    res = write(fd, data.c_str(), data.size());
    if (res == 0) {
        LOG_E("nothing was writen to file " + file);
        return -1;
    } else if (res < 0) {
        LOG_E("writing to file failed: " + file + ": " + std::string(strerror(errno)));
        return -1;
    }
    close(fd);
}

int ControlPanel::openFileDescriptor(std::string file) {
    int fd;
    fd = open(file.c_str(), O_RDONLY);
    if (fd < 0) {
        LOG_E("opening file failed: " + file + ": " + std::string(strerror(errno)));
        return -1;
    }
    return fd;
}

int ControlPanel::closeDevice() {
    oled.ssd1306WriteCmd(SSD1306_DISPLAYOFF);
    oled.closeI2c();
    close(fdButton1);
    close(fdButton2);

    int res = 0;
    res = writeToFile("/sys/class/gpio/unexport", "17");

    res = writeToFile("/sys/class/gpio/unexport", "4");

    usleep(10000);
}

void ControlPanel::run() {

    if (!isOpened) {
        LOG_E("Not started because it is not opened");
        return;
    }

    //    oled.println("Hello Word!");
    struct pollfd pfds[2];
    pfds[0].fd = fdButton1;
    pfds[0].events = POLLPRI | POLLERR;
    pfds[1].fd = fdButton2;
    pfds[1].events = POLLPRI | POLLERR;

    int displayTimeout = 0;
    bool displayActive = false;


    LOG_I("started");
    while (isRunning) {
        char c, d;
        lseek(fdButton1, 0, SEEK_SET);
        read(fdButton1, &c, 1);
        lseek(fdButton2, 0, SEEK_SET);
        read(fdButton2, &d, 1);
        int ret = poll(pfds, 2, 1000);

        if (displayActive) {
            displayTimeout--;
            if (displayTimeout < 0) {
                displayActive = false;
                oled.clear();
                oled.home();
            }
        }

        if (ret == 0) {
            //            printf("Timeout\n");
        } else {

            if (pfds[0].revents & POLLPRI) {
                if (c == '0') {
                    //                    printf("button1: Release\n");

                } else {
                    //                    printf("button1: Push\n");
                }

            }
            if (pfds[1].revents & POLLPRI) {
                if (d == '0') {
                    //                    printf("button2: Release\n");
                    if (!displayActive) {
                        std::vector<std::string> msgs = deviceManager.getOledMessages();
                        for (auto e : msgs) {
                            oled.println(e);
                        }
                        displayActive = true;
                    }
                    displayTimeout = 5;
                } else {
                    //                    printf("button2: Push\n");
                }

            }
        }
    }
    LOG_I("finished");
}

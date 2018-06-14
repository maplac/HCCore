/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   ControlPanel.h
 * Author: Martin
 *
 * Created on 3. června 2018, 17:50
 */

#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include "Runnable.h"
#include <zmq.hpp>
#include <json.hpp>
#include <vector>
#include "SSD1306AsciiWire.h"

class DeviceManager;

class ControlPanel : public Runnable {
public:
    ControlPanel(zmq::context_t *zmqContext, DeviceManager &deviceManager);
    virtual ~ControlPanel();
    
    int openDevice();
    int closeDevice();
    
private:
    zmq::context_t *zmqContext;
    DeviceManager &deviceManager;
    SSD1306AsciiWire oled;
    bool isOpened;
    int fdButton1;
    int fdButton2;
    const int pinButton1 = 17;
    const int pinButton2 = 4;
    
    int writeToFile(std::string file, std::string data);
    int openFileDescriptor(std::string file);

protected:
    virtual void run();

};

#endif /* CONTROLPANEL_H */


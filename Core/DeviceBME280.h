/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DeviceBME280.h
 * Author: Martin
 *
 * Created on 23. dubna 2017, 20:45
 */

#ifndef DEVICEBME280_H
#define DEVICEBME280_H

#include "DeviceGeneric.h"
#include <time.h>
#include <deque>


class DeviceBME280 : public DeviceGeneric{
public:
    DeviceBME280(int id, Interface interface);
    virtual ~DeviceBME280();
    
    virtual nlohmann::json getDevice();
    /** 0 parameter not found, 1 parameter changed*/
    virtual int setParameter(const nlohmann::json &parameter);

    /** -1 error, 0 nothing, 1 something*/
    // called from mainLoop() when GUI send message
    virtual int processMsgFromGui(const nlohmann::json &msg, nlohmann::json& reply);
    // called from mainLoop() when remote device sends packet
    virtual int processMsgFromDevice(const nlohmann::json &msg, nlohmann::json& reply);
    virtual int saveDeviceToFile() override;
    virtual int loadReadoutsBuffer();

private:
    typedef struct readout_tag {
        struct timeval time;
        int temperature;
        int pressure;
        int humidity;
    }readout;
    readout lastReadout;
    std::deque<readout> readoutsBuffer;
    
    int saveLastReadout();
    int removeOldReadouts();
    readout stringToReadout(const std::vector<std::string> &cells);
};

#endif /* DEVICEBME280_H */


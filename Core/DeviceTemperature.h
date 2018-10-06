/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   DeviceTemperature.h
 * Author: Martin
 *
 * Created on 18. února 2018, 14:19
 */

#ifndef DEVICETEMPERATURE_H
#define DEVICETEMPERATURE_H

#include "DeviceGeneric.h"
#include <time.h>
#include <deque>
#include <string>
#include <vector>

class DeviceTemperature : public DeviceGeneric {
public:
    DeviceTemperature(int id, Interface interface);
    virtual ~DeviceTemperature();

    virtual nlohmann::json getDevice();
    virtual nlohmann::json getDeviceForWeb();
    /** 0 parameter not found, 1 parameter changed*/
    virtual int setParameter(const nlohmann::json &parameter);

    /** -1 error, 0 nothing, 1 something*/
    // called from mainLoop() when GUI send message
    virtual int processMsgFromGui(const nlohmann::json &msg, nlohmann::json& reply);
    // called from mainLoop() when remote device sends packet
    virtual int processMsgFromDevice(const nlohmann::json &msg, nlohmann::json& reply);
    virtual int saveDeviceToFile() override;
    virtual int loadReadoutsBuffer();
    virtual std::string getOledMessage() override;


private:

    typedef struct readout_tag {
        struct timeval time;
        float temperature;
        float voltage;
    } readout;

    typedef struct readoutString_tag {
        std::string date;
        std::string hours;
        std::string minutes;
        std::string seconds;
        std::string temperature;
        std::string voltage;
        bool isValid = true;
    } readoutString;
    typedef struct {
        std::vector<std::string> time;
        std::vector<int> temperature;
        std::vector<int> temperatureMin;
        std::vector<int> temperatureMax;
    }readouts_averaged;
    readout lastReadout;
    std::deque<readout> readoutsBuffer;
    int packetCounter;
    unsigned int lostReadouts;

    int saveLastReadout();
    int removeOldReadouts();
    readout stringToReadout(const std::vector<std::string> &cells);
    int getReadoutsDay(readouts_averaged &readouts);
    int getReadoutsWeek(readouts_averaged &readouts);
    int getReadoutsMonth(readouts_averaged &readouts);
    int getReadoutsYear(readouts_averaged &readouts);
    readoutString splitReadout(std::string line);

};

#endif /* DEVICETEMPERATURE_H */


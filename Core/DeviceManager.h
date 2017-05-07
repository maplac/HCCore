/* 
 * File:   DeviceManager.h
 * Author: Martin
 *
 * Created on 17. července 2016, 21:28
 */

#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "includeAllDevices.h"
#include <json.hpp>
#include <vector>
#include "globalConstants.h"

class DeviceManager {
public:
    DeviceManager();
    virtual ~DeviceManager();
    int loadDevices();
    int saveDevices();
    DeviceGeneric *getDeviceById(int id);
    
    nlohmann::json getAllDevices(void);
    std::vector<int> getDevicesIds();
    
    int processMsgFromDevice(const nlohmann::json &message, nlohmann::json &reply);
    /*
     * -1 device doesn't exist
     * -2 bad message
     *  1 send reply
     */
    int processMsgFromGui(const nlohmann::json &message, nlohmann::json &reply);
    
    
    
private:
//    std::vector<DeviceGeneric *> deviceList;
    std::map<int,DeviceGeneric*> devicesList;
    
    DeviceGeneric *createDevice(int id, DeviceGeneric::Type type, DeviceGeneric::Interface interface);
//    void addDevice(int id,DeviceType::value type);
//    void addDevice(nlohmann::json deviceJson);
};

#endif	/* DEVICEMANAGER_H */


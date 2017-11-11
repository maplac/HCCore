/* 
 * File:   DeviceGeneric.h
 * Author: Martin
 *
 * Created on 25. června 2016, 21:32
 */

#ifndef DEVICEGENERIC_H
#define	DEVICEGENERIC_H

#include "globalConstants.h"
#include <unistd.h>
#include <string>
#include <ctime>
#include <chrono>
#include <json.hpp>
#include <sys/time.h>

#define PATH_DATA       "/home/pi/HomeControl-Data/"
#define PATH_DEVICES    "/home/pi/HomeControl-Config/"

std::vector<std::string> split(const std::string& str, std::string delim);

class DeviceGeneric {
public:
    
    enum class Interface {none, rf24, udp};
    enum class Type{none, BME280};
    
    DeviceGeneric(int id, Type type, Interface interface);
    virtual ~DeviceGeneric();

    static int typeToInt(Type type);
    static Type intToType(int type);
    static std::string typeToString(Type type);
    static Type stringToType(std::string type);
    static int interfaceToInt(Interface interface);
    static Interface intToInterface(int interface);
    static std::string idToString(int id);
//    static std::string timeToString(const std::chrono::system_clock::time_point& tp);
    static std::string timeToString(const struct timeval &timestamp, const char * format = "%Y-%m-%d %H:%M:%S");
    static std::string timeToStringLocal(const struct timeval &timestamp, const char * format = "%Y-%m-%d %H:%M:%S");
    
    int getId();
    virtual nlohmann::json getDevice();
    /** 0 parameter not found, 1 parameter changed*/
    virtual int setParameter(const nlohmann::json &parameter);
    virtual int saveDeviceToFile() = 0;
    
    
    /** -1 error, 0 nothing, 1 something*/
    // called from mainLoop() when GUI send message
    virtual int processMsgFromGui(const nlohmann::json &msg, nlohmann::json& reply) = 0;
    // called from mainLoop() when remote device sends packet
    virtual int processMsgFromDevice(const nlohmann::json &msg, nlohmann::json& reply) = 0;
    
    virtual int loadReadoutsBuffer();

protected:
    std::string status;
    struct timeval lastConnected;
    int id;
    Type type;
    std::string name;
    std::string description;
    Interface interface;
    int pipeIndex;
    bool isWaitingForFirstData;
};

#endif	/* DEVICEGENERIC_H */


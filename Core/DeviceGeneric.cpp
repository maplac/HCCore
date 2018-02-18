/* 
 * File:   DeviceGeneric.cpp
 * Author: Martin
 * 
 * Created on 25. června 2016, 21:32
 */

#include "DeviceGeneric.h"
#define OBJECT_NAME     "Device"
#include "Log.h"

#include <time.h> 

using json = nlohmann::json;

DeviceGeneric::DeviceGeneric(int id, Type type, Interface interface) {
    this->id = id;
    this->type = type;
    this->interface = interface;
    pipeIndex = -1;
    isWaitingForFirstData = true;
}

DeviceGeneric::~DeviceGeneric() {

}

json DeviceGeneric::getDevice() {
    json device;
    device["id"] = id;
    device["type"] = typeToString(type);
    device["interface"] = interfaceToInt(interface);
    device["name"] = name;
    device["description"] = description;
    device["status"] = status;
    device["lastConnected"] = timeToString(lastConnected);
    device["pipeIndex"] = pipeIndex;
    return device;
}

int DeviceGeneric::getId() {
    return id;
}

//std::string DeviceGeneric::timeToString(const std::chrono::system_clock::time_point& tp) {
//    // convert to system time:
//    std::time_t t = std::chrono::system_clock::to_time_t(tp);
//    char buff[20];
//    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
//    std::string str(buff);
//    return str;
//}

std::string DeviceGeneric::timeToString(const struct timeval &timestamp, const char * format) {
    time_t t = timestamp.tv_sec;
    char buf[20];
    strftime(buf, 20, format, gmtime(&t));
    return std::string(buf);
}

std::string DeviceGeneric::timeToStringLocal(const struct timeval &timestamp, const char * format) {
    time_t t = timestamp.tv_sec;
    char buf[20];
    strftime(buf, 20, format, localtime(&t));
    return std::string(buf);
}

struct timeval DeviceGeneric::stringToTime(const std::string& timeString, const char* format) {
    std::tm loadedTime = {};
    strptime(timeString.c_str(), format, &loadedTime);
    //    std::cout << "h: " << loadedTime.tm_hour << ", m: " << loadedTime.tm_min << ", s: " << loadedTime.tm_sec << std::endl;
    struct timeval time;
    time.tv_sec = loadedTime.tm_sec + loadedTime.tm_min * 60 + loadedTime.tm_hour * 3600 + loadedTime.tm_yday * 86400 +
            (loadedTime.tm_year - 70) * 31536000 + ((loadedTime.tm_year - 69) / 4) * 86400 -
            ((loadedTime.tm_year - 1) / 100) * 86400 + ((loadedTime.tm_year + 299) / 400) * 86400;
    time.tv_usec = 0;
    return time;
}

int DeviceGeneric::setParameter(const json &parameter) {

    if (parameter.find("description") != parameter.end()) {
        description = parameter["description"];
        return 1;
    }
    if (parameter.count("name") > 0) {
        name = parameter["name"];
        return 1;
    }
    if (parameter.find("status") != parameter.end()) {
        status = parameter["status"];
        return 1;
    }
    if (parameter.find("pipeIndex") != parameter.end()) {
        pipeIndex = parameter["pipeIndex"];
        return 1;
    }
    if (parameter.find("interface") != parameter.end()) {
        interface = intToInterface(parameter["interface"]);
        return 1;
    }
    if (parameter.find("lastConnected") != parameter.end()) {
        //        lastConnected = stringToTime();
        return 1;
    }
    return 0;
}

DeviceGeneric::Type DeviceGeneric::intToType(int type) {
    switch (type) {
        case 1: return Type::BME280;
        case 2: return Type::Temperature;
        default: return Type::none;
    }
}

int DeviceGeneric::typeToInt(Type type) {
    switch (type) {
        case Type::BME280: return 1;
        case Type::Temperature: return 2;
        default: return 0;
    }
}

std::string DeviceGeneric::typeToString(Type type) {
    switch (type) {
        case Type::BME280: return "BME280";
        case Type::Temperature: return "Temperature";
        default: return "none";
    }
}

DeviceGeneric::Interface DeviceGeneric::intToInterface(int interface) {
    switch (interface) {
        case 1: return Interface::rf24;
        case 2: return Interface::udp;
        default: return Interface::none;
    }
}

int DeviceGeneric::interfaceToInt(Interface interface) {
    switch (interface) {
        case Interface::rf24: return 1;
        case Interface::udp: return 2;
        default: return 0;
    }
}

std::vector<std::string> split(const std::string& str, std::string delim) {
    int end = 0, start = 0;
    std::vector<std::string> splited;
    end = str.find_first_of(delim, start);

    if (end == std::string::npos) {
        splited.push_back(str);
        return splited;
    }

    while (end != std::string::npos) {

        splited.push_back(str.substr(start, end - start));

        start = end + delim.length();
        end = str.find_first_of(delim, start);
    }
    if (start < str.length()) {
        splited.push_back(str.substr(start, str.length() - start));
    }
    return splited;
}

DeviceGeneric::Type DeviceGeneric::stringToType(std::string type) {

    if (type.compare("BME280") == 0) {
        return Type::BME280;
    } else if (type.compare("Temperature") == 0) {
        return Type::Temperature;
    } else {
        return Type::none;
    }
}

std::string DeviceGeneric::idToString(int id) {
    std::string idString = "ID";
    if (id < 100) {
        idString = idString + "0";
    }
    if (id < 10) {
        idString = idString + "0";
    }
    idString = idString + std::to_string(id);
    return idString;
}

int DeviceGeneric::loadReadoutsBuffer() {

}


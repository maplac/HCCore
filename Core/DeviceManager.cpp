/* 
 * File:   DeviceManager.cpp
 * Author: Martin
 * 
 * Created on 17. července 2016, 21:29
 */

#include "DeviceManager.h"
#include "PacketManager.h"
#define OBJECT_NAME     "Device manager"
#include "Log.h"
#include "DeviceGeneric.h"
#include "DeviceTemperature.h"

#include <sstream>
#include <fstream>
#include <string>
#include <string.h> // for strerror
#include <dirent.h>
#include <unistd.h>


//using namespace std;
using json = nlohmann::json;

DeviceManager::DeviceManager() {
}

DeviceManager::~DeviceManager() {
    for (auto const &ent : devicesList) {
        delete ent.second;
    }
}

int DeviceManager::loadDevices() {

    //    call destructor for every element
    for (auto const &ent : devicesList) {
        delete ent.second;
    }
    devicesList.clear();

    // get list of all files in folder
    std::string dir = PATH_DEVICES;
    std::vector<std::string> files = std::vector<std::string>();
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL) {
        LOG_W("opening file " + dir + " while enumerating");
        return 1;
    }
    while ((dirp = readdir(dp)) != NULL) {
        files.push_back(std::string(dirp->d_name));
    }
    closedir(dp);

    // over all files
    for (int i = 0; i < files.size(); i++) {
        //cout << files[i] << endl;
        if (((std::string) files[i]).length() < 3) {
            continue;
        }

        // get name and extension
        std::vector<std::string> splitted = split(files[i], ".");
        if (splitted.size() < 2) {
            continue;
        }
        std::string name = splitted[0];
        std::string extension = splitted[1];

        // if the file has correct name
        if (extension.compare("json") == 0 && name.length() == 5 && name.substr(0, 2).compare("ID") == 0) {

            // read the file and convert to json
            std::ofstream file;
            std::string path = PATH_DEVICES + files[i];
            file.open(path.c_str(), std::ios::in);
            if (!file.is_open()) {
                LOG_E("File " + dir + " could not be opened");
                continue;
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            std::string fileString = buffer.str();
            json fileJson;
            try {
                fileJson = json::parse(fileString);
            } catch (std::exception &e) {
                LOG_E("loadDevices() parsing file " + path + " failed: " + std::string(e.what()));
            }

            if (fileJson.count("device") != 0) {
                //                addDevice(fileJson["device"]);
                json deviceJson = fileJson["device"];
                if (deviceJson.count("type") == 0) {
                    LOG_E("loadDevices() type undefined");
                    continue;
                }
                if (deviceJson.count("interface") == 0) {
                    LOG_E("loadDevices() interface undefined");
                    continue;
                }
                if (deviceJson.count("id") == 0) {
                    LOG_E("loadDevices() id undefined");
                    continue;
                }
                DeviceGeneric::Type type = DeviceGeneric::stringToType(deviceJson["type"]);
                DeviceGeneric::Interface interface = DeviceGeneric::intToInterface(deviceJson["interface"]);
                int id = deviceJson["id"];

                DeviceGeneric* device = createDevice(id, type, interface);
                if (device == nullptr) {
                    LOG_E("loadDevice() create device failed");
                    continue;
                }
                for (json::iterator it = deviceJson.begin(); it != deviceJson.end(); ++it) {
                    json param = {
                        {it.key(), it.value()}
                    };
                    int res = device->setParameter(param);
                    if (res < 0) {
                        LOG_E("loadDevices() set parameter failed, id = " + std::to_string(id) + ", " + param.dump(3));
                    } else if (res == 0) {
                        //                        LOG_W("loadDevices() set parameter skipped, id = " + std::to_string(id) + ", " + param.dump(3));
                    }

                }
                devicesList[id] = device;
                device->loadReadoutsBuffer();
            } else {
                LOG_E("loadDevices() file " + path + " does not contain entry \"device\"");
            }
        }
    }
    return 0;
}

int DeviceManager::saveDevices() {

    for (auto const &ent : devicesList) {
        DeviceGeneric *device = ent.second;

        device->saveDeviceToFile();
    }
}

DeviceGeneric* DeviceManager::getDeviceById(int id) {
    if (devicesList.find(id) == devicesList.end()) {
        return nullptr;
    } else {
        return devicesList[id];
    }
}

std::vector<int> DeviceManager::getDevicesIds() {
    std::vector<int> ids;
    for (auto const &ent : devicesList) {
        ids.push_back(ent.first);
    }
    return ids;
}

json DeviceManager::getAllDevices() {
    std::vector<json> devicesJson;
    for (auto const &ent : devicesList) {
        devicesJson.push_back(ent.second->getDeviceForWeb());
    }
    return json(devicesJson);
}

DeviceGeneric * DeviceManager::createDevice(int id, DeviceGeneric::Type type, DeviceGeneric::Interface interface) {
    DeviceGeneric *device;
    switch (type) {
        case DeviceGeneric::Type::BME280:
            device = new DeviceBME280(id, interface);
            break;
        case DeviceGeneric::Type::Temperature:
            device = new DeviceTemperature(id, interface);
            break;
        default:
            return nullptr;
    }

    return device;
}

int DeviceManager::processMsgFromDevice(const nlohmann::json& message, nlohmann::json & reply) {
    int id = message["desId"];

    DeviceGeneric *device = getDeviceById(id);

    // if the id is not in the list
    if (device == nullptr) {
        LOG_W("processMsgFromDevice() device does not exist, id = " + std::to_string(id));
        return -1;
    }

    // pass the message to the device
    return device->processMsgFromDevice(message, reply);
}

int DeviceManager::processMsgFromGui(const nlohmann::json& message, nlohmann::json & reply) {

    std::string type = message["type"];

    if (type.compare("pullAllDevices") == 0) {
        reply["desId"] = message["srcId"];
        reply["type"] = "pushAllDevices";
        reply["devices"] = getAllDevices();
        return 1;
    }

    //TODO report errors back to the client
    int id = message["desId"];
    DeviceGeneric *device = getDeviceById(id);

    // if the device doesn't exist
    if (device == nullptr) {
        LOG_E("processMsgFromGui() device does not exist, id = " + std::to_string(id));
        return -1;
    }

    // pass the message to the device
    return device->processMsgFromGui(message, reply);

}

std::vector<std::string> DeviceManager::getOledMessages() {
    std::vector<std::string> msgs;

    for (auto const &ent : devicesList) {
        std::string msg = ent.second->getOledMessage();
        if (msg.empty())
            continue;
        msgs.push_back(msg);
    }

    return msgs;
}

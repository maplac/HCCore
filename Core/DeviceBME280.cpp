/*
 * File:   DeviceBME280.cpp
 * Author: Martin
 *
 * Created on 23. dubna 2017, 20:45
 */

#include "DeviceBME280.h"
#define OBJECT_NAME "BME280"
#include "Log.h"

#include <string>
#include <sstream>
#include <fstream>
#include <string.h> // for strerror
#include <dirent.h>
#include <unistd.h>

#define READOUTS_BUFFER_PERIOD 60//(12*60*60)
using json = nlohmann::json;

DeviceBME280::DeviceBME280(int id, Interface interface) : DeviceGeneric(id, Type::BME280, interface) {
    status = "measuring";
}

DeviceBME280::~DeviceBME280() {
}

int DeviceBME280::setParameter(const json &parameter) {
    int res = DeviceGeneric::setParameter(parameter);

    // if the parameter is not generic
    if (res == 0) {

        if (parameter.find("temperature") != parameter.end()) {
            lastReadout.temperature = parameter["temperature"];
        } else if (parameter.find("pressure") != parameter.end()) {
            lastReadout.pressure = parameter["pressure"];
        } else if (parameter.find("humidity") != parameter.end()) {
            lastReadout.humidity = parameter["humidity"];
        } else {
            return 0;
        }

    } else {
        return res;
    }
}

nlohmann::json DeviceBME280::getDevice() {
    json device = DeviceGeneric::getDevice();
    device["temperature"] = lastReadout.temperature;
    device["pressure"] = lastReadout.pressure;
    device["humidity"] = lastReadout.humidity;
    return device;
}

/*
 * desId
 * srcId: RF24SERVER_ID
 * type: dataReceived
 * pipeIndex
 * data
 *
 */
int DeviceBME280::processMsgFromDevice(const nlohmann::json& msg, nlohmann::json& reply) {
    gettimeofday(&lastConnected, NULL);
    std::string msgType = msg["type"];
    int srcId = msg["srcId"];

    // if the interface is RF24
    if (srcId == RF24SERVER_ID) {

        // update pipe index
        pipeIndex = msg["pipeIndex"];
        if (msgType.compare("dataReceived") == 0) {

            // convert json data dot vector
            if (msg.find("data") == msg.end()) {
                LOG_E("processMsgFromDevice() message doesn't contain entry \"data\".");
                return -1;
            }
            json dataJson = msg["data"];
            uint8_t data[32];
            int i = 0;
            for (json::iterator it = dataJson.begin(); it != dataJson.end(); ++it) {
                data[i] = (uint8_t) * it;
                i++;
            }
            if (i < 11) {
                LOG_E("processMsgFromDevice() data array is too short");
                return -1;
            }

            // get readout from recived packet
            float *dataF = (float*) &data;
            lastReadout.temperature = round(dataF[1]*10);
            lastReadout.pressure = round(dataF[2]);
            lastReadout.humidity = round(dataF[3]);
            lastReadout.time = lastConnected;

            // push new readout to a queque that hold last values
            while (readoutsBuffer.size() > 30) {
                readoutsBuffer.pop_front();
            }
            readoutsBuffer.push_back(lastReadout);

            std::stringstream ss;
            ss << "temperature = " << lastReadout.temperature;
            ss << ", pressure = " << lastReadout.pressure;
            ss << ", humidity = " << lastReadout.humidity;
            ss << ", time = " << timeToString(lastReadout.time);
            //            ss << ", local = " << timeToStringLocal(lastReadout.time);
            LOG_I(ss.str());

            // create messager for web server
            reply["type"] = "pushNewData";
            reply["lastConnected"] = timeToString(lastConnected);
            reply["data"] = {
                {"temperature", lastReadout.temperature},
                {"pressure", lastReadout.pressure},
                {"humidity", lastReadout.humidity},
                {"time", timeToStringLocal(lastReadout.time)}
            };

            saveLastReadout();
            removeOldReadouts();
        } else {
            LOG_E("processMsgFromGui() unknown type of message");
            return -1;
        }
    } else {
        LOG_E("processMsgFromGui() interface is not RF24");
        return -1;
    }
    return 1;


}

int DeviceBME280::processMsgFromGui(const nlohmann::json& msg, nlohmann::json& reply) {

    std::string msgType = msg["type"];

    if (msgType.compare("setParameter") == 0) {
        if (msg.find("parameter") != msg.end()) {
            json::iterator it = ((json) msg["parameter"]).begin();
            int res = setParameter({
                {it.key(), it.value()}
            });
            if (res == 0) {
                LOG_E("processMsgFromGui() parameter does't exist:\n" + msg["parameter"].dump(3));
                return -1;
            } else if (res < 0) {
                LOG_E("processMsgFromGui() change parameter failed:\n" + msg["parameter"].dump(3));
                return -1;
            } else {
                reply["desId"] = BROADCAST_ID;
                reply["type"] = "pushDevice";
                reply["device"] = getDevice();
            }

        } else {
            LOG_E("processMsgFromGui() setParameter does not contain entry \"parameter\"");
            return -1;
        }

    } else if (msgType.compare("pullDevice") == 0) {
        reply["desId"] = msg["srcId"];
        reply["type"] = "pushDevice";
        reply["device"] = getDevice();

    } else if (msgType.compare("pullDataBuffer") == 0) {
        if (readoutsBuffer.empty()) {
            LOG_W("processMsgFromGui() readout buffer is empty");
            return -1;
        }
        reply["desId"] = msg["srcId"];
        reply["type"] = "pushDataBuffer";

        std::vector<int> readoutsTemperature;
        std::vector<int> readoutsPressure;
        std::vector<int> readoutsHumidity;
        std::vector<std::string> readoutsTime;
        for (int i = 0; i < readoutsBuffer.size(); i++) {
            readoutsTemperature.push_back(readoutsBuffer[i].temperature);
            readoutsHumidity.push_back(readoutsBuffer[i].humidity);
            readoutsPressure.push_back(readoutsBuffer[i].pressure);
            readoutsTime.push_back(timeToStringLocal(readoutsBuffer[i].time));
        }
        reply["data"] = {
            {"temperature", readoutsTemperature},
            {"pressure", readoutsPressure},
            {"humidity", readoutsHumidity},
            {"time", readoutsTime}
        };
//        LOG_I("processMsgFromGui() data:\n" + reply["data"].dump(2));
        

    } else {
        LOG_E("processMsgFromGui() unknown type of message");
        return -1;
    }
    return 1;
}

int DeviceBME280::saveLastReadout() {


    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    std::string dateString = timeToString(timeNow, "%Y-%m-%d");
    std::string idString = idToString(id);
    std::string typeString = typeToString(type);
    std::string fileName = idString + "_" + typeString + "_" + dateString + ".csv";
    std::string filePath = std::string(PATH_DATA) + "/" + idString + "/";
    std::ofstream fs;
    fs.open(filePath + fileName, std::ios::out | std::ios::app);
    if (!fs.is_open()) {
        LOG_E("opening file " + fileName + ": " + std::string(strerror(errno)));
        return -1;
    }
    fs << timeToString(lastReadout.time) << ", ";
    fs << std::setprecision(4) << lastReadout.temperature << ", ";
    fs << std::setprecision(5) << lastReadout.pressure << ", ";
    fs << std::setprecision(2) << lastReadout.humidity << ", " << std::endl;
    fs.close();

    return 1;
}

int DeviceBME280::loadReadoutsBuffer() {

    using namespace std;
    struct timeval timeNow;
    gettimeofday(&timeNow, NULL);
    string timeNowString = timeToString(timeNow, "%Y-%m-%d");
    timeNow.tv_sec -= READOUTS_BUFFER_PERIOD;
    string timeStartString = timeToString(timeNow, "%Y-%m-%d");


    string fileName = idToString(id) + "_" + typeToString(type) + "_" + timeStartString + ".csv";
    string filePath = std::string(PATH_DATA) + "/" + idToString(id) + "/";
    ifstream fs;
    fs.open(filePath + fileName, std::ios::in);
    if (!fs.is_open()) {
        LOG_W("loadReadoutsBuffer()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
        return -1;
    }

    // read whole file
    string line;
    while (getline(fs, line)) {
        vector<string> cells = split(line, ",");

        // if the line is deformed skip it
        if (cells.size() < 4) {
            continue;
        }

        readout r = stringToReadout(cells);
        //        LOG_I("1loaded time      : " + timeToString(r.time));
        readoutsBuffer.push_back(r);
        //        LOG_I("1loaded time local: " + timeToStringLocal(r.time));

    }
    fs.close();

    // data are loaded from two files
    if (timeStartString.compare(timeNowString) != 0) {
        fileName = idToString(id) + "_" + typeToString(type) + "_" + timeNowString + ".csv";
        filePath = string(PATH_DATA) + "/" + idToString(id) + "/";
        ifstream fs;
        fs.open(filePath + fileName, ios::in);
        if (!fs.is_open()) {
            LOG_W("loadReadoutsBuffer()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
            return -1;
        }

        while (getline(fs, line)) {
            vector<string> cells = split(line, ",");

            // if the line is deformed skip it
            if (cells.size() < 4) {
                continue;
            }

            readoutsBuffer.push_back(stringToReadout(cells));
        }
        fs.close();
    }

    removeOldReadouts();

    return 0;
}

DeviceBME280::readout DeviceBME280::stringToReadout(const std::vector<std::string> &cells) {
    readout r;
    std::string cellTime = cells[0];

    std::tm loadedTime = {};
    strptime(cellTime.c_str(), "%Y-%m-%d %H:%M:%S", &loadedTime);
    //    std::cout << "h: " << loadedTime.tm_hour << ", m: " << loadedTime.tm_min << ", s: " << loadedTime.tm_sec << std::endl;
    r.time.tv_sec = loadedTime.tm_sec + loadedTime.tm_min * 60 + loadedTime.tm_hour * 3600 + loadedTime.tm_yday * 86400 +
            (loadedTime.tm_year - 70) * 31536000 + ((loadedTime.tm_year - 69) / 4) * 86400 -
            ((loadedTime.tm_year - 1) / 100) * 86400 + ((loadedTime.tm_year + 299) / 400) * 86400;
    r.time.tv_usec = 0;
    //    LOG_I("loaded time      : " + timeToString(r.time));
    //    LOG_I("loaded time local: " + timeToStringLocal(r.time));

    r.temperature = stoi(cells[1]);
    r.pressure = stoi(cells[2]);
    r.humidity = stoi(cells[3]);
    return r;
}

int DeviceBME280::saveDeviceToFile() {
    // open the file
    std::string path = PATH_DEVICES + idToString(id) + ".json";
    std::ofstream out_file;
    out_file.open(path, std::ios::out);
    if (!out_file.is_open()) {
        LOG_E("opening file " + path + ": " + std::string(strerror(errno)));
        return -1;
    }

    // serialize the device
    json deviceJson = {
        {"device", getDevice()}
    };
    out_file << deviceJson.dump(3);
    out_file.close();
    return 0;
}

int DeviceBME280::removeOldReadouts() {
    using namespace std;
    struct timeval timeBreak;
    gettimeofday(&timeBreak, NULL);
    timeBreak.tv_sec -= READOUTS_BUFFER_PERIOD;

    int deleted = 0;
    LOG_I("timeBreak: " + timeToString(timeBreak));
    for (int i = 0; i < readoutsBuffer.size(); i++) {
        if (timercmp(&timeBreak, &readoutsBuffer[i].time, >)) {
            //            LOG_I("removed: " + timeToString(readoutsBuffer[i].time));
            readoutsBuffer.erase(readoutsBuffer.begin() + i);
            deleted++;
            i--;
        }
    }
    return deleted;
}

/*
using namespace std::chrono;
    using namespace std;

    char buff[11];
    system_clock::time_point tp = system_clock::now() - std::chrono::hours(READOUTS_BUFFER_PERIOD);
    time_t t = system_clock::to_time_t(tp);
    strftime(buff, 11, "%Y-%m-%d", localtime(&t));
    string timeStartString(buff);
    tp = system_clock::now();
    t = system_clock::to_time_t(tp);
    strftime(buff, 11, "%Y-%m-%d", localtime(&t));
    string timeNowString(buff);
    //    LOG_I("file timeStart: " + timeStartString);
    //    LOG_I("file timeNow: " + timeNowString);


    string fileName = idToString(id) + "_" + typeToString(type) + "_" + timeStartString + ".csv";
    string filePath = std::string(PATH_DATA) + "/" + idToString(id) + "/";
    ifstream fs;
    fs.open(filePath + fileName, std::ios::in);
    if (!fs.is_open()) {
        LOG_E("loadReadoutsBuffer()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
        return -1;
    }

    // read whole file
    string line;
    while (getline(fs, line)) {
        vector<string> cells = split(line, ",");

        // if the line is deformed skip it
        if (cells.size() < 4) {
            continue;
        }
        string cellTime = cells[0];
        std::tm tmm = {};
        time_t t = time(0); // get time now
        std::tm now = *localtime(& t);
        strptime(cellTime.c_str(), "%Y-%m-%d %H:%M:%S", &tmm);
        now.tm_year = tmm.tm_year;
        now.tm_mon = tmm.tm_mon;
        now.tm_mday = tmm.tm_mday;
        now.tm_min = tmm.tm_min;
        now.tm_sec = tmm.tm_sec;
        std::cout << "h: " << now.tm_hour << ", m: " << now.tm_min << std::endl;
        std::time_t t1 = std::mktime(&tmm);
        system_clock::time_point tp = system_clock::from_time_t(t1);
        std::time_t t2 = system_clock::to_time_t(tp);
        char buff2[20];
        strftime(buff2, 20, "%Y-%m-%d %H:%M:%S", localtime(&t1));
        std::string str2(buff2);
        std::cout << str2 << std::endl;
        strftime(buff2, 20, "%Y-%m-%d %H:%M:%S", gmtime(&t2));
        str2 = buff2;
        std::cout << str2 << std::endl;

        readout r;
        r.time = tp;
        r.temperature = stoi(cells[1]);
        r.pressure = stoi(cells[2]);
        r.humidity = stoi(cells[3]);
        readoutsBuffer.push_back(r);
        //            char buff2[20];
        //            time_t t = system_clock::to_time_t(tp);
        //            strftime(buff2, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
        //            string str2(buff2);
        //            LOG_I("loaded time: " + str2);
    }
    fs.close();
 */
/*
 * File:   DeviceBME280.cpp
 * Author: Martin
 *
 * Created on 23. dubna 2017, 20:45
 */

#include "DeviceBME280.h"
#define OBJECT_NAME "BME280"
#include "Log.h"
#include "PacketManager.h"

#include <string>
#include <sstream>
#include <fstream>
#include <string.h> // for strerror
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <limits> // min/max value for a variable
#include <iomanip> // setprecision

#define READOUTS_BUFFER_PERIOD 86400 // in seconds
#define NUMBER_POINTS_IN_GRAPH  200
using json = nlohmann::json;

DeviceBME280::DeviceBME280(int id, Interface interface) : DeviceGeneric(id, Type::BME280, interface) {
    status = "unknown";
    packetCounter = -1;
    lostReadouts = 0;
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
        } else if (parameter.find("voltage") != parameter.end()) {
            lastReadout.voltage = parameter["voltage"];
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
    device["voltage"] = lastReadout.voltage;
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

    std::string msgType = msg["type"];
    int srcId = msg["srcId"];

    // if the interface is RF24
    if (srcId == RF24SERVER_ID) {

        // update pipe index
        pipeIndex = msg["pipeIndex"];
        if (msgType.compare("dataReceived") == 0) {

            // convert json data to vector
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
            // if packet counter is zero, it goes from 255 to 1 (it skips 0)
            if (data[1] == 0) {
                LOG_I("processMsgFromDevice() Device started.");
                packetCounter = 0;
            } else {

                // if it is not the first received packet
                if (packetCounter >= 0) {
                    // increment packet counter
                    packetCounter = (packetCounter + 1) % 256;
                    if (packetCounter == 0) {
                        packetCounter = 1;
                    }

                    // if the packet counter does not match the received number
                    if (packetCounter != data[1]) {
                        LOG_E("processMsgFromDevice() packets lost: expected " + std::to_string(packetCounter) +
                                ", received " + std::to_string(data[1]));
                        uint32_t *dataUI32 = (uint32_t*) & data;
                        LOG_E("processMsgFromDevice() total lost packets " + std::to_string(dataUI32[5]));
                    }
                }
                packetCounter = data[1];
            }

            gettimeofday(&lastConnected, NULL);

            // get readouts from received packet
            readout currentReadout;
            float *dataF = (float*) &data;
            currentReadout.temperature = dataF[1];
            currentReadout.pressure = dataF[2];
            currentReadout.humidity = dataF[3];
            currentReadout.voltage = dataF[4];
            currentReadout.time = lastConnected;
            uint32_t *dataUI32 = (uint32_t*) & data;
            unsigned int receivedLostReadouts = dataUI32[5];

            // if the received readouts differs for more then 30% from previous readouts
            if (abs(currentReadout.temperature - lastReadout.temperature) > (lastReadout.temperature * 0.3)) {
                std::stringstream ss;
                ss << "Possible outliner detected: previous value = " << lastReadout.temperature;
                ss << " degC, received value = " << currentReadout.temperature << " degC.";
                LOG_W(ss.str());
            }
            if (abs(currentReadout.pressure - lastReadout.pressure) > (lastReadout.pressure * 0.3)) {
                std::stringstream ss;
                ss << "Possible outliner detected: previous value = " << lastReadout.pressure;
                ss << " Pa, received value = " << currentReadout.pressure << " Pa.";
                LOG_W(ss.str());
            }
            if (abs(currentReadout.humidity - lastReadout.humidity) > (lastReadout.humidity * 0.3)) {
                std::stringstream ss;
                ss << "Possible outliner detected: previous value = " << lastReadout.humidity;
                ss << " %, received value = " << currentReadout.humidity << " %";
                LOG_W(ss.str());
            }

            /*
            // if some readouts were lost
            if (receivedLostReadouts != lostReadouts) {

                // do not print error if this is the first received packet or the device just started
                if (!isWaitingForFirstData && packetCounter!=0 ) {
                    std::stringstream ss;
                    ss << "Lost readout detected: count = " << (receivedLostReadouts - lostReadouts);
                    ss << ", total = " << (lostReadouts + receivedLostReadouts) << ".";
                    LOG_E(ss.str());
                }
            }
             */
            // save received readouts
            lastReadout.temperature = currentReadout.temperature;
            lastReadout.pressure = currentReadout.pressure;
            lastReadout.humidity = currentReadout.humidity;
            lastReadout.voltage = currentReadout.voltage;
            lastReadout.time = currentReadout.time;
            lostReadouts = receivedLostReadouts;
            isWaitingForFirstData = false;


            std::stringstream ss;
            ss << "temperature = " << lastReadout.temperature;
            ss << ", pressure = " << lastReadout.pressure;
            ss << ", humidity = " << lastReadout.humidity;
            ss << ", voltage = " << lastReadout.voltage;
            //            LOG_I(ss.str());

            // create message for web server
            reply["type"] = "pushNewData";
            reply["lastConnected"] = timeToStringLocal(lastConnected);
            reply["data"] = {
                {"temperature", lastReadout.temperature},
                {"pressure", lastReadout.pressure},
                {"humidity", lastReadout.humidity},
                {"voltage", lastReadout.voltage},
                {"time", timeToStringLocal(lastReadout.time)}
            };

            readoutsBuffer.push_back(lastReadout);
            saveLastReadout();
            removeOldReadouts();
            saveDeviceToFile();
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

int DeviceBME280::processMsgFromGui(const nlohmann::json& msg, nlohmann::json & reply) {

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

        std::string subType;
        if (msg.find("subType") != msg.end()) {
            subType = msg["subType"];
        } else {
            subType = "day";
        }

        readouts_averaged readouts;

        if (subType.compare("day") == 0) {
            getReadoutsDay(readouts);
        } else if (subType.compare("week") == 0) {
            getReadoutsWeek(readouts);
        } else if (subType.compare("month") == 0) {
            getReadoutsMonth(readouts);
        } else if (subType.compare("year") == 0) {
            getReadoutsYear(readouts);
        } else {
            LOG_W("processMsgFromGui() unknown subtype: " + subType);
            return -1;
        }


        reply["desId"] = msg["srcId"];
        reply["type"] = "pushDataBuffer";
        reply["subType"] = subType;
        reply["data"] = {
            {"temperature", readouts.temperature,},
            {"temperatureMin", readouts.temperatureMin,},
            {"temperatureMax", readouts.temperatureMax,},
            {"pressure", readouts.pressure},
            {"pressureMin", readouts.pressureMin},
            {"pressureMax", readouts.pressureMax},
            {"humidity", readouts.humidity},
            {"humidityMin", readouts.humidityMin},
            {"humidityMax", readouts.humidityMax},
            {"time", readouts.time}
        };

        LOG_I("Size of message is " + std::to_string(reply.dump().size()) + " characters.");


        //        std::string path = "msg_dump.json";
        //        std::ofstream out_file;
        //        out_file.open(path, std::ios::out);
        //        if (!out_file.is_open()) {
        //            LOG_E("opening file " + path + ": " + std::string(strerror(errno)));
        //            return -1;
        //        }
        //
        //        out_file << reply.dump(3);
        //        out_file.close();


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
    std::string filePath = std::string(PATH_DATA) + idString + "/";
    std::ofstream fs;
    fs.open(filePath + fileName, std::ios::out | std::ios::app);
    if (!fs.is_open()) {
        LOG_E("opening file " + fileName + ": " + std::string(strerror(errno)));
        return -1;
    }
    fs << timeToString(lastReadout.time) << ", ";
    fs << std::setprecision(5) << lastReadout.temperature << ", ";
    fs << std::setprecision(6) << lastReadout.pressure << ", ";
    fs << std::setprecision(5) << lastReadout.humidity << ", ";
    fs << std::setprecision(4) << lastReadout.voltage << ", " << std::endl;
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
    string filePath = std::string(PATH_DATA) + idToString(id) + "/";
    ifstream fs;
    fs.open(filePath + fileName, std::ios::in);
    if (!fs.is_open()) {
        LOG_W("loadReadoutsBuffer()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
    } else {

        // read whole file
        string line;
        while (getline(fs, line)) {
            vector<string> cells = split(line, ",");

            // if the line is deformed skip it
            if (cells.size() < 5) {
                continue;
            }

            readout r = stringToReadout(cells);
            //        LOG_I("1loaded time      : " + timeToString(r.time));
            readoutsBuffer.push_back(r);
            //        LOG_I("1loaded time local: " + timeToStringLocal(r.time));

        }
    }
    fs.close();

    // data are loaded from two files
    if (timeStartString.compare(timeNowString) != 0) {
        fileName = idToString(id) + "_" + typeToString(type) + "_" + timeNowString + ".csv";
        filePath = string(PATH_DATA) + idToString(id) + "/";
        ifstream fs;
        fs.open(filePath + fileName, ios::in);
        if (!fs.is_open()) {
            LOG_W("loadReadoutsBuffer()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
            return -1;
        }

        string line;
        while (getline(fs, line)) {
            vector<string> cells = split(line, ",");

            // if the line is deformed skip it
            if (cells.size() < 5) {
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

    r.temperature = stof(cells[1]);
    r.pressure = stof(cells[2]);
    r.humidity = stof(cells[3]);
    r.voltage = stof(cells[4]);
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
    //    LOG_I("timeBreak: " + timeToString(timeBreak));
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

int DeviceBME280::getReadoutsDay(readouts_averaged &readouts) {
    if (readoutsBuffer.empty()) {
        LOG_W("processMsgFromGui() readout buffer is empty");
        return -1;
    }

    //    float sumTemperature = 0;
    //    float minTemperature = std::numeric_limits<float>::max();
    //    float maxTemperature = std::numeric_limits<float>::lowest();
    //    float sumPressure = 0;
    //    float minPressure = std::numeric_limits<float>::max();
    //    float maxPressure = std::numeric_limits<float>::lowest();
    //    float sumHumidity = 0;
    //    float minHumidity = std::numeric_limits<float>::max();
    //    float maxHumidity = std::numeric_limits<float>::lowest();
    //    struct timeval sumTime = readoutsBuffer[readoutsBuffer.size() - 1].time;
    int size = floor(readoutsBuffer.size() / NUMBER_POINTS_IN_GRAPH);
    size = size < 2 ? 1 : size;
    for (int i = readoutsBuffer.size() - 1; i > (size - 2);) {
        float sumTemperature = 0;
        float minTemperature = std::numeric_limits<float>::max();
        float maxTemperature = std::numeric_limits<float>::lowest();
        float sumPressure = 0;
        float minPressure = std::numeric_limits<float>::max();
        float maxPressure = std::numeric_limits<float>::lowest();
        float sumHumidity = 0;
        float minHumidity = std::numeric_limits<float>::max();
        float maxHumidity = std::numeric_limits<float>::lowest();
        struct timeval sumTime = readoutsBuffer[i].time;
        int sumCounter = 0;

        for (int j = 0; j < size; ++j) {
            if (i < 0) {
                break;
            }
            sumTemperature += readoutsBuffer[i].temperature;
            sumPressure += readoutsBuffer[i].pressure;
            sumHumidity += readoutsBuffer[i].humidity;

            if (readoutsBuffer[i].temperature < minTemperature)
                minTemperature = readoutsBuffer[i].temperature;
            if (readoutsBuffer[i].temperature > maxTemperature)
                maxTemperature = readoutsBuffer[i].temperature;

            if (readoutsBuffer[i].pressure < minPressure)
                minPressure = readoutsBuffer[i].pressure;
            if (readoutsBuffer[i].pressure > maxPressure)
                maxPressure = readoutsBuffer[i].pressure;

            if (readoutsBuffer[i].humidity < minHumidity)
                minHumidity = readoutsBuffer[i].humidity;
            if (readoutsBuffer[i].humidity > maxHumidity)
                maxHumidity = readoutsBuffer[i].humidity;

            i--;
            sumCounter++;
        }
        readouts.temperature.insert(readouts.temperature.begin(), round(sumTemperature * 100 / sumCounter));
        readouts.temperatureMin.insert(readouts.temperatureMin.begin(), round(minTemperature * 100));
        readouts.temperatureMax.insert(readouts.temperatureMax.begin(), round(maxTemperature * 100));
        readouts.pressure.insert(readouts.pressure.begin(), round(sumPressure / sumCounter));
        readouts.pressureMin.insert(readouts.pressureMin.begin(), round(minPressure));
        readouts.pressureMax.insert(readouts.pressureMax.begin(), round(maxPressure));
        readouts.humidity.insert(readouts.humidity.begin(), round(sumHumidity * 100 / sumCounter));
        readouts.humidityMin.insert(readouts.humidityMin.begin(), round(minHumidity * 100));
        readouts.humidityMax.insert(readouts.humidityMax.begin(), round(maxHumidity * 100));
        readouts.time.insert(readouts.time.begin(), timeToStringLocal(sumTime));

        //        sumTemperature = 0;
        //        minTemperature = std::numeric_limits<float>::max();
        //        maxTemperature = std::numeric_limits<float>::lowest();
        //        sumPressure = 0;
        //        minPressure = std::numeric_limits<float>::max();
        //        maxPressure = std::numeric_limits<float>::lowest();
        //        sumHumidity = 0;
        //        minHumidity = std::numeric_limits<float>::max();
        //        maxHumidity = std::numeric_limits<float>::lowest();
    }
    LOG_I("buffer size: " + std::to_string(readouts.temperature.size()));
}

int DeviceBME280::getReadoutsWeek(readouts_averaged &readouts) {

    using namespace std;
    LOG_I("start");
    for (int day = 6; day >= 0; day--) {

        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        timeNow.tv_sec -= day * 24 * 60 * 60;
        string timeStartString = timeToString(timeNow, "%Y-%m-%d");
        string fileName = idToString(id) + "_" + typeToString(type) + "_" + timeStartString + ".csv";
        string filePath = std::string(PATH_DATA) + idToString(id) + "/";

        //LOG_I("file: " + fileName);
        ifstream fs;
        fs.open(filePath + fileName, std::ios::in);
        if (!fs.is_open()) {
            LOG_W("getReadoutsWeek()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
            continue;
        } else {
            // read whole file
            string line;
            int lastHour = -1;
            float sumTemperature = 0;
            float minTemperature = std::numeric_limits<float>::max();
            float maxTemperature = std::numeric_limits<float>::lowest();
            float sumPressure = 0;
            float minPressure = std::numeric_limits<float>::max();
            float maxPressure = std::numeric_limits<float>::lowest();
            float sumHumidity = 0;
            float minHumidity = std::numeric_limits<float>::max();
            float maxHumidity = std::numeric_limits<float>::lowest();
            int sumCounter = 0;
            readoutString rs;

            // get line from the file
            while (getline(fs, line)) {

                rs = splitReadout(line);
                if (!rs.isValid) {
                    LOG_E("invalid line: " + line);
                    continue;
                }

                // get hour of the current readout
                int hour = stoi(rs.hours);
                if (lastHour < 0) {
                    lastHour = hour;
                }

                // if the hour is different from previous readout
                if ((hour >= (lastHour + 1))) {

                    string readoutTime;
                    readoutTime = rs.date + " " + to_string(lastHour) + ":30:00";
                    timeval t = stringToTime(readoutTime);
                    //t.tv_sec += 3600;
                    readoutTime = timeToStringLocal(t);

                    readouts.temperature.push_back(round(sumTemperature * 100 / sumCounter));
                    readouts.temperatureMin.push_back(round(minTemperature * 100));
                    readouts.temperatureMax.push_back(round(maxTemperature * 100));
                    readouts.pressure.push_back((sumPressure / sumCounter));
                    readouts.pressureMin.push_back(minPressure);
                    readouts.pressureMax.push_back(maxPressure);
                    readouts.humidity.push_back((sumHumidity * 100 / sumCounter));
                    readouts.humidityMin.push_back(minHumidity * 100);
                    readouts.humidityMax.push_back(maxHumidity * 100);
                    readouts.time.push_back(readoutTime);

                    lastHour = hour;
                    sumTemperature = 0;
                    minTemperature = std::numeric_limits<float>::max();
                    maxTemperature = std::numeric_limits<float>::lowest();
                    sumPressure = 0;
                    minPressure = std::numeric_limits<float>::max();
                    maxPressure = std::numeric_limits<float>::lowest();
                    sumHumidity = 0;
                    minHumidity = std::numeric_limits<float>::max();
                    maxHumidity = std::numeric_limits<float>::lowest();

                    sumCounter = 0;
                    //                    LOG_I("time: " + readoutTime);
                }


                float temperature = stof(rs.temperature);
                float pressure = stof(rs.pressure);
                float humidity = stof(rs.humidity);

                sumTemperature += temperature;
                sumPressure += pressure;
                sumHumidity += humidity;

                if (temperature < minTemperature)
                    minTemperature = temperature;
                if (temperature > maxTemperature)
                    maxTemperature = temperature;

                if (pressure < minPressure)
                    minPressure = pressure;
                if (pressure > maxPressure)
                    maxPressure = pressure;

                if (humidity < minHumidity)
                    minHumidity = humidity;
                if (humidity > maxHumidity)
                    maxHumidity = humidity;

                sumCounter++;
            }
            if (rs.isValid) {
                string readoutTime;
                readoutTime = rs.date + " " + to_string(lastHour) + ":30:00";
                timeval t = stringToTime(readoutTime);
                //t.tv_sec += 3600;
                readoutTime = timeToStringLocal(t);
                readouts.temperature.push_back(round(sumTemperature * 100 / sumCounter));
                readouts.temperatureMin.push_back(round(minTemperature * 100));
                readouts.temperatureMax.push_back(round(maxTemperature * 100));
                readouts.pressure.push_back((sumPressure / sumCounter));
                readouts.pressureMin.push_back(minPressure);
                readouts.pressureMax.push_back(maxPressure);
                readouts.humidity.push_back((sumHumidity * 100 / sumCounter));
                readouts.humidityMin.push_back(minHumidity * 100);
                readouts.humidityMax.push_back(maxHumidity * 100);
                readouts.time.push_back(readoutTime);

                //                LOG_I("time: " + readoutTime);
            }
        }
        fs.close();

    }
    LOG_I("buffer size: " + to_string(readouts.temperature.size()));
    LOG_I("done");

}

int DeviceBME280::getReadoutsMonth(readouts_averaged &readouts) {
    using namespace std;

    LOG_I("start");
    for (int day = 30; day >= 0; day--) {

        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        timeNow.tv_sec -= day * 24 * 60 * 60;
        string timeStartString = timeToString(timeNow, "%Y-%m-%d");
        string fileName = idToString(id) + "_" + typeToString(type) + "_" + timeStartString + ".csv";
        string filePath = std::string(PATH_DATA) + idToString(id) + "/";

        //LOG_I("file: " + fileName);
        ifstream fs;
        fs.open(filePath + fileName, std::ios::in);
        if (!fs.is_open()) {
            LOG_W("getReadoutsWeek()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
            continue;
        } else {
            // read whole file
            string line;
            int lastHour = -1;
            float sumTemperature = 0;
            float minTemperature = std::numeric_limits<float>::max();
            float maxTemperature = std::numeric_limits<float>::lowest();
            float sumPressure = 0;
            float minPressure = std::numeric_limits<float>::max();
            float maxPressure = std::numeric_limits<float>::lowest();
            float sumHumidity = 0;
            float minHumidity = std::numeric_limits<float>::max();
            float maxHumidity = std::numeric_limits<float>::lowest();
            int sumCounter = 0;
            readoutString rs;

            // get line from the file
            while (getline(fs, line)) {

                rs = splitReadout(line);
                if (!rs.isValid) {
                    LOG_E("invalid line: " + line);
                    continue;
                }

                // get hour of the current readout
                int hour = stoi(rs.hours);
                if (lastHour < 0) {
                    lastHour = hour;
                }

                // if the hour is different from previous readout
                if ((hour >= (lastHour + 1))) {

                    string readoutTime;
                    readoutTime = rs.date + " " + to_string(lastHour) + ":30:00";
                    timeval t = stringToTime(readoutTime);
                    //t.tv_sec += 3600;
                    readoutTime = timeToStringLocal(t);

                    readouts.temperature.push_back(round(sumTemperature * 100 / sumCounter));
                    readouts.temperatureMin.push_back(round(minTemperature * 100));
                    readouts.temperatureMax.push_back(round(maxTemperature * 100));
                    readouts.pressure.push_back((sumPressure / sumCounter));
                    readouts.pressureMin.push_back(minPressure);
                    readouts.pressureMax.push_back(maxPressure);
                    readouts.humidity.push_back((sumHumidity * 100 / sumCounter));
                    readouts.humidityMin.push_back(minHumidity * 100);
                    readouts.humidityMax.push_back(maxHumidity * 100);
                    readouts.time.push_back(readoutTime);

                    lastHour = hour;
                    sumTemperature = 0;
                    minTemperature = std::numeric_limits<float>::max();
                    maxTemperature = std::numeric_limits<float>::lowest();
                    sumPressure = 0;
                    minPressure = std::numeric_limits<float>::max();
                    maxPressure = std::numeric_limits<float>::lowest();
                    sumHumidity = 0;
                    minHumidity = std::numeric_limits<float>::max();
                    maxHumidity = std::numeric_limits<float>::lowest();

                    sumCounter = 0;
                    //                    LOG_I("time: " + readoutTime);
                }


                float temperature = stof(rs.temperature);
                float pressure = stof(rs.pressure);
                float humidity = stof(rs.humidity);

                sumTemperature += temperature;
                sumPressure += pressure;
                sumHumidity += humidity;

                if (temperature < minTemperature)
                    minTemperature = temperature;
                if (temperature > maxTemperature)
                    maxTemperature = temperature;

                if (pressure < minPressure)
                    minPressure = pressure;
                if (pressure > maxPressure)
                    maxPressure = pressure;

                if (humidity < minHumidity)
                    minHumidity = humidity;
                if (humidity > maxHumidity)
                    maxHumidity = humidity;

                sumCounter++;
            }
            if (rs.isValid) {
                string readoutTime;
                readoutTime = rs.date + " " + to_string(lastHour) + ":30:00";
                timeval t = stringToTime(readoutTime);
                //t.tv_sec += 3600;
                readoutTime = timeToStringLocal(t);
                readouts.temperature.push_back(round(sumTemperature * 100 / sumCounter));
                readouts.temperatureMin.push_back(round(minTemperature * 100));
                readouts.temperatureMax.push_back(round(maxTemperature * 100));
                readouts.pressure.push_back((sumPressure / sumCounter));
                readouts.pressureMin.push_back(minPressure);
                readouts.pressureMax.push_back(maxPressure);
                readouts.humidity.push_back((sumHumidity * 100 / sumCounter));
                readouts.humidityMin.push_back(minHumidity * 100);
                readouts.humidityMax.push_back(maxHumidity * 100);
                readouts.time.push_back(readoutTime);
                //                LOG_I("time: " + readoutTime);
            }
        }
        fs.close();

    }
    LOG_I("buffer size: " + to_string(readouts.temperature.size()));
    LOG_I("done");
    return 0;
}

int DeviceBME280::getReadoutsYear(readouts_averaged& readouts) {
    using namespace std;

    LOG_I("start of GetYear");
    for (int day = 365; day >= 0; day--) {

        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        timeNow.tv_sec -= day * 24 * 60 * 60;
        string timeStartString = timeToString(timeNow, "%Y-%m-%d");
        string fileName = idToString(id) + "_" + typeToString(type) + "_" + timeStartString + ".csv";
        string filePath = std::string(PATH_DATA) + idToString(id) + "/";

        //LOG_I("file: " + fileName);
        ifstream fs;
        fs.open(filePath + fileName, std::ios::in);
        if (!fs.is_open()) {
            //LOG_W("getReadoutsWeek()  opening file " + filePath + fileName + ": " + string(strerror(errno)));
            continue;
        } else {
            // read whole file
            string line;
            //            int lastHour = -1;
            float sumTemperature = 0;
            float minTemperature = std::numeric_limits<float>::max();
            float maxTemperature = std::numeric_limits<float>::lowest();
            float sumPressure = 0;
            float minPressure = std::numeric_limits<float>::max();
            float maxPressure = std::numeric_limits<float>::lowest();
            float sumHumidity = 0;
            float minHumidity = std::numeric_limits<float>::max();
            float maxHumidity = std::numeric_limits<float>::lowest();
            int sumCounter = 0;
            readoutString rs;

            // get line from the file
            while (getline(fs, line)) {
                if (line.empty())
                    continue;

                rs = splitReadout(line);
                if (!rs.isValid) {
                    LOG_E("invalid line: " + line);
                    continue;
                }

                float temperature = stof(rs.temperature);
                float pressure = stof(rs.pressure);
                float humidity = stof(rs.humidity);

                sumTemperature += temperature;
                sumPressure += pressure;
                sumHumidity += humidity;

                if (temperature < minTemperature)
                    minTemperature = temperature;
                if (temperature > maxTemperature)
                    maxTemperature = temperature;

                if (pressure < minPressure)
                    minPressure = pressure;
                if (pressure > maxPressure)
                    maxPressure = pressure;

                if (humidity < minHumidity)
                    minHumidity = humidity;
                if (humidity > maxHumidity)
                    maxHumidity = humidity;

                sumCounter++;
            }
            if (rs.isValid) {
                string readoutTime;
                readoutTime = rs.date + " 12:00:00";
                timeval t = stringToTime(readoutTime);
                //t.tv_sec += 3600;
                readoutTime = timeToStringLocal(t);
                readouts.temperature.push_back(round(sumTemperature * 100 / sumCounter));
                readouts.temperatureMin.push_back(round(minTemperature * 100));
                readouts.temperatureMax.push_back(round(maxTemperature * 100));
                readouts.pressure.push_back((sumPressure / sumCounter));
                readouts.pressureMin.push_back(minPressure);
                readouts.pressureMax.push_back(maxPressure);
                readouts.humidity.push_back((sumHumidity * 100 / sumCounter));
                readouts.humidityMin.push_back(minHumidity * 100);
                readouts.humidityMax.push_back(maxHumidity * 100);
                readouts.time.push_back(readoutTime);
                //                LOG_I("time: " + readoutTime);
            }
        }
        fs.close();

    }
    LOG_I("buffer size: " + to_string(readouts.temperature.size()));
    LOG_I("done");
    return 0;
}

DeviceBME280::readoutString DeviceBME280::splitReadout(std::string line) {
    // 2018-01-09 18:07:03, 19.68, 97324.7, 67.665, 1.383,
    // |    0   | 1  2  3   | 4 |  |  5  |  |  6 |  | 7 |
    using namespace std;

    // make sure the last character is a comma
    line.append(",");
    readoutString out;
    string::iterator itCurrent = line.begin();
    string::iterator itLast = line.begin();
    const char tokens[] = {' ', ':', ':', ',', ',', ',', ',', ','};
    int tokenIndex = 0;
    int tokensSize = sizeof (tokens);

    out.isValid = false;

    //    cout << "split readout: " << endl;
    for (itCurrent = line.begin(); itCurrent < line.end(); itCurrent++) {

        if (tokenIndex >= tokensSize) {
            out.isValid = true;
            break;
        }
        //        cout<< "<" << *itCurrent << ">";
        if (*itCurrent == tokens[tokenIndex]) {

            string cell;
            if (tokenIndex == 0) {
                cell = string(itLast, itCurrent);
            } else {
                cell = string(itLast + 1, itCurrent);
            }
            itLast = itCurrent;
            //            cout << "[" << cell << "] ";

            switch (tokenIndex) {
                case 0: out.date = cell;
                    break;
                case 1: out.hours = cell;
                    break;
                case 2: out.minutes = cell;
                    break;
                case 3: out.seconds = cell;
                    break;
                case 4: out.temperature = cell;
                    break;
                case 5: out.pressure = cell;
                    break;
                case 6: out.humidity = cell;
                    break;
                case 7: out.voltage = cell;
                    break;
                default: break;
            }
            tokenIndex++;
        }

    }
    //    cout << endl;
    //    cout << out.date << " " << out.temperature << endl;
    return out;
}

std::string DeviceBME280::getOledMessage() {
    std::stringstream ss;
    ss << std::to_string(id) << ": ";
    ss << std::fixed << std::setprecision(1) << (round(lastReadout.temperature * 10) / 10);
    ss << "$C";
    return ss.str();
}

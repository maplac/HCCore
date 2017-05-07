/* 
 * File:   Log.cpp
 * Author: Martin
 * 
 * Created on 24. září 2016, 19:22
 */

#include "Log.h"
#include <iostream>
#include <sstream>
#include <ctime>
#include <chrono>

#define NAME_LENGTH     15

void Log::info(std::string name, std::string msg) {
    std::stringstream ss;
    ss << "[" << Log::getCurrentTime() << "][INFO ] " << "[" << name;
    for (int i = name.length(); i < NAME_LENGTH; i++) {
        ss << " ";
    }
    ss << "] " << msg << std::endl;
    std::cout << ss.str();
}

void Log::warn(std::string name, std::string msg) {
    std::stringstream ss;
    ss << "[" << Log::getCurrentTime() << "][WARN ] " << "[" << name;
    for (int i = name.length(); i < NAME_LENGTH; i++) {
        ss << " ";
    }
    ss << "] " << msg << std::endl;
    std::cout << ss.str();
}

void Log::error(std::string name, std::string msg) {
    std::stringstream ss;
    ss << "[" << Log::getCurrentTime() << "][ERROR] " << "[" << name;
    for (int i = name.length(); i < NAME_LENGTH; i++) {
        ss << " ";
    }
    ss << "] " << msg << std::endl;
    std::cout << ss.str();
}

std::string Log::getCurrentTime() {
    std::chrono::system_clock::time_point nowTime = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(nowTime);
    auto seconds = std::chrono::time_point_cast<std::chrono::seconds>(nowTime);
    auto fraction = nowTime - seconds;
    std::chrono::milliseconds milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(fraction);
    char buff[20];
    int res = strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    buff[res] = 0;
    std::string str(buff);
    int millis = milliseconds.count();
    str.append(".");
    if (millis == 0) {
        str.append("000");
    } else {
        if (millis < 10)
            str.append("0");
        if (millis < 100)
            str.append("0");
        str.append(std::to_string(millis));
    }
    return str;
}

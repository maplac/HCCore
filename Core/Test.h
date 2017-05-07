/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Test.h
 * Author: Martin
 *
 * Created on 15. ledna 2017, 19:36
 */

#ifndef TEST_H
#define TEST_H

#include <time.h>
#include <ctime>
#include <chrono>
#include <string>
#include <deque>
#include <json.hpp>

class Sample {
public:
    Sample();
    virtual ~Sample();
    double temperature;
    double humidity;
    std::string time;
};


class Test {
public:
    Test();
    virtual ~Test();
    
    std::string timeToString(const std::chrono::system_clock::time_point& tp);
    Sample * generateSample();
    void getSamples(nlohmann::json &msg);
    
    std::deque<Sample*> samples;

};

#endif /* TEST_H */


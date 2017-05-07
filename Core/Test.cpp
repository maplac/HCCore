/* 
 * File:   Test.cpp
 * Author: Martin
 * 
 * Created on 15. ledna 2017, 19:36
 */

#include "Test.h"
#include <algorithm>

using json = nlohmann::json;

Sample::Sample() {
}

Sample::~Sample() {
}

Test::Test() {
}

Test::~Test() {
    for(Sample * s : samples){
        delete s;
    }
}

std::string Test::timeToString(const std::chrono::system_clock::time_point& tp) {
    std::time_t t = std::chrono::system_clock::to_time_t(tp);
    char buff[20];
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
    std::string str(buff);
    return str;
}

Sample * Test::generateSample() {
    
    while (samples.size() > 20){
        samples.pop_back();
    }
    
    Sample * s = new Sample();
    s->time = timeToString(std::chrono::system_clock::now());
    
    int r = rand() % 500 - 100;
    s->temperature = r/10.0;
    r = rand() % 500 + 500;
    s->humidity = r/10.0;
    
    samples.push_front(s);
    return s;
}

void Test::getSamples(json & msg) {
    std::vector<double> te;
    std::vector<double> hu;
    std::vector<std::string> ti;
    
    for(Sample * s : samples){
        te.push_back(s->temperature);
        hu.push_back(s->humidity);
        ti.push_back(s->time);
    }
    std::reverse(te.begin(), te.end());
    std::reverse(hu.begin(), hu.end());
    std::reverse(ti.begin(), ti.end());
    msg["temperature"] = json(te);
    msg["humidity"] = json(hu);
    msg["time"] = json(ti);
}

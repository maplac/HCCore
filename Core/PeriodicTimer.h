/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PeriodicTimer.h
 * Author: Martin
 *
 * Created on 29. prosince 2016, 19:45
 */

#ifndef PERIODICTIMER_H
#define PERIODICTIMER_H

#include <thread>
#include <atomic>
#include <zmq.hpp>

#define PERIOD_TIMER_ID     -203

class PeriodicTimer {
public:
    PeriodicTimer(zmq::context_t *zmqContext, int id, int period = 3600000);
    virtual ~PeriodicTimer();
    void start();
    void start(int period);
    void stop();
    bool isRunning();
    void setPeriod(int period);
    int getPeriod();
    

private:
    int threadPipe[2];
    std::thread thread;
    std::atomic<bool> isActive;
    std::atomic<bool> isExiting;
    std::atomic<int> period;
    zmq::context_t *zmqContext;
    int id;
   
    void run();

};

#endif /* PERIODICTIMER_H */


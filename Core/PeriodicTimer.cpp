/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PeriodicTimer.cpp
 * Author: Martin
 * 
 * Created on 29. prosince 2016, 19:45
 */


#include "PeriodicTimer.h"
#define OBJECT_NAME     "Timer"
#include "Log.h"

#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <zhelpers.hpp>
#include <json.hpp>


using json = nlohmann::json;

PeriodicTimer::PeriodicTimer(zmq::context_t *zmqContext, int id, int period) {
    this->zmqContext = zmqContext;
    this->period = period;
    this->id = id;
    isExiting = false;
    isActive= false;
    pipe(threadPipe);
    thread = std::thread(&PeriodicTimer::run, this);
    LOG_I("created");
}

PeriodicTimer::~PeriodicTimer() {

    isActive= false;
    isExiting = true;
    const char *msg = "q";
    write(threadPipe[1], msg, strlen(msg));

    if (thread.joinable())
        thread.join();

    close(threadPipe[0]);
    close(threadPipe[1]);
}

bool PeriodicTimer::isRunning() {
    return isActive.load();
}

void PeriodicTimer::start() {
    isActive= true;
    const char *msg = "i";
    write(threadPipe[1], msg, strlen(msg));
    LOG_I("start()");
}

void PeriodicTimer::start(int period) {
    this->period = period;
    start();
}

void PeriodicTimer::setPeriod(int period) {
    this->period = period;
}

void PeriodicTimer::stop() {
    if (!isRunning())
        return;
    
    isActive= false;
    const char *msg = "i";
    write(threadPipe[1], msg, strlen(msg));
    LOG_I("stop()");
}

void PeriodicTimer::run() {

    // Connect to 0MQ
    zmq::socket_t zmqSocket(*zmqContext, ZMQ_PUB);
    zmqSocket.connect("inproc://internalConnection");


    // prepare polling socket reading and pipe reading
    struct pollfd fds[1];
    int pollStatus;
    fds[0].fd = threadPipe[0];
    fds[0].events = POLLIN;

    LOG_I("started");
    while (!isExiting) {

        int timeout;
        if (isRunning()){
            timeout = period;
        }else{
            timeout = 3600000;
        }

        // wait until something happens
        pollStatus = poll(fds, 1, timeout);
        if (pollStatus < 0) {
            LOG_W("on poll: " + std::string(strerror(errno)));
            continue;
        }

        // poll timeouted
        if (pollStatus == 0) {
            
            // if the timer is active send message to the main thread
            if (isRunning()) {
                json msgTransmitJson = {
                    {"srcId", PERIOD_TIMER_ID},
                    {"type", "timeout"},
                    {"desId", id},
                };
                s_send(zmqSocket, msgTransmitJson.dump());
            }
        }


        // pipe received something
        if (fds[0].revents != 0) {
            if (fds[0].revents == POLLIN) {
                char msg;
                int len = read(threadPipe[0], &msg, 1);
                if (msg == 'q') {
                    isExiting = true;
                    break;
                } else if (msg == 'i') {
                    continue;
                }
            } else {
                LOG_W("event on pipe is not POLLIN");
            }
        }
    }
    zmqSocket.close();
    LOG_I("finished");
}

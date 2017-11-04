/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Martin
 *
 * Created on 4. listopadu 2017, 17:58
 */

// linker: -I/usr/local/lib -L/usr/local/lib -lzmq -pthread
// source path: /usr/local/lib

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>

#include <zmq.hpp>
#include <zhelpers.hpp>
//#include <thread>
#include <atomic>
#include <unistd.h>

std::atomic<bool> isRunning;

// ZQM

void my_handler(int s) {
//    std::cout << "Caught signal: " << std::to_string(s) << std::endl;
    isRunning = false;
}

//void delayedSend(zmq::socket_t *zmqSocketOut){
//    usleep(100000);
//    s_send(*zmqSocketOut, "get log");
//}

int main(int argc, char** argv) {
    isRunning = true;


    // initialize Ctrl-C catching
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);



    // create ZMQ context
    zmq::context_t zmqContext(1);


    zmq::socket_t zmqSocketOut(zmqContext, ZMQ_PUB);
    zmqSocketOut.bind("tcp://*:20002");

    // receiving requests from clients
    zmq::socket_t zmqSocketIn(zmqContext, ZMQ_SUB);
    zmqSocketIn.connect("tcp://localhost:20003");
    zmqSocketIn.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    //  Configure socket to not wait at close time
    //int linger = 0;
    //socket_zqm_in.setsockopt(ZMQ_LINGER, &linger, sizeof (linger));


    // structure for polling 0MQ sockets
    zmq::pollitem_t zmqPollItems[] = {
        { (void *) zmqSocketIn, 0, ZMQ_POLLIN, 0}
    };

//    std::thread sendThread(&delayedSend, &zmqSocketOut);
    
    std::string msgOut = "get log";
    
    usleep(200000);
    s_send(zmqSocketOut, msgOut);

    std::cout << "LogView started." << std::endl;
    while (isRunning.load()) {
        try {
            // wait for messages
            zmq::poll(&zmqPollItems[0], 1, -1);
        } catch (std::exception &e) {
//            std::cout << "Caught exception while polling ZMQ" << std::endl;
            isRunning = false;
            break;
        }

        //  MESSAGE FROM LOGGER
        if (zmqPollItems[0].revents && ZMQ_POLLIN) {
            std::string msgReceivedStr = s_recv(zmqSocketIn);

            //            std::cout << "msg:" << std::endl;
            std::cout << msgReceivedStr << std::endl;


        } else {
            if (zmqPollItems[0].revents && ZMQ_POLLERR) {
                std::cout << "error polling zmqSocketIn" << std::endl;
            }
        }
//        s_send(zmqSocketOut, msgOut);
    }
    std::cout << " LogView closing." << std::endl;

    zmqSocketOut.close();
    zmqSocketIn.close();
//    sendThread.join();
    
    return 0;
}


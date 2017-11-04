/* 
 * File:   main.cpp
 * Author: Martin
 *
 * Created on 5. června 2016, 20:08
 */


/* Linker: 
 * -I/usr/local/lib -L/usr/local/lib -lzmq -pthread
 * source folder: 
 * /usr/include/c++/4.9.2/experimental 
 * /usr/include/c++/4.9.2/ 
 * /usr/local/lib
 */


#include "mainLoop.h"
#define OBJECT_NAME     "main.cpp"
#include "Log.h"

#include <cstdlib>
#include <zmq.hpp>
#include <zhelpers.hpp>
#include <string>
#include <iostream>
#include "globals.h"



#define DEVICE_ID  201

#include <json.hpp>
using json = nlohmann::json;
using namespace std;

// ZQM
zmq::context_t zmqContext;

void my_handler(int s) {
    LOG_I("Caught signal: " + std::to_string(s));
    zmq::socket_t zmqSocket(zmqContext, ZMQ_PUB);
    zmqSocket.connect("inproc://internalConnection");
    isMainLoopInterrupted = 1;
    s_send(zmqSocket, "interrupt main thread");
}

int main(int argc, char** argv) {

    usleep(200000);
    bool isDaemon = false;

    for (int i = 0; i < argc; i++) {
        std::string arg = std::string(argv[i]);
        if (arg.compare("daemon") == 0) {
            isDaemon = true;
            LOG_I("HC is going to be a daemon.");
        }
    }

    if (isDaemon) {
        int res = daemon(1, 1);
        if (res != 0) {
            LOG_E("daemon call failed");
        }
    }

    // initialize Ctrl-C catching
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);

    // create ZMQ context
    zmqContext = zmq::context_t(1);
    
    isMainLoopInterrupted = 0;
    // start main loop
    mainLoop(zmqContext);

    LOG_I("Exit");
    
    exit(0);
}

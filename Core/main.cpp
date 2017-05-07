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
    LOG_I("Caught signal: "+ std::to_string(s));
    zmq::socket_t zmqSocket (zmqContext, ZMQ_PUB);
    zmqSocket.connect ("inproc://internalConnection");
    isMainLoopInterrupted = 1;
    s_send(zmqSocket, "interrupt main thread");
}

int main(int argc, char** argv) {

//    json par;
//    par["parameters"]={{"value",15}};
//    std::cout<<par.dump(3)<<std::endl;
//    std::cout<<(*par.begin()).dump(3)<<std::endl;
//    return 0;
    
    // initialize Ctrl-C catching
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // create ZMQ context
    zmqContext = zmq::context_t(1);
//    globalZmqContext = &zmqContext;
    
    // start main loop
    mainLoop(zmqContext);
    
    LOG_I("Exit");
    return 0;
}

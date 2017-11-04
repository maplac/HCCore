/* 
 * File:   main.cpp
 * Author: Martin
 *
 * Created on 20. srpna 2017, 21:57
 */

#include <cstdlib>
#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <queue>

#include <zmq.hpp>
#include <zhelpers.hpp>
#include <thread>
#include <atomic>
#include <sstream>
#include <mutex>

#define PATH_LOG    "/home/pi/HomeControl-Log/"

//using namespace std;

std::atomic<bool> isRunning;
std::deque<std::string> buffer;
std::mutex mutexBuffer;

// ZQM
zmq::context_t zmqContext;

std::string timeToString(const struct timeval &timestamp, const char * format = "%Y-%m-%d %H:%M:%S");

void my_handler(int s) {
    std::cout << "Caught signal: " << std::to_string(s) << std::endl;
    isRunning = false;
}

void zmqLoop(zmq::context_t * zmqContext) {
//    std::cout << "Zmq thread started." << std::endl;
    // sending reply to clients
    zmq::socket_t zmqSocketOut(*zmqContext, ZMQ_PUB);
    zmqSocketOut.bind("tcp://*:20003");

    // receiving requests from clients
    zmq::socket_t zmqSocketIn(*zmqContext, ZMQ_SUB);
    zmqSocketIn.connect("tcp://localhost:20002");
    zmqSocketIn.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    //  Configure socket to not wait at close time
    //int linger = 0;
    //socket_zqm_in.setsockopt(ZMQ_LINGER, &linger, sizeof (linger));

    zmq::socket_t zmqSocketIntern(*zmqContext, ZMQ_SUB);
    zmqSocketIntern.bind("inproc://loggerInternalConnection"); // bind parent, child connects
    zmqSocketIntern.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // structure for polling 0MQ sockets
    zmq::pollitem_t zmqPollItems[] = {
        { (void *) zmqSocketIn, 0, ZMQ_POLLIN, 0},
        { (void *) zmqSocketIntern, 0, ZMQ_POLLIN, 0}
    };

    std::cout << "Zmq loop started." << std::endl;
    while (isRunning.load()) {
        try {
            // wait for messages
            zmq::poll(&zmqPollItems[0], 2, -1);
        } catch (std::exception &e) {
            std::cout << "Caught exception while polling ZMQ" << std::endl;
            isRunning = false;
            break;
        }

        //  MESSAGE FROM INTERNAL THREADS
        if (zmqPollItems[1].revents && ZMQ_POLLIN) {

            // read and forward message
            std::string msgReceivedStr = s_recv(zmqSocketIntern);
            std::cout << "received: " << msgReceivedStr << std::endl;
            s_send(zmqSocketOut, msgReceivedStr);

        } else {
            if (zmqPollItems[1].revents && ZMQ_POLLERR) {
                std::cout << "error polling zmqSocketInternal" << std::endl;
            }
        }

        //  MESSAGE FROM CLIENT
        if (zmqPollItems[0].revents && ZMQ_POLLIN) {
            std::string msgReceivedStr = s_recv(zmqSocketIn);
            
            std::stringstream ss;
            
            mutexBuffer.lock();
            for (int i = 0; i < buffer.size(); i++) {
                ss << buffer[i];
            }
            mutexBuffer.unlock();
            
            std::cout << "msg:" << std::endl;
            std::cout << ss;
            
            s_send(zmqSocketOut, ss.str());

        } else {
            if (zmqPollItems[0].revents && ZMQ_POLLERR) {
                std::cout << "error polling zmqSocketIn" << std::endl;
            }
        }
    }
    std::cout << "zmq loop finished" << std::endl;
    zmqSocketOut.close();
    zmqSocketIn.close();
    zmqSocketIntern.close();
    std::cout << "zmq thread finished" << std::endl;
}

int main(int argc, char** argv) {


    isRunning = true;

    // initialize Ctrl-C catching
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);
    sigaction(SIGTERM, &sigIntHandler, NULL);



    int lineCounter = 0;
    int lineTarget = 1000;
    int bufferSize = 1000;
    bool startPrinted = false;
    

    // create ZMQ context
    zmqContext = zmq::context_t(1);
   

    zmq::socket_t zmqSocket(zmqContext, ZMQ_PUB);
    zmqSocket.connect("inproc://loggerInternalConnection");

    std::thread zmqThread(&zmqLoop, &zmqContext);

    std::cout << "main loop started" << std::endl;
    while (std::cin.good() && isRunning.load()) {

        // get current time and create filename
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        std::string filePath = std::string(PATH_LOG) + "log-" + timeToString(timeNow) + ".txt";

        // open file for writing
        std::ofstream os;
        os.open(filePath.c_str(), std::ios::out | std::ios::trunc);
        if (!os.is_open()) {
            std::cout << "File " << filePath << " cannot be opened!" << std::endl;
            break;
        }

        if (!startPrinted) {
            startPrinted = true;
            os << "[" << timeToString(timeNow) << "] " << "Logger started." <<std::endl;
        }

        lineCounter = 0;

        // read input and save it to the file
        while (std::cin.good() && isRunning.load() && lineCounter < lineTarget) {
           std::string line;
            getline(std::cin, line);

            // if the input stream is interrupted
            if (!std::cin.good()) {
                continue;
            }
            os << line << std::endl;
            lineCounter++;

            mutexBuffer.lock();
            // add line to the queue
            buffer.push_front(line);

            // remove old entries from the queue
            while (buffer.size() >= bufferSize) {
                buffer.pop_back();
            }
            mutexBuffer.unlock();

            // send the line to zmqLoop thread
            s_send(zmqSocket, line);
        }
        if (!std::cin.good()) {
            os << "[" << timeToString(timeNow) << "] " << "Logger finished." << std::endl;
        }
        os.close();

    }
    std::cout << "main loop finished" << std::endl;
    
    // send message to the second thread so it can wake up and exit
    s_send(zmqSocket, "Logger is closing.");
    zmqThread.join();
    zmqSocket.close();
    return 0;
}

std::string timeToString(const struct timeval &timestamp, const char * format) {
    time_t t = timestamp.tv_sec;
    char buf[20];
    strftime(buf, 20, format, gmtime(&t));
    return std::string(buf);
}

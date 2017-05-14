

#include "mainLoop.h"
#include "DeviceManager.h"
#define OBJECT_NAME     "Main loop"
#include "Log.h"

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

//#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zmq.hpp>
#include <zhelpers.hpp>
#include <json.hpp>
#include <string>
#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <chrono>
#include <thread>
#include <sstream>
#include <fstream>
#include "PeriodicTimer.h"
#include "Rf24Server.h"

#define THIS_DRIVER_ID          201

volatile int isMainLoopInterrupted;

//using namespace std;
using json = nlohmann::json;

void tcpServer(zmq::context_t &zmqContext, int pipe);

int mainLoop(zmq::context_t &zmqContext) {
    isMainLoopInterrupted = 0;

    // sending to web server
    zmq::socket_t zmqSocketOut(zmqContext, ZMQ_PUB);
    zmqSocketOut.bind("tcp://*:20001");

    // receiving from web server
    zmq::socket_t zmqSocketIn(zmqContext, ZMQ_SUB);
    zmqSocketIn.connect("tcp://localhost:20000");
    zmqSocketIn.setsockopt(ZMQ_SUBSCRIBE, "", 0);
    //  Configure socket to not wait at close time
    //int linger = 0;
    //socket_zqm_in.setsockopt(ZMQ_LINGER, &linger, sizeof (linger));

    // receiving from UDP server
    zmq::socket_t zmqSocketIntern(zmqContext, ZMQ_SUB);
    zmqSocketIntern.bind("inproc://internalConnection"); // bind parent, child connects
    zmqSocketIntern.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    // structure for polling 0MQ sockets
    zmq::pollitem_t zmqPollItems[] = {
        { (void *) zmqSocketIn, 0, ZMQ_POLLIN, 0},
        { (void *) zmqSocketIntern, 0, ZMQ_POLLIN, 0}
    };

    // start UDP server for receiving packets from remote devices
    //    UdpServer udpServer(&zmqContext);
    //    udpServer.start();

    // start NRF24L01+ server
    Rf24Server rfServer(&zmqContext);
    rfServer.start();

    // create device manager and load devices from json files
    DeviceManager deviceManager;

    if (deviceManager.loadDevices() < 0) {
        isMainLoopInterrupted = 1;
    }

    std::cout << "LOADED DEVICES start" << std::endl;
    for (int i = 0; i < deviceManager.getDevicesIds().size(); i++) {
        std::cout << deviceManager.getDeviceById(deviceManager.getDevicesIds()[i])->getDevice().dump(3) << std::endl;
        if (i != deviceManager.getDevicesIds().size() - 1) {
            std::cout << "--------------------------------------------" << std::endl;
        }
    }
    std::cout << "LOADED DEVICES end" << std::endl;
    deviceManager.saveDevices();

    // wait for the 0MQ connection
    std::this_thread::sleep_for(std::chrono::seconds(1));

    // send list of all devices
    json msgJson = {
        {"desId", BROADCAST_ID},
        {"srcId", THIS_DRIVER_ID},
        {"type", "pushAllDevices"}
    };
    msgJson["devices"] = deviceManager.getAllDevices();
    //    LOG_I("\n" + msgJson.dump(3));
    s_send(zmqSocketOut, msgJson.dump());


    PeriodicTimer tim1(&zmqContext, 1, 1000);
    //    PeriodicTimer tim2(&zmqContext,2, 3000);
    //    PeriodicTimer tim3(&zmqContext,3, 10000);
    //tim1.start();
    //    tim2.start();
    //    tim3.start();

    Test test;

    LOG_I("started");
    //********************************************************************
    while (!isMainLoopInterrupted) {

        try {
            // wait for messages
            zmq::poll(&zmqPollItems[0], 2, -1);
        } catch (std::exception &e) {
            LOG_I("Caught exception while polling ZMQ: " + std::string(e.what()));
            isMainLoopInterrupted = 1;
            break;
        }

        //  MESSAGE FROM INTERNAL THREADS
        if (zmqPollItems[1].revents && ZMQ_POLLIN) {

            // read and parse a message
            std::string msgReceivedStr = s_recv(zmqSocketIntern);
            json msgReceivedJson = json::parse(msgReceivedStr);
            //            LOG_I("received internal msg:\n" + msgReceivedJson.dump(3));

            // who send the message
            int srcId = msgReceivedJson["srcId"];
            std::string type = msgReceivedJson["type"];

            // from RF24 server
            if (srcId == RF24SERVER_ID) {

                // prepare message for the web server
                json msgTransmitJson;
                msgTransmitJson["desId"] = BROADCAST_ID;
                msgTransmitJson["srcId"] = msgReceivedJson["desId"];

                // if the web server must be notified about the new state
                if (deviceManager.processMsgFromDevice(msgReceivedJson, msgTransmitJson) > 0) {
                    s_send(zmqSocketOut, msgTransmitJson.dump());
                }
                // from Period Timers
            } else if (srcId == PERIOD_TIMER_ID) {
                if (type.compare("timeout") == 0) {
                    int timerId = msgReceivedJson["desId"];

                    test.generateSample();

                    json msgTransmitJson;
                    msgTransmitJson["srcId"] = THIS_DRIVER_ID;
                    msgTransmitJson["desId"] = BROADCAST_ID;
                    msgTransmitJson["type"] = "test";
                    test.getSamples(msgTransmitJson);
                    s_send(zmqSocketOut, msgTransmitJson.dump());

                    //                    DeviceGeneric * device = deviceManager.getDeviceById(timerId);
                    //
                    //                    if (device != nullptr) {
                    //                        device->timeouted();
                    //                    }
                    //                    LOG_I("Timer " + std::to_string(timerId));
                    //                    if (timerId == 3)
                    //                        tim1.stop();
                }

            }

        } else {
            if (zmqPollItems[1].revents && ZMQ_POLLERR) {
                LOG_E("polling zmqSocketInternal");
            }
        }

        //  MESSAGE FROM WEB SERVER
        if (zmqPollItems[0].revents && ZMQ_POLLIN) {
            std::string msgReceivedStr = s_recv(zmqSocketIn);
            json msgReceivedJson = json::parse(msgReceivedStr);
            LOG_I("received web msg: " + msgReceivedJson.dump());
            int desId = msgReceivedJson["desId"];

            if (desId == THIS_DRIVER_ID) {

            } else if (desId == BROADCAST_ID) {

                json msgTransmitJson;
                msgTransmitJson["srcId"] = THIS_DRIVER_ID;
                msgTransmitJson["desId"] = msgReceivedJson["srcId"];

                std::string type = msgReceivedJson["type"];
                if (type.compare("pullAllDevices") == 0) {
                    msgTransmitJson["type"] = "pushAllDevices";
                    msgTransmitJson["devices"] = deviceManager.getAllDevices();
                    s_send(zmqSocketOut, msgTransmitJson.dump());
                }
            } else {
                // prepare message for the web server
                json msgTransmitJson;
                msgTransmitJson["srcId"] = desId;

                if (deviceManager.processMsgFromGui(msgReceivedJson, msgTransmitJson) == 1)
                    s_send(zmqSocketOut, msgTransmitJson.dump());
            }

        } else {
            if (zmqPollItems[0].revents && ZMQ_POLLERR) {
                LOG_E("polling zmqSocketIn");
            }
        }
    }
    //********************************************************************

    rfServer.stop();
    // clean up
    zmqSocketOut.close();
    zmqSocketIn.close();
    zmqSocketIntern.close();
    LOG_I("finished.");
}

//========================================================================
//========================================================================

void tcpServer(zmq::context_t &zmqContext, int pipeTcp) {
    //    cout << "TCP server started." << endl;
    //
    //    int serverSocket, clientSocket;
    //    struct sockaddr_in serverAddress, clientAddress;
    //
    //    // Open TCP receiving socket
    //    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
    //    if (serverSocket < 0) {
    //        cout << "ERROR opening socket" << endl;
    //        isMainLoopInterrupted = 1;
    //        return;
    //    }
    //    bzero((char *) &serverAddress, sizeof (serverAddress));
    //    serverAddress.sin_family = AF_INET;
    //    serverAddress.sin_addr.s_addr = INADDR_ANY;
    //    serverAddress.sin_port = htons(TCP_SERVER_PORT_NUM);
    //    if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof (serverAddress)) < 0) {
    //        cout << "ERROR on binding" << endl;
    //        isMainLoopInterrupted = 1;
    //        return;
    //    }
    //    if (listen(serverSocket, 10) < 0) {
    //        cout << "ERROR on listen" << endl;
    //        isMainLoopInterrupted = 1;
    //        return;
    //    }
    //
    //    // Connect to 0MQ
    //    zmq::socket_t zmqSocket(zmqContext, ZMQ_PUB);
    //    zmqSocket.connect("inproc://internalConnection");
    //
    //    // prepare polling connections and pipe
    //    struct pollfd fds[2];
    //    int pollStatus;
    //    fds[0].fd = serverSocket;
    //    fds[0].events = POLLIN;
    //    fds[1].fd = pipeTcp;
    //    fds[1].events = POLLIN;
    //
    //    //********************************************************************
    //    while (!isMainLoopInterrupted) {
    //
    //        // wait until something happen
    //        pollStatus = poll(fds, 2, -1);
    //        if (pollStatus == 0)
    //            break;
    //        if (pollStatus < 0) {
    //            cout << "ERROR on poll" << endl;
    //            continue;
    //        }
    //
    //        // new client try to connect
    //        if (fds[0].revents != 0) {
    //            if (fds[0].revents == POLLIN) {
    //                // accept the client
    //                socklen_t addressLength = sizeof (clientAddress);
    //                clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddress, &addressLength);
    //                if (clientSocket < 0) {
    //                    cout << "ERROR on accept" << endl;
    //                    continue;
    //                }
    //
    //                // send new socket to the main thread
    //                json msgTransmitJson = {
    //                    {"srcId", TCP_SERVER_ID},
    //                    {"type", "newSocket"},
    //                    {"socket", clientSocket}
    //                };
    //                s_send(zmqSocket, msgTransmitJson.dump());
    //            } else {
    //                cout << "WARNING event on socket is not POLLIN" << endl;
    //            }
    //        }
    //
    //        // message via pipe from main thred -> interrupt
    //        if (fds[1].revents != 0) {
    //            if (fds[1].revents == POLLIN) {
    //                char msg[10];
    //                int len;
    //                len = read(pipeTcp, msg, 10);
    //                //                msg[len] = 0;
    //                //                isMainLoopInterrupted = 1;
    //                //                cout << msg << endl;
    //            } else {
    //                cout << "WARNING event on pipe is not POLLIN" << endl;
    //            }
    //        }
    //    }
    //    //********************************************************************
    //
    //    // clean up
    //    close(serverSocket);
    //    zmqSocket.close();
    //    cout << "TCP server finished." << endl;
}
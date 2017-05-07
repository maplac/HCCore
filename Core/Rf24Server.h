/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Rf24Server.h
 * Author: Martin
 *
 * Created on 20. dubna 2017, 21:04
 */

#ifndef RF24SERVER_H
#define RF24SERVER_H

#include "Runnable.h"
#include "globalConstants.h"

#include <zmq.hpp>
#include <json.hpp>
#include <vector>

class Rf24Server : public Runnable {
public:
    Rf24Server(zmq::context_t *zmqContext);
    virtual ~Rf24Server();
private:
    zmq::context_t *zmqContext;
    char packet[32];
    int packetLength;

protected:
    virtual void run();

};

#endif /* RF24SERVER_H */


/* 
 * File:   mainLoop.h
 * Author: Martin
 *
 * Created on 18. srpna 2016, 18:18
 */

#ifndef MAINLOOP_H
#define	MAINLOOP_H

#include <zmq.hpp>

#include "Test.h"

extern volatile int isMainLoopInterrupted;

int mainLoop(zmq::context_t &zmqContext);

#endif	/* MAINLOOP_H */


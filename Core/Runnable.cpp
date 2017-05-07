/* 
 * File:   Runnable.cpp
 * Author: Martin
 * 
 * Created on 28. srpna 2016, 18:07
 */

#include "Runnable.h"
#include <signal.h>
#include <unistd.h>
#include <string.h>

Runnable::Runnable() {
    isRunning = false;
    isStarted = false;
    threadPipe[0] = -1;
    threadPipe[1] = -1;
}

Runnable::~Runnable() {
}

bool Runnable::isThreadRunning() {
    return isRunning.load();
}

void Runnable::start() {

    if (isStarted)
        return;
    pipe(threadPipe);
    thread = std::thread(&Runnable::run, this);
    isRunning = true;
    isStarted = true;
}

void Runnable::stop() {
    if (!isStarted)
        return;

    isStarted = false;
    isRunning = false;
    const char *msg = "quit";
    write(threadPipe[1], msg, strlen(msg));

    if (thread.joinable())
        thread.join();

    close(threadPipe[0]);
    close(threadPipe[1]);
    threadPipe[0] = -1;
    threadPipe[1] = -1;
}

int Runnable::getReadingPipe() {
    return threadPipe[0];
}

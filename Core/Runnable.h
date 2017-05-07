/* 
 * File:   Runnable.h
 * Author: Martin
 *
 * Created on 28. srpna 2016, 18:07
 */

#ifndef RUNNABLE_H
#define	RUNNABLE_H

#include <thread>
#include <atomic>

class Runnable {
public:
    Runnable();
    virtual ~Runnable();
    virtual void start();
    virtual void stop();
    bool isThreadRunning();

private:
    int threadPipe[2];
    std::atomic<bool> isStarted;
    std::thread thread;

protected:
    std::atomic<bool> isRunning;
    virtual void run() = 0;
    int getReadingPipe();

};

#endif	/* RUNNABLE_H */


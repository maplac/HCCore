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

#define PATH_LOG    "/home/pi/HomeControl-Log/"

using namespace std;

std::string timeToString(const struct timeval &timestamp, const char * format = "%Y-%m-%d %H:%M:%S");

int main(int argc, char** argv) {

    int lineCounter = 0;
    int lineTarget = 1000;
    int bufferSize = 1000;
    deque<string> buffer;

    while (cin.good()) {

        // get current time and create filename
        struct timeval timeNow;
        gettimeofday(&timeNow, NULL);
        string filePath = string(PATH_LOG) + "log-" + timeToString(timeNow) + ".txt";

        // open file for writing
        ofstream os;
        os.open(filePath.c_str(), ios::out | ios::trunc);
        if (!os.is_open()) {
            cout << "File " << filePath << " cannot be opened!" << endl;
            break;
        }

        lineCounter = 0;

        // read input and save it to the file
        while (cin.good() && lineCounter < lineTarget) {
            string line;
            getline(cin, line);
            os << line << endl;
            lineCounter++;
            
            // add line to the queue
            buffer.push_front(line);
            
            // remove old entries from the queue
            while(buffer.size() >= bufferSize){
                buffer.pop_back();
            }
        }
        os.close();

    }

    return 0;
}

std::string timeToString(const struct timeval &timestamp, const char * format) {
    time_t t = timestamp.tv_sec;
    char buf[20];
    strftime(buf, 20, format, gmtime(&t));
    return std::string(buf);
}

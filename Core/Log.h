/* 
 * File:   Log.h
 * Author: Martin
 *
 * Created on 24. září 2016, 19:21
 */

#ifndef OBJECT_NAME
#define	OBJECT_NAME     "undefined"
#endif

#define LOG_I(x) Log::info(OBJECT_NAME, x)
#define LOG_W(x) Log::warn(OBJECT_NAME, x)
#define LOG_E(x) Log::error(OBJECT_NAME, x)

#ifndef LOG_H
#define	LOG_H

#include <string>

class Log {
public:

    virtual ~Log() { }
    static void info(std::string name, std::string msg);
    static void warn(std::string name, std::string msg);
    static void error(std::string name, std::string msg);

private:

    Log() { }
    static std::string getCurrentTime();
};

#endif	/* LOG_H */


/* 
 * File:   PacketManager.h
 * Author: Martin
 *
 * Created on 11. září 2016, 21:14
 */

#ifndef PACKETMANAGER_H
#define	PACKETMANAGER_H

#define PACKET_MAX_SIZE                 100
#define PACKET_MIN_SIZE                 9

#define PACKET_INDEX_START1             0
#define PACKET_INDEX_START2             1
#define PACKET_INDEX_ID                 2
#define PACKET_INDEX_COUNTER            3
#define PACKET_INDEX_TYPE               4
#define PACKET_INDEX_START_ADDRESS      5
#define PACKET_INDEX_LENGTH             6
#define PACKET_INDEX_START_DATA         7

#define PACKET_INDEX_INIT_ID            7
#define PACKET_INDEX_INIT_TYPE          8
#define PACKET_INDEX_INIT_STATUS        9

#define PACKET_TYPE_SET           1
#define PACKET_TYPE_GET           2
#define PACKET_TYPE_REPLY         3
#define PACKET_TYPE_INIT          4
#define PACKET_TYPE_ERROR         5

// common
#define ADDRESS_POSITION_ID              0
#define ADDRESS_POSITION_TYPE            1
#define ADDRESS_POSITION_VERSION         2
#define ADDRESS_POSITION_STATUS          3
#define ADDRESS_POSITION_ERROR_H         4
#define ADDRESS_POSITION_ERROR_L         5
//#define ADDRESS_POSITION_COMMAND         31

#define ADDRESS_VALUE_TYPE_SWITCH_ONOFF  1
#define ADDRESS_VALUE_TYPE_DHT22         2
#define ADDRESS_VALUE_TYPE_SWITCH_VALUE  3

// Switch OnOff
//#define ADDRESS_POSITION_VALUE           32
//
//// DHT22
//
//// Switch Value
//#define ADDRESS_POSITION_VALUE           32
//#define ADDRESS_POSITION_MAX_VALUE       33

// 255, 255, id, type, addr, length, value, cs

#include <vector>
#include <json.hpp>

class PacketManager {
public:
    virtual ~PacketManager();
    static std::vector<int> jsonToVector(const nlohmann::json &message);
    static std::vector<int> stringToVector(const std::string &packet);
    static std::string vectorToString(const std::vector<int> &packet);
    static int isPacketCorrect(const std::vector<int> &packet);
    static std::vector<int> createPacket(int id, int type, int length);
    
private:
    PacketManager();
};

#endif	/* PACKETMANAGER_H */


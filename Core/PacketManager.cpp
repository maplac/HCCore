/* 
 * File:   PacketManager.cpp
 * Author: Martin
 * 
 * Created on 11. září 2016, 21:14
 */

#include "PacketManager.h"
#define OBJECT_NAME     "Packet manager"
#include "Log.h"

#include <iostream>
#include <sstream>
#include <string>
#include <exception>

PacketManager::PacketManager() {
}

PacketManager::~PacketManager() {
}

std::vector<int> PacketManager::jsonToVector(const nlohmann::json &message) {
    std::vector<int> packet;
    if (message.find("packet") == message.end()) {
        LOG_E("message doesn't contain entry \"packet\".");
        return packet;
    }
    nlohmann::json packetJson = message["packet"];

    for (nlohmann::json::iterator it = packetJson.begin(); it != packetJson.end(); ++it) {
        packet.push_back(*it);
    }
    return packet;
}

int PacketManager::isPacketCorrect(const std::vector<int>& packet) {
    if (packet.size() < PACKET_MIN_SIZE || packet.size() > PACKET_MAX_SIZE) {
        LOG_E("Received packet is not correct and will be ignored (length)");
        return -1;
    }
    if (packet[PACKET_INDEX_START1] != 255 && packet[PACKET_INDEX_START2] != 255) {
        LOG_E("Received packet is not correct and will be ignored (head)");
        return -1;
    }
    if (packet[PACKET_INDEX_LENGTH] != packet.size() - 8) {
        LOG_E("Received packet is not correct and will be ignored (length mismatch)");
        return -1;
    }

    return packet[PACKET_INDEX_ID];
}

std::vector<int> PacketManager::stringToVector(const std::string& packet) {
    std::vector<int> vec;
    std::stringstream ss;
    ss.str(packet);
    std::string item;
    while (std::getline(ss, item, ' ')) {
        int value = 0;
        try {
            value = std::stoi(item);
        } catch (std::exception &e) {
            LOG_E("Converting \"" + item + "\" to a number failed.");
        }
        vec.push_back(value);
    }
    return vec;
}

std::string PacketManager::vectorToString(const std::vector<int>& packet) {
    std::stringstream ss;
    for (int i = 0; i < packet.size(); i++) {
        ss << packet[i];
        if (i < packet.size() - 1)
            ss << " ";
    }
    return ss.str();
}

std::vector<int> PacketManager::createPacket(int id, int type, int length) {
    std::vector<int> packet(8 + length);
    packet[PACKET_INDEX_START1] = 255;
    packet[PACKET_INDEX_START2] = 255;
    packet[PACKET_INDEX_ID] = id;
    packet[PACKET_INDEX_TYPE] = type;
    packet[PACKET_INDEX_LENGTH] = length;
    return packet;
}

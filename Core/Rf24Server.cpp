/* 
 * File:   Rf24Server.cpp
 * Author: Martin
 * 
 * Created on 20. dubna 2017, 21:04
 */

#include "Rf24Server.h"
#define OBJECT_NAME     "RF24 server"
#include "Log.h"
#include "globalConstants.h"

#include <zhelpers.hpp>
#include <string>
#include <sys/poll.h>

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
extern "C" {
#include "rf24.h"
#include "gpio.h"
#include "spi.h"
#include "nRF24L01.h"
}

using json = nlohmann::json;

Rf24Server::Rf24Server(zmq::context_t *zmqContext) : zmqContext(zmqContext) {
}

Rf24Server::~Rf24Server() {
    if (isRunning)
        stop();
}

void Rf24Server::run() {
    // Connect to 0MQ
    zmq::socket_t zmqSocket(*zmqContext, ZMQ_PUB);
    zmqSocket.connect("inproc://internalConnection");


    rf24_t radio;


    uint64_t pipe_addresses[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
    uint8_t pipe_no = 0;

    rf24_initialize(&radio, RF24_SPI_DEV_0, 16, 21);
    // printf("res=%d", res);
    rf24_set_retries(&radio, 15, 15);

    rf24_open_writing_pipe(&radio, pipe_addresses[1]);
    rf24_open_reading_pipe(&radio, 0, pipe_addresses[0]);
    rf24_open_reading_pipe(&radio, 1, pipe_addresses[1]);

    rf24_dump(&radio);

    char buf[32];
    for (int i = 0; i < 32; i++) {
        buf[i] = 65 + i;
    }

    //rf24_send(&radio, &buf, 32 * sizeof(uint8_t));

    rf24_start_listening(&radio);
    //sleep(1);
    //send_pong(&radio);
    rf24_mask_tx_ds(&radio);
    //rf24_unmask_tx_ds(&radio);
    //    rf24_irq_poll(&radio, &send_pong);

    uint32_t gpioFd = gpio_get_file_descriptor(radio.irq_pin, GPIO_EDGE_FALLING);

    if (gpioFd <= 0) {
        LOG_E("gpio_get_file_descriptor failed");
        isRunning = false;
    }
    //    lseek(gpioFd, 0, SEEK_SET);

    // prepare polling socket reading and pipe reading
    struct pollfd fds[2];
    int pollStatus;
    fds[0].fd = gpioFd;
    fds[0].events = POLLPRI;
    fds[1].fd = getReadingPipe();
    fds[1].events = POLLIN;

    int counter = 0;

    LOG_I("started");
    while (isRunning) {

        // wait until something happens
        pollStatus = poll(fds, 2, -1);
        if (pollStatus < 0) {
            LOG_W("on poll: " + std::string(strerror(errno)));
            continue;
        }

        // packet from remote device
        if (fds[0].revents != 0) {
            uint8_t val;
            read(gpioFd, &val, 1);
            lseek(gpioFd, 0, SEEK_SET);
            char buffer[32];
            uint8_t i, len, done;

            rf24_sync_status(&radio);

            /*
            fprintf(stderr, "[rf24 pong callback] Got IRQ on pin %d. TX ok: %d, TX fail: %d RX ready: %d RX_LEN: %d PIPE: %d COUNTER: %d\n",
                    radio.irq_pin,
                    radio.status.tx_ok,
                    radio.status.tx_fail_retries,
                    radio.status.rx_data_available,
                    radio.status.rx_data_len,
                    radio.status.rx_data_pipe,
                    counter
                    );
            */
            if (radio.status.rx_data_available) {
                len = radio.status.rx_data_len;

                rf24_receive(&radio, &buffer, len);
                std::vector<char> packet(buffer,buffer+32);
                usleep(20);
                rf24_reset_status(&radio);


                //rf24_stop_listening(&radio);
                //rf24_send(&radio, &buf, 32 * sizeof (uint8_t));
                //rf24_start_listening(&radio);

                
                json msgTransmitJson = {
                    {"srcId", RF24SERVER_ID},
                    {"type", "dataReceived"},
                    {"desId", buffer[0]},
                    {"pipeIndex", radio.status.rx_data_pipe},
                    {"data", json(packet)},
                };
//                LOG_I(msgTransmitJson.dump(3));
                s_send(zmqSocket, msgTransmitJson.dump());
            } else {
                rf24_reset_status(&radio);
            }


            //
            //                uint8_t val;
            //                read(gpioFd, &val, 1);
            //                lseek(gpioFd, 0, SEEK_SET);
            //
            //                LOG_I("interrupt");

            //                packetLength = recvfrom(serverSocket, packet, PACKET_MAX_SIZE, 0,
            //                        (struct sockaddr*) &clientAddress, &addressLen);
            //                if (packetLength < 0) {
            //                    LOG_W("on recvfrom: " + std::string(strerror(errno)));
            //                    continue;
            //                }
            //                packet[packetLength] = 0;
            //                LOG_I("received packet: " + std::string(packet));
            //
            //                std::string packetStr = std::string(packet);
            //                std::vector<int> packetVector = PacketManager::stringToVector(packetStr);
            //                int id = PacketManager::isPacketCorrect(packetVector);
            //                if (id >= 0) {
            //
            //                    // send packet to the main thread
            //                    json msgTransmitJson = {
            //                        {"srcId", UDP_SERVER_ID},
            //                        {"type", "packet"},
            //                        {"desId", id},
            //                        {"packet", json(packetVector)},
            //                        {"ip", std::string(inet_ntoa(clientAddress.sin_addr))},
            //                        {"port", ntohs(clientAddress.sin_port)},
            //                        {"addrLen", (unsigned int) addressLen}
            //                    };
            //                    s_send(zmqSocket, msgTransmitJson.dump());
            //                }
        }

        // message via pipe from main thread -> interrupt
        if (fds[1].revents != 0) {
            if (fds[1].revents == POLLIN) {
                char data[10];
                int len = read(getReadingPipe(), data, 10);
                isRunning = false;
                break;
            } else {
                LOG_W("event on pipe is not POLLIN");
            }
        }
    }
    zmqSocket.close();
    close(gpioFd);
    LOG_I("finished");
}
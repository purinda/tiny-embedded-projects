#ifndef LIFX_H
#define LIFX_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    /* frame */
    uint16_t size;
    uint16_t protocol : 12;
    uint8_t  addressable : 1;
    uint8_t  tagged : 1;
    uint8_t  origin : 2;
    uint32_t source;

    /* frame address */
    uint8_t target[8];
    uint8_t reserved[6];
    uint8_t res_required : 1;
    uint8_t ack_required : 1;
    uint8_t : 6;
    uint8_t sequence;

    /* protocol header */
    uint64_t : 64;
    uint16_t type;
    uint16_t : 16;
    /* variable length payload follows */
} lx_protocol_header_t;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct {
    /* set power */
    uint16_t level;
    uint32_t duration;
} lx_set_power_t;
#pragma pack(pop)

class Lifx {
   private:
    static const uint16_t LX_PORT        = 56700;
    static const uint8_t  MAX_LX_DEVICES = 16;
    static const uint8_t  SIZE_OF_MAC    = 6;

    uint8_t   numLxDevices;
    byte      lxDevices[MAX_LX_DEVICES][SIZE_OF_MAC];
    IPAddress lxDevicesAddr[MAX_LX_DEVICES];
    char      packetBuffer[128];
    WiFiUDP*  UDP;

   public:
    IPAddress* bcastAddr;

    Lifx();
    ~Lifx();

    void buildFrame(lx_protocol_header_t* lxHead, uint8_t extraSize, uint8_t tagged, uint8_t* target, uint16_t message);
    void discover();
    void setPower(uint16_t level);
    void setBulb(uint8_t index, byte *mac, IPAddress ip);
};

#endif

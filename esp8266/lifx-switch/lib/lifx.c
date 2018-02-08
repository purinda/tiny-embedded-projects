#include "lifx.h"

void lx_build_frame(lx_protocol_header_t* lxHead,
    uint8_t                               extraSize,
    uint8_t                               tagged,
    uint8_t*                              target,
    uint16_t                              message) {
    /* frame */
    lxHead->size        = (uint8_t) 36 + extraSize;
    lxHead->protocol    = (uint16_t) 1024;
    lxHead->addressable = (uint8_t) 1;
    lxHead->tagged      = tagged;
    lxHead->origin      = (uint8_t) 0;
    lxHead->source      = (uint32_t) 3549;

    /* frame address */
    uint8_t i = 0;
    for (i = 0; i < 8; i++) {
        lxHead->target[i] = (uint8_t) target[i];
    }
    lxHead->res_required = (uint8_t) 0;
    lxHead->ack_required = (uint8_t) 0;
    lxHead->sequence     = (uint8_t) 0;

    /* protocol header */
    lxHead->type = message;
}


void lx_discover() {
    /* Build lxDiscovery payload */
    byte                  target_addr[8] = {0};
    lx_protocol_header_t* lxHead;
    lxHead = (lx_protocol_header_t*) calloc(1, sizeof(lx_protocol_header_t));
    lxMakeFrame(lxHead, 0, 1, target_addr, 2);

    /* Start listening for responses */
    UDP.begin(4097);
    delay(500);

    /* Send a couple of discovery packets out to the network*/
    for (int i = 0; i < 1; i++) {
        byte* b = (byte*) lxHead;
        UDP.beginPacket(bcastAddr, lxPort);
        UDP.write(b, sizeof(lx_protocol_header_t));
        UDP.endPacket();

        delay(500);
    }

    free(lxHead);

    for (int j = 0; j < 20; j++) {
        int sizePacket = UDP.parsePacket();
        if (sizePacket) {
            UDP.read(packetBuffer, sizePacket);
            byte target_addr[SIZE_OF_MAC];
            memcpy(target_addr, packetBuffer + 8, SIZE_OF_MAC);

            // Print MAC
            for (int i = 1; i <= sizeof(target_addr); i++) {
                Serial.print(target_addr[i - 1], HEX);
            }
            Serial.println();

            // Check if this device is new
            uint8_t previouslyKnownDevice = 0;
            for (int i = 0; i < LX_DEVICES; i++) {
                if (!memcmp(lxDevices[i], target_addr, SIZE_OF_MAC)) {
                    Serial.println("Previously seen target");
                    previouslyKnownDevice = 1;
                    break;
                }
            }

            // Store new devices
            if (!previouslyKnownDevice) {
                lxDevicesAddr[LX_DEVICES] = (uint32_t) UDP.remoteIP();

                Serial.println(UDP.remoteIP());
                memcpy(lxDevices[LX_DEVICES++], target_addr, SIZE_OF_MAC);
                Serial.print("Storing device as LX_DEVICE ");
                Serial.println(LX_DEVICES);
            }
        }
        delay(20);
    }
}


void lx_power(uint16_t level) {
    static bool lock;

    // Mutex
    if (lock == 1) {
        return;
    } else {
        lock = 1;
    }

    for (int i = 0; i < LX_DEVICES; i++) {
        /* Set target_addr from know lxDevices */
        byte target_addr[8] = {0};
        memcpy(target_addr, lxDevices[i], SIZE_OF_MAC);

        /* Build payload */
        uint8_t  type     = 117;
        uint32_t duration = 500;

        lx_protocol_header_t* lxHead;
        lxHead = (lx_protocol_header_t*) calloc(1, sizeof(lx_protocol_header_t));
        lxMakeFrame(lxHead, sizeof(lx_set_power_t), 0, target_addr, type);

        lx_set_power_t* lxSetPower;
        lxSetPower           = (lx_set_power_t*) calloc(1, sizeof(lx_set_power_t));
        lxSetPower->duration = 500;
        lxSetPower->level    = level;

        UDP.beginPacket(lxDevicesAddr[i], lxPort);
        byte* b = (byte*) lxHead;
        UDP.write(b, sizeof(lx_protocol_header_t));
        b = (byte*) lxSetPower;
        UDP.write(b, sizeof(lx_set_power_t));
        UDP.endPacket();

        free(lxSetPower);
        free(lxHead);

        Serial.print("Sending lxPower packet to ");
        Serial.println(lxDevicesAddr[i].toString());
    }

    Serial.println();
    lock = 0;
}
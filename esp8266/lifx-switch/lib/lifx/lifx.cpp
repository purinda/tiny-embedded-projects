#include <lifx.h>

Lifx::Lifx() {
    numLxDevices = 1;

    static uint8_t zeros[Lifx::SIZE_OF_MAC] = {0};
    for (int i = 0; i < MAX_LX_DEVICES; i++) {
        memcpy(this->lxDevices[i], zeros, SIZE_OF_MAC);
    }

    this->UDP       = new WiFiUDP();
    this->bcastAddr = new IPAddress(255, 255, 255, 255);
}

void Lifx::buildFrame(lx_protocol_header_t* lxHead,
    uint8_t                                 extraSize,
    uint8_t                                 tagged,
    uint8_t*                                target,
    uint16_t                                message) {
    /* Frame */
    lxHead->size        = (uint8_t) 36 + extraSize;
    lxHead->protocol    = (uint16_t) 1024;
    lxHead->addressable = (uint8_t) 1;
    lxHead->tagged      = tagged;
    lxHead->origin      = (uint8_t) 0;
    lxHead->source      = (uint32_t) 3549;

    /* Frame address */
    uint8_t i = 0;
    for (i = 0; i < 8; i++) {
        lxHead->target[i] = (uint8_t) target[i];
    }
    lxHead->res_required = (uint8_t) 0;
    lxHead->ack_required = (uint8_t) 0;
    lxHead->sequence     = (uint8_t) 0;

    /* Protocol header */
    lxHead->type = message;
}

void Lifx::discover() {
    /* Build lxDiscovery payload */
    byte                  target_addr[8] = {0};
    lx_protocol_header_t* lxHead;
    lxHead = (lx_protocol_header_t*) calloc(1, sizeof(lx_protocol_header_t));
    buildFrame(lxHead, 0, 1, target_addr, 2);

    /* Start listening for responses */
    this->UDP->begin(4097);
    delay(500);

    /* Send a couple of discovery packets out to the network*/
    for (int i = 0; i < 1; i++) {
        byte* b = (byte*) lxHead;
        this->UDP->beginPacket(*this->bcastAddr, this->LX_PORT);
        this->UDP->write(b, sizeof(lx_protocol_header_t));
        this->UDP->endPacket();

        delay(500);
    }

    free(lxHead);

    for (int j = 0; j < MAX_LX_DEVICES; j++) {
        int sizePacket = this->UDP->parsePacket();

        if (sizePacket) {
            this->UDP->read(packetBuffer, sizePacket);
            byte target_addr[SIZE_OF_MAC];
            memcpy(target_addr, packetBuffer + 8, SIZE_OF_MAC);

            // Print MAC
            for (int i = 1; i <= sizeof(target_addr); i++) {
                Serial.print(target_addr[i - 1], HEX);
            }
            Serial.println();

            // Check if this device is new
            uint8_t previouslyKnownDevice = 0;
            for (int i = 0; i < numLxDevices; i++) {
                if (!memcmp(this->lxDevices[i], target_addr, SIZE_OF_MAC)) {
                    Serial.println("Previously seen target");
                    previouslyKnownDevice = 1;
                    break;
                }
            }

            // Store new devices
            if (!previouslyKnownDevice) {
                lxDevicesAddr[numLxDevices] = (uint32_t) this->UDP->remoteIP();

                Serial.println(this->UDP->remoteIP());
                memcpy(lxDevices[numLxDevices++], target_addr, SIZE_OF_MAC);
                Serial.print("Storing device as Lifx:");
                Serial.println(numLxDevices);
            }
        }

        delay(50);
    }
}

void Lifx::setPower(uint16_t level) {
    static bool lock;

    // Mutex
    if (lock == 1) {
        return;
    } else {
        lock = 1;
    }

    for (int i = 0; i < numLxDevices; i++) {
        /* Set target_addr from know lxDevices */
        byte target_addr[8] = {0};
        memcpy(target_addr, lxDevices[i], SIZE_OF_MAC);

        /* Build payload */
        uint8_t  type     = 117;
        uint32_t duration = 500;

        lx_protocol_header_t* lxHead;
        lxHead = (lx_protocol_header_t*) calloc(1, sizeof(lx_protocol_header_t));
        buildFrame(lxHead, sizeof(lx_set_power_t), 0, target_addr, type);

        lx_set_power_t* lxSetPower;
        lxSetPower           = (lx_set_power_t*) calloc(1, sizeof(lx_set_power_t));
        lxSetPower->duration = 500;
        lxSetPower->level    = level;

        this->UDP->beginPacket(lxDevicesAddr[i], LX_PORT);
        byte* b = (byte*) lxHead;
        this->UDP->write(b, sizeof(lx_protocol_header_t));
        b = (byte*) lxSetPower;
        this->UDP->write(b, sizeof(lx_set_power_t));
        this->UDP->endPacket();

        free(lxSetPower);
        free(lxHead);

        Serial.print("Sending power change packet to ");
        Serial.println(lxDevicesAddr[i].toString());
    }

    lock = 0;
}
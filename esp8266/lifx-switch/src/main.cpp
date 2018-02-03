#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "../../config/global_config.h"

const uint8_t BOUNCE_DURATION = 20;
const uint8_t MAX_LX_DEVICES = 16;
uint8_t LX_DEVICES = 1;

const uint8_t SIZE_OF_MAC = 6;
volatile unsigned long bounceTime = 0;

byte lxDevices[MAX_LX_DEVICES][SIZE_OF_MAC];
IPAddress lxDevicesAddr[MAX_LX_DEVICES];

IPAddress bcastAddr(255, 255, 255, 255);
int lxPort = 56700;

WiFiUDP UDP;

uint8_t buttonToggled = 0;
uint8_t buttonTimeout = 0;
uint8_t interuptPin = 0;

#pragma pack(push, 1)
typedef struct {
    /* frame */
    uint16_t size;
    uint16_t protocol : 12;
    uint8_t addressable : 1;
    uint8_t tagged : 1;
    uint8_t origin : 2;
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

byte packetBuffer[128];

void lxMakeFrame(lx_protocol_header_t *lxHead, uint8_t extraSize,
    uint8_t tagged, uint8_t *target, uint16_t message) {

    /* frame */
    lxHead->size = (uint8_t)36 + extraSize;
    lxHead->protocol = (uint16_t)1024;
    lxHead->addressable = (uint8_t)1;
    lxHead->tagged = tagged;
    lxHead->origin = (uint8_t)0;
    lxHead->source = (uint32_t)3549;

    /* frame address */
    uint8_t i = 0;
    for (i = 0; i < 8; i++) {
        lxHead->target[i] = (uint8_t)target[i];
    }
    lxHead->res_required = (uint8_t)0;
    lxHead->ack_required = (uint8_t)0;
    lxHead->sequence = (uint8_t)0;

    /* protocol header */
    lxHead->type = message;
}

void lxDiscovery() {
    /* Build lxDiscovery payload */
    byte target_addr[8] = {0};
    lx_protocol_header_t *lxHead;
    lxHead = (lx_protocol_header_t *)calloc(1, sizeof(lx_protocol_header_t));
    lxMakeFrame(lxHead, 0, 1, target_addr, 2);

    /* Start listening for responses */
    UDP.begin(4097);
    delay(500);

    /* Send a couple of discovery packets out to the network*/
    for (int i = 0; i < 1; i++) {
        byte *b = (byte *)lxHead;
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
                lxDevicesAddr[LX_DEVICES] = (uint32_t)UDP.remoteIP();

                Serial.println(UDP.remoteIP());
                memcpy(lxDevices[LX_DEVICES++], target_addr, SIZE_OF_MAC);
                Serial.print("Storing device as LX_DEVICE ");
                Serial.println(LX_DEVICES);
            }
        }
        delay(20);
    }
}

void lxPower(uint16_t level) {
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
        uint8_t type = 117;
        uint32_t duration = 500;

        lx_protocol_header_t *lxHead;
        lxHead = (lx_protocol_header_t *)calloc(1, sizeof(lx_protocol_header_t));
        lxMakeFrame(lxHead, sizeof(lx_set_power_t), 0, target_addr, type);

        lx_set_power_t *lxSetPower;
        lxSetPower = (lx_set_power_t *)calloc(1, sizeof(lx_set_power_t));
        lxSetPower->duration = 500;
        lxSetPower->level = level;

        UDP.beginPacket(lxDevicesAddr[i], lxPort);
        byte *b = (byte *)lxHead;
        UDP.write(b, sizeof(lx_protocol_header_t));
        b = (byte *)lxSetPower;
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

void isr_p0() {
    // it ignores presses that occur in intervals less then the bounce time
    if(millis() > bounceTime) {
        // Your code here to handle new button press ?
        bounceTime = millis() + BOUNCE_DURATION;  // set whatever bounce time in ms is appropriate
    } else {
        return;
    }

    buttonToggled = !buttonToggled;

    if (buttonToggled == 0) {
        lxPower(0);
    } else {
        lxPower(65534);
    }

    /* Button watching */
    char output[8];
    sprintf(output, "Toggled %d", buttonToggled);
    Serial.println(output);
    Serial.println("");
}

void isr_timeout() { buttonTimeout = 1; }

void setup() {

    uint8_t zeros[SIZE_OF_MAC] = {0};
    for (int i = 0; i < MAX_LX_DEVICES; i++) {
        memcpy(lxDevices[i], zeros, SIZE_OF_MAC);
    }

    // Setup LIFX Master Bedroom
    lxDevices[0][0] = 0xD0;
    lxDevices[0][1] = 0x73;
    lxDevices[0][2] = 0xD5;
    lxDevices[0][3] = 0x26;
    lxDevices[0][4] = 0xB8;
    lxDevices[0][5] = 0x4D;
    lxDevicesAddr[0][0] = 10;
    lxDevicesAddr[0][1] = 10;
    lxDevicesAddr[0][2] = 0;
    lxDevicesAddr[0][3] = 51;

    Serial.begin(115200);
    delay(10);

    // We start by connecting to a WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    bcastAddr = WiFi.localIP();
    bcastAddr[3] = 255;

    /* Setup Interupts */
    pinMode(0, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(0), isr_p0, CHANGE);
}

void loop() {

}
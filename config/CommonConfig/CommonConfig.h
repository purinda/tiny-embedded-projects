#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H

class CommonConfig {
   public:
    static char* ssid;
    static char* password;
    static char* nodeName;
    static int   serialBaud;
    static bool  powerSaving;
};


#endif
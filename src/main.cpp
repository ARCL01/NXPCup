#include "mbed.h"
#include "pinmap.h"

UnbufferedSerial pc(USBTX, USBRX);
I2C i2c(PTE25, PTE24);

void print_serial(const char* message) {
    pc.write(message, strlen(message));
}

bool get_pixy_version() {
    char request[] = {
        0xae,
        0xc1,  
        14,
        0
    };

    int result = i2c.write(0x54<<1, request, 4);
    
    if (result != 0) {
        print_serial("Chyba pri zapisu I2C\r\n");
        return false;
    }
    
    char response[22];
    
    result = i2c.read(0x54<<1, response, 22);
    
    if (result != 0) {
        print_serial("I2C write error!\n");
        return false;
    }

    if(response[0] != 0xaf || response[1] != 0xc1 || response[2] != 15 || response[3] != 16) {
        print_serial("Invalid response Pixy2!\n");
        return false;
    }

    short hw_version = response[6] | (response[7] << 8);

    return true;
}

int main() {
    while(true) {
        get_pixy_version();
    }
}
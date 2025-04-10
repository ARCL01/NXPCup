#include "mbed.h"
#include "pinmap.h"

// Struktura pro uložení všech verzních informací z Pixy2
struct Pixy2Version {
    uint16_t hw_version;       // Hardware verze
    uint8_t fw_version_major;  // Firmware verze (hlavní)
    uint8_t fw_version_minor;  // Firmware verze (vedlejší)
    uint16_t fw_build;         // Firmware build číslo
    char fw_type[11];          // Firmware typ (lidsky čitelný řetězec + null znak)
};

UnbufferedSerial pc(USBTX, USBRX);
I2C i2c(PTE25, PTE24);

void print_serial(const char* message) {
    pc.write(message, strlen(message));
}

bool get_pixy_version(Pixy2Version &version) {
    char request[] = { 0xae, 0xc1, 14, 0 };
    int result = i2c.write(0x54<<1, request, 4);
    if (result != 0) {
        print_serial("Chyba pri zapisu I2C\r\n");
        return false;
    }
    
    char response[22];
    result = i2c.read(0x54<<1, response, 22);
    if (result != 0) {
        print_serial("I2C read error!\r\n");
        return false;
    }
    
    if(response[0] != 0xaf || response[1] != 0xc1 || response[2] != 15 || response[3] != 16) {
        print_serial("Invalid response Pixy2!\r\n");
        return false;
    }
    
    // Validace checksumu
    uint16_t checksum = (uint16_t)(response[4]) | ((uint16_t)(response[5]) << 8);
    uint16_t payload_sum = 0;
    for (int i = 6; i < 22; i++) {
        payload_sum += (uint8_t)response[i];
    }
    
    if (checksum != payload_sum) {
        print_serial("Checksum mismatch!\r\n");
        return false;
    }
    
    // Extrakce všech dat z odpovědi do struktury
    version.hw_version = (uint16_t)(response[6]) | ((uint16_t)(response[7]) << 8);
    version.fw_version_major = response[8];
    version.fw_version_minor = response[9];
    version.fw_build = (uint16_t)(response[10]) | ((uint16_t)(response[11]) << 8);
    
    // Kopírování firmware typu (pozice 12-21)
    memcpy(version.fw_type, &response[12], 10);
    version.fw_type[10] = '\0'; // Zajištění ukončení řetězce null znakem
    
    return true;
}

int main() {
    while(true) {
        Pixy2Version version;
        if (get_pixy_version(version)) {
            char buffer[128];
            sprintf(buffer, "Hardware Version: %d\r\nFirmware Version: %d.%d\r\nFirmware Build: %d\r\nFirmware Type: %s\r\n\r\n", 
                    version.hw_version, 
                    version.fw_version_major, 
                    version.fw_version_minor, 
                    version.fw_build, 
                    version.fw_type);
            print_serial(buffer);
        }
        
        wait_us(1000000); // Počkejte 1 sekundu před další kontrolou
    }
}
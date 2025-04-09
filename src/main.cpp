#include "mbed.h"

#define WAIT_TIME_MS 500
const int PIXY2_I2C_ADDRESS = 0x84;
const uint8_t PIXY_TYPE_REQUEST_VERSION = 14;
const uint8_t PIXY_TYPE_RESPONSE_VERSION = 15;

DigitalOut led1(LED_BLUE);
DigitalOut led2(LED_GREEN);
DigitalOut led3(LED_RED);

// Opravený pořadí pinů - PTE24 = SDA, PTE25 = SCL
I2C i2c(PTE25, PTE24);
UnbufferedSerial serial_port(USBTX, USBRX, 9600);

void print_serial(const char* message);
bool get_pixy_version();


int main() {
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; // Povolení hodin pro PORT E
    
    // Konfigurace PTE24 (SCL) jako I2C
    PORTE->PCR[24] = PORT_PCR_MUX(5); // ALT5 = I2C0_SCL
    
    // Konfigurace PTE25 (SDA) jako I2C
    PORTE->PCR[25] = PORT_PCR_MUX(5); // ALT5 = I2C0_SDA
    
    // Nastavení pull-up rezistorů pro I2C piny
    PORTE->PCR[24] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK; // Pull-up pro SCL
    PORTE->PCR[25] |= PORT_PCR_PE_MASK | PORT_PCR_PS_MASK;
    print_serial("I2C Scanner\r\n");
    
    int count = 0;
    
    // Standardní I2C adresy jsou 7-bitové, takže projdeme 0-127
    for (int address = 0; address < 128; address++) {
        // I2C používá 8-bitovou adresu (7-bitová adresa + R/W bit)
        // Write operace má LSB = 0
        int actual_address = address<<1;
        
        // Pokusíme se provést write transakci o délce 0 bytů, pouze pro zjištění, zda zařízení existuje
        if (i2c.write(actual_address, NULL, 0) == 0) {
            print_serial("Nalezeno I2C zařízení na adrese ");
            char array[] = {'a', 'b', 'c', 'd', 'e', 'l', 'g', 'd', 'd'};
            sprintf(array, "%d", actual_address);
            serial_port.write(array, 8);
            count++;
        }
        
    }
    
    print_serial("end");
    
    while(true) {
    }
}

// int main()
// {
//     led1 = led2 = led3 = true;
    
//     // Nastavení frekvence I2C sběrnice (100kHz)
//     i2c.frequency(1000000);
    
//     while(true) {
//         print_serial("marek");
//         if(get_pixy_version()) {
//             led2 = false;
//             led3 = true;
//         } else {
//             led2 = true;
//             led3 = false;
//         }
//         get_pixy_version();
//     }
// }

void print_serial(const char* message) {
    serial_port.write(message, strlen(message));
}

bool get_pixy_version() {
    char request[] = {
        0xc1,  // Sync (první bajt) - podle dokumentace 0xc1ae
        0xae,  // Sync (druhý bajt)
        PIXY_TYPE_REQUEST_VERSION,  // Typ požadavku (14)
        0x00  // Délka dat (v tomto případě 0)
    };
    
    // Odeslání požadavku s korekcí adresy (posun o 1 bit doleva)
    int result = i2c.write(0x54<<1, request, 4);
    
    if (result != 0) {
        print_serial("Chyba pri zapisu I2C\r\n");
        return false;
    } else {
        print_serial("OK zapis");
    }
    
    // Počkáme na zpracování
    ThisThread::sleep_for(chrono::milliseconds(100));
    
    // Buffer pro přijetí odpovědi
    uint8_t response[22];  // Velikost paketu pro verzi
    
    // Čtení odpovědi
    result = i2c.read(PIXY2_I2C_ADDRESS << 1, (char*)response, 22);
    
    if (result != 0) {
        print_serial("Chyba pri cteni I2C\r\n");
        return false;
    }
    
    // Kontrola, zda je odpověď platná - sync je 0xc1af
    if (response[0] != 0xc1 || response[1] != 0xaf || response[2] != PIXY_TYPE_RESPONSE_VERSION) {
        print_serial("Neplatna odpoved\r\n");
        return false;
    }
    
    // Vytvoření zprávy s verzí podle protokolu:
    // 6-7: HW verze, 8: FW major, 9: FW minor, 10-11: FW build, 12+: FW typ
    char version_msg[150];
    char fw_type[10] = {0}; // pro uložení typu firmware
    memcpy(fw_type, &response[12], 10); // Kopírovat řetězec typu firmware
    
    sprintf(version_msg, "Pixy2 verze:\r\nHW: v%d\r\nFW: v%d.%d.%d %s\r\nChecksum: 0x%04x\r\n", 
            response[6] | (response[7] << 8),  // Hardware verze (16-bit)
            response[8],                        // Firmware major
            response[9],                        // Firmware minor
            response[10] | (response[11] << 8), // Firmware build (16-bit)
            fw_type,                           // Firmware typ (string)
            response[4] | (response[5] << 8)    // Checksum
    );
    
    print_serial(version_msg);
    return true;
}

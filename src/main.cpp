#include "mbed.h"
#include "Pixy2I2C.h"
#include "TPixy2.h"
#include "ArduinoCompat.h"

// Vytvoření typedef pro Pixy2 s I2C komunikací
typedef TPixy2<Link2I2C> Pixy2I2C;

// Inicializace sériové komunikace
UnbufferedSerial pc(USBTX, USBRX);

// Pomocná funkce pro výpis na sériovou linku
void print_serial(const char* message) {
    pc.write(message, strlen(message));
}

int main() {
    Pixy2I2C pixy;
    
    print_serial("marecek");
    int8_t result = pixy.init();
    if (result < 0) {
        char buffer[64];
        sprintf(buffer, "Chyba při inicializaci Pixy2: %d\r\n", result);
        print_serial(buffer);
        return -1;
    }
    
    print_serial("Pixy2 úspěšně inicializována.\r\n");
    
    pixy.setLED(0, 0, 255);
    pixy.setLamp(255, 255);
    
    result = pixy.getVersion();
    if(result < 0)
        return -3;
    while(true) {
        
        char buffer[128];
        sprintf(buffer, "Hardware Version: %d\r\nFirmware Version: %d.%d\r\nFirmware Build: %d\r\nFirmware Type: %.10s\r\n\r\n", 
                pixy.version->hardware, 
                pixy.version->firmwareMajor, 
                pixy.version->firmwareMinor, 
                pixy.version->firmwareBuild, 
                pixy.version->firmwareType);
        print_serial(buffer);
        wait_us(1000000);
    }

}
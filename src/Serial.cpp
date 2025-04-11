#ifndef SERIAL_CPP
#define SERIAL_CPP
#include "mbed.h"

// Třída pro sériovou komunikaci podobnou Arduinu založená na základní UnbufferedSerial
class Serial {
private:
    UnbufferedSerial* serial;
public:
    // Konstruktor - inicializace sériového rozhraní
    Serial(PinName tx = USBTX, PinName rx = USBRX, int baudrate = 9600) {
        serial = new UnbufferedSerial(tx, rx, baudrate);
    }
    
    // Destruktor
    ~Serial() {
        delete serial;
    }
    
    // Zápis řetězce s přidáním nového řádku (kompatibilní s Arduino)
    size_t println(const char* str) {
        size_t written = serial->write(str, strlen(str));
        const char nl[] = "\r\n";
        return written + serial->write(nl, 2);
    }
    
    // Zápis celého čísla (kompatibilní s Arduino)
    size_t print(int value) {
        char buffer[12]; // Dostatečně velký buffer pro int
        int len = snprintf(buffer, sizeof(buffer), "%d", value);
        return serial->write(buffer, len);
    }
    
    // Zápis celého čísla s přidáním nového řádku (kompatibilní s Arduino)
    size_t println(int value) {
        char buffer[12]; // Dostatečně velký buffer pro int
        int len = snprintf(buffer, sizeof(buffer), "%d", value);
        size_t written = serial->write(buffer, len);
        const char nl[] = "\r\n";
        return written + serial->write(nl, 2);
    }
};
#endif
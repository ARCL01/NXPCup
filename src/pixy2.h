#ifndef PIXY2_H
#define PIXY2_H

#include "mbed.h"

// Definice konstant
#define PIXY_ARRAYSIZE               100
#define PIXY_START_WORD              0xaa55
#define PIXY_START_WORD_CC           0xaa56
#define PIXY_START_WORDX             0x55aa

#define PIXY_SYNC_BYTE               0x5a
#define PIXY_SYNC_BYTE_DATA          0x5b

// Pixy2 blok
typedef struct {
    uint16_t signature;
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    int16_t angle;
} Block;

// Hlavní třída Pixy2
class Pixy2 {
public:
    Pixy2(PinName mosi, PinName miso, PinName sclk, PinName cs);
    ~Pixy2();
    
    bool init();
    int8_t getVersion();
    int8_t getBlocks(uint8_t maxBlocks = 0xFF);
    
    // Pole pro ukládání detekovaných bloků
    Block blocks[PIXY_ARRAYSIZE];
    uint8_t blocksCount;

private:
    SPI _spi;
    DigitalOut _cs;
    Timer _timer;
    
    // Buffer pro SPI komunikaci
    uint8_t _buf[64];
    uint8_t _bufPayload[32];
    
    // Pomocné metody pro SPI komunikaci
    uint16_t getWord();
    uint8_t getByte();
    int8_t send(uint8_t *data, uint8_t len);
    int8_t recv(uint8_t *data, uint8_t len, uint16_t *cs = NULL);
    int16_t recvPacket();
    int16_t getResync();
};

#endif // PIXY2_H
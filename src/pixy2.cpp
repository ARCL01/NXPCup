#include "pixy2.h"

Pixy2::Pixy2(PinName mosi, PinName miso, PinName sclk, PinName cs) : 
    _spi(mosi, miso, sclk), _cs(cs) {
    _spi.format(8, 0); // 8 bit, mode 0
    _spi.frequency(1000000); // 1MHz SPI clock
    _cs = 1; // CS inactive high
    blocksCount = 0;
}

Pixy2::~Pixy2() {
}

bool Pixy2::init() {
    // Resync s kamerou
    for (uint8_t i = 0; i < 5; i++) {
        if (getResync() >= 0) {
            return true;
        }
        wait_us(100000); // 100ms
    }
    return false;
}

int8_t Pixy2::getVersion() {
    uint8_t outBuf[4];
    uint8_t inBuf[16];
    int16_t result;
    
    // Připrav paket
    outBuf[0] = 0x00; // Verze příkazu
    outBuf[1] = PIXY_SYNC_BYTE; // Sync byte
    outBuf[2] = 0x14; // Příkaz pro verzi
    outBuf[3] = 0; // Délka

    // Odešli paket
    send(outBuf, 4);
    
    // Přijmi odpověď
    result = recv(inBuf, 16);
    
    if (result < 0)
        return result;
        
    // Vytiskni verzi - pro debuggování
    printf("Hardware: %d.%d\n", inBuf[8], inBuf[9]);
    printf("Firmware: %d.%d.%d\n", inBuf[12], inBuf[13], inBuf[14]);
    
    return 0;
}

int8_t Pixy2::getBlocks(uint8_t maxBlocks) {
    uint8_t outBuf[6];
    int16_t result;
    
    // Připrav paket
    outBuf[0] = 0x00; // Verze příkazu 
    outBuf[1] = PIXY_SYNC_BYTE; // Sync byte
    outBuf[2] = 0x20; // Příkaz pro získání bloků
    outBuf[3] = 2; // Délka
    outBuf[4] = 1; // Barevné bloky
    outBuf[5] = 0; // Rezervováno
    
    // Odešli paket
    send(outBuf, 6);
    
    // Přijmi odpověď
    result = recvPacket();
    
    if (result < 0) {
        blocksCount = 0;
        return result;
    }
    
    // Počet bloků je výsledek / 14 (14 bytů na blok)
    blocksCount = result / 14;
    if (blocksCount > maxBlocks)
        blocksCount = maxBlocks;
        
    if (blocksCount > PIXY_ARRAYSIZE)
        blocksCount = PIXY_ARRAYSIZE;
    
    // Zpracuj bloky
    for (uint8_t i = 0; i < blocksCount; i++) {
        blocks[i].signature = _buf[i*14] | (_buf[i*14+1] << 8);
        blocks[i].x = _buf[i*14+2] | (_buf[i*14+3] << 8);
        blocks[i].y = _buf[i*14+4] | (_buf[i*14+5] << 8);
        blocks[i].width = _buf[i*14+6] | (_buf[i*14+7] << 8);
        blocks[i].height = _buf[i*14+8] | (_buf[i*14+9] << 8);
        blocks[i].angle = _buf[i*14+10] | (_buf[i*14+11] << 8);
    }
    
    return blocksCount;
}

uint16_t Pixy2::getWord() {
    // Načti word (16 bitů) přes SPI
    uint16_t w;
    
    _cs = 0;
    w = _spi.write(0) & 0xff;
    w |= (_spi.write(0) & 0xff) << 8;
    _cs = 1;
    
    return w;
}

uint8_t Pixy2::getByte() {
    // Načti byte (8 bitů) přes SPI
    uint8_t b;
    
    _cs = 0;
    b = _spi.write(0) & 0xff;
    _cs = 1;
    
    return b;
}

int8_t Pixy2::send(uint8_t *data, uint8_t len) {
    // Odešli data přes SPI
    _cs = 0;
    for (uint8_t i = 0; i < len; i++) {
        _spi.write(data[i]);
    }
    _cs = 1;
    
    return len;
}

int8_t Pixy2::recv(uint8_t *data, uint8_t len, uint16_t *cs) {
    // Přijmi data přes SPI
    uint16_t checksum = 0;
    
    _cs = 0;
    for (uint8_t i = 0; i < len; i++) {
        data[i] = _spi.write(0) & 0xff;
        if (cs)
            checksum += data[i];
    }
    _cs = 1;
    
    if (cs)
        *cs = checksum;
        
    return len;
}

int16_t Pixy2::recvPacket() {
    uint8_t header[4];
    int16_t res;
    
    // Načti hlavičku paketu
    recv(header, 4);
    
    // Zkontroluj sync byte
    if (header[0] != 0xaf || header[1] != 0xb3) {
        return getResync();
    }
    
    // Velikost dat
    uint16_t dataLen = header[3] << 8 | header[2];
    
    // Zkontroluj velikost dat
    if (dataLen > sizeof(_buf)) {
        return -1;
    }
    
    // Načti data
    res = recv(_buf, dataLen);
    
    if (res < 0)
        return res;
        
    return dataLen;
}

int16_t Pixy2::getResync() {
    uint8_t buf[4];
    uint8_t i;
    int16_t res;
    
    // Vyprázdni RX buffer
    for (i = 0; i < 4; i++)
        buf[i] = 0;
        
    // Pošli sync blok
    buf[0] = 0x00;
    buf[1] = PIXY_SYNC_BYTE;
    buf[2] = 0; // Prázdný příkaz
    buf[3] = 0; // Nulová délka
    
    send(buf, 4);
    
    // Počkej na odpověď
    for (i = 0; i < 100; i++) {
        wait_us(500);
        
        // Zkus přijmout paket
        res = recvPacket();
        if (res >= 0)
            return res;
    }
    
    return -1;
}
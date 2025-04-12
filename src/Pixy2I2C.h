//
// I2C komunikační vrstva pro Pixy2 kamera na mbed platformě
//

#ifndef _LINK2I2C_H
#define _LINK2I2C_H

#include "mbed.h"
#include "DefaultSerial.h"

#define PIXY_I2C_DEFAULT_ADDR           0x54
#define PIXY_DEFAULT_ARGVAL             0x80000000
#define PIXY_I2C_MAX_SEND               16 // don't send any more than 16 bytes at a time

#include "TPixy2.h"

// I2C komunikační vrstva pro Pixy2
class Link2I2C
{
public:
    Link2I2C(): m_i2c(PTE25, PTE24)
    {
    }
    
    int8_t open(uint32_t arg)
    {
        if (arg == PIXY_DEFAULT_ARGVAL)
            m_addr = PIXY_I2C_DEFAULT_ADDR;
        else
            m_addr = arg;
        
        return 0;
    }
    
    void close()
    {
        // Mbed I2C nepotřebuje explicitní uzavření
    }
    
    int16_t recv(uint8_t *buf, uint8_t len, uint16_t *cs=NULL)
    {
        int result;
        
        if (cs)
            *cs = 0;
        
        // Mbed i2c.read očekává 8-bitovou adresu posunutou o bit doleva 
        if(len !=0) {

            result = m_i2c.read(m_addr << 1, (char*)buf, len);
        } else {
            return len;
        }
        
        if (result != 0)
        {
            return PIXY_RESULT_ERROR; // Chyba komunikace
        }

        // Výpočet kontrolního součtu, pokud je požadován
        if (cs)
        {
            for (uint8_t i = 0; i < len; i++)
            {
                *cs += buf[i];
            }
        }
        
        return len;
    }
    
    int16_t send(uint8_t *buf, uint8_t len)
    {
        int8_t i, packet;
        int result;
        
        // Posíláme data v blocích (max 16 bytů najednou)
        for (i = 0; i < len; i += PIXY_I2C_MAX_SEND)
        {
            if (len - i < PIXY_I2C_MAX_SEND)
                packet = len - i;
            else
                packet = PIXY_I2C_MAX_SEND;
            
            // Mbed i2c.write očekává 8-bitovou adresu posunutou o bit doleva
            if(len!=0) {
                result = m_i2c.write(m_addr << 1, (const char*)(buf + i), packet);
            } else {
                return len;
            }
            
            if (result != 0)
            {
                return PIXY_RESULT_ERROR; // Chyba komunikace
            }
        }
        
        return len;
    }
    
private:
    I2C m_i2c;
    uint8_t m_addr;
};


#endif // _LINK2I2C_H
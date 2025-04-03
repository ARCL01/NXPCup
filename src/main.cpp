/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "pixy2.h"

#define WAIT_TIME_MS 500 
DigitalOut led1(LED1);
DigitalOut led2(LED2);

// Pixy2 kamera - upravte piny podle vašeho zapojení
// Pro FRDM-K64F můžete použít piny pro SPI:
// PTC6 (MOSI), PTC7 (MISO), PTC5 (SCK), PTC4 (CS)
Pixy2 pixy(PTC6, PTC7, PTC5, PTC4);

int main()
{
    printf("This is the bare metal blinky example running on Mbed OS %d.%d.%d.\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    // Inicializace Pixy2
    if (!pixy.init()) {
        printf("Chyba inicializace Pixy2!\n");
        // Blikání LED1 při chybě
        while (true) {
            led1 = !led1;
            thread_sleep_for(200);
        }
    }
    
    printf("Pixy2 inicializována úspěšně.\n");
    
    // Získej verzi pro ověření komunikace
    pixy.getVersion();

    while (true)
    {
        led1 = !led1;
        
        // Získej bloky z Pixy2
        int blocks = pixy.getBlocks();
        
        if (blocks > 0) {
            printf("Detekováno %d bloků:\n", blocks);
            led2 = 1;  // LED2 značí detekci
            
            // Projdi detekované bloky
            for (int i = 0; i < blocks; i++) {
                printf("  Blok %d: sig=%u x=%u y=%u šířka=%u výška=%u úhel=%d\n", 
                       i, pixy.blocks[i].signature, pixy.blocks[i].x, 
                       pixy.blocks[i].y, pixy.blocks[i].width, 
                       pixy.blocks[i].height, pixy.blocks[i].angle);
            }
        } else {
            led2 = 0;  // Žádná detekce
        }
        
        thread_sleep_for(WAIT_TIME_MS);
    }
}

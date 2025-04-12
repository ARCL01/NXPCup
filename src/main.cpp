#include "mbed.h"
#include "Pixy2I2C.h"
#include "TPixy2.h"
#include "ArduinoCompat.h"
#include "DefaultSerial.h"

// Vytvoření typedef pro Pixy2 s I2C komunikací
typedef TPixy2<Link2I2C> Pixy2I2C;

int main() {
    // Inicializace Pixy2 kamery
    Pixy2I2C pixy;
    
    // Inicializace kamery
    debugSerial.println("Inicializuji Pixy2 kameru...");
    int8_t res = pixy.init();
    
    if (res < 0) {
        char buf[32];
        sprintf(buf, "Chyba pri inicializaci Pixy2: %d", res);
        debugSerial.println(buf);
        return -1;
    }
    
    debugSerial.println("Pixy2 kamera uspesne inicializovana");
    
    // Výpis verze firmware
    if (pixy.version) {
        char buf[64];
        sprintf(buf, "Pixy2 verze: HW: %d.%d SW: %d.%d.%d %s", 
                pixy.version->hardware >> 8, pixy.version->hardware & 0xff,
                pixy.version->firmwareMajor, pixy.version->firmwareMinor, 
                pixy.version->firmwareBuild, pixy.version->firmwareType);
        debugSerial.println(buf);
    }
    
    // Nekonečná smyčka pro získávání dat
    while (1) {
        // Získání všech dostupných vlastností
        int8_t features = pixy.line.getAllFeatures();
        
        if (features < 0) {
            char buf[32];
            sprintf(buf, "Chyba pri ziskavani prvku: %d", features);
            debugSerial.println(buf);
        } else if (features == 0) {
            debugSerial.println("Zadne prvky nebyly detekovany");
        } else {
            char buf[64];
            sprintf(buf, "Detekovane prvky: 0x%02X (", features);
            if (features & LINE_VECTOR) strcat(buf, "vektory ");
            if (features & LINE_INTERSECTION) strcat(buf, "krizovatky ");
            if (features & LINE_BARCODE) strcat(buf, "carove kody ");
            strcat(buf, ")");
            debugSerial.println(buf);
            
            // Výpis informací o vektorech
            if (pixy.line.numVectors) {
                sprintf(buf, "Detekovano %d vektoru:", pixy.line.numVectors);
                debugSerial.println(buf);
                
                for (uint8_t i = 0; i < pixy.line.numVectors; i++) {
                    Vector *v = &pixy.line.vectors[i];
                    sprintf(buf, "Vektor %d: (%d,%d) do (%d,%d) index: %d flags: %d", 
                            i, v->m_x0, v->m_y0, v->m_x1, v->m_y1, v->m_index, v->m_flags);
                    debugSerial.println(buf);
                    
                    // Interpretace příznaků vektoru
                    if (v->m_flags) {
                        sprintf(buf, "  Priznaky: ");
                        if (v->m_flags & LINE_FLAG_INVALID) strcat(buf, "NEPLATNY ");
                        if (v->m_flags & LINE_FLAG_INTERSECTION_PRESENT) strcat(buf, "KRIZOVATKA_PRITOMNA ");
                        debugSerial.println(buf);
                    }
                }
            }
            
            // Výpis informací o křižovatkách
            if (pixy.line.numIntersections) {
                sprintf(buf, "Detekovano %d krizovatek:", pixy.line.numIntersections);
                debugSerial.println(buf);
                
                for (uint8_t i = 0; i < pixy.line.numIntersections; i++) {
                    Intersection *intr = &pixy.line.intersections[i];
                    sprintf(buf, "Krizovatka %d: (%d,%d) s %d carami", 
                            i, intr->m_x, intr->m_y, intr->m_n);
                    debugSerial.println(buf);
                    
                    for (uint8_t j = 0; j < intr->m_n; j++) {
                        sprintf(buf, "  Cara %d: index: %d uhel: %d", 
                                j, intr->m_intLines[j].m_index, intr->m_intLines[j].m_angle);
                        debugSerial.println(buf);
                    }
                }
            }
            
            // Výpis informací o čárových kódech
            if (pixy.line.numBarcodes) {
                sprintf(buf, "Detekovano %d carovych kodu:", pixy.line.numBarcodes);
                debugSerial.println(buf);
                
                for (uint8_t i = 0; i < pixy.line.numBarcodes; i++) {
                    Barcode *bc = &pixy.line.barcodes[i];
                    sprintf(buf, "Carovy kod %d: (%d,%d) hodnota: %d priznaky: %d", 
                            i, bc->m_x, bc->m_y, bc->m_code, bc->m_flags);
                    debugSerial.println(buf);
                    
                    // Interpretace příznaků čárového kódu
                    if (bc->m_flags) {
                        sprintf(buf, "  Priznaky: ");
                        if (bc->m_flags & LINE_FLAG_INVALID) strcat(buf, "NEPLATNY ");
                        debugSerial.println(buf);
                    }
                }
            }
        }
        
        // Pauza před dalším čtením
        wait_us(250000); // 250ms pauza
    }
}
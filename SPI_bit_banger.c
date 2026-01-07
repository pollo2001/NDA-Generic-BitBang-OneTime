#include "driverlib.h"
//*****************************************************************************
// NDA-SAFE GENERIC BIT-BANG SPI TEMPLATE (Configurable bit width)
// GPIO pins:
// LE_PIN   -> GPIO output (Latch Enable)
// DATA_PIN -> GPIO output (Data / MOSI)
// CLK_PIN  -> GPIO output (Clock)
//*****************************************************************************

#define LE_PIN   GPIO_PIN3   // Example: P1.3
#define DATA_PIN GPIO_PIN4   // Example: P1.4
#define CLK_PIN  GPIO_PIN5   // Example: P1.5

// Configurable bit width for SPI write
#define SPI_BIT_WIDTH 24  // Can scale up to 64, higher values may slow down transfers or hit slave buffer limits

// Prototypes
void SPI_Write(uint64_t data);
void setupPins();
void shutdownPins();

void main(void)
{
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);

    setupPins();

    // Optional small wait for slave initialization
    __delay_cycles(100);

    // Example writes (replace with actual device values)
    SPI_Write(0x000000); // Config 1
    SPI_Write(0x000000); // Config 2
    SPI_Write(0x000000); // Config 3
    SPI_Write(0x000000); // Config 4

    // Let LE settle briefly
    __delay_cycles(2);

    shutdownPins();

    // Enter deep sleep (LPM4) to minimize power and digital noise
    __bis_SR_register(LPM4_bits);
    __no_operation();
}

// Generic bit-bang SPI write function
void SPI_Write(uint64_t data){
    int8_t i;

    // Pull LE low to start transfer
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, LE_PIN);

    // Send each bit MSB-first
    for(i = SPI_BIT_WIDTH-1; i >= 0; i--){
        if((data >> i) & 0x01)
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, DATA_PIN);
        else
            GPIO_setOutputLowOnPin(GPIO_PORT_P1, DATA_PIN);

        // Pulse CLK
        GPIO_setOutputHighOnPin(GPIO_PORT_P1, CLK_PIN);
        GPIO_setOutputLowOnPin(GPIO_PORT_P1, CLK_PIN);
    }

    // Latch high to finalize transfer
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, LE_PIN);
}

// Setup GPIO pins and DCO
void setupPins(){
    GPIO_setAsOutputPin(GPIO_PORT_P1, LE_PIN | DATA_PIN | CLK_PIN);
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, DATA_PIN | CLK_PIN);
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, LE_PIN);

    // Max DCO frequency for timing
    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_3);
}

// Shutdown GPIO pins / clocks safely
void shutdownPins(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, LE_PIN | DATA_PIN | CLK_PIN);
    CS_turnOffSMCLK();
}

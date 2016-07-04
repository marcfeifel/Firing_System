/////////////////////////////////////
//  Generated Initialization File  //
/////////////////////////////////////

#include "compiler_defs.h"
#include "C8051F930_defs.h"

// Peripheral specific initialization functions,
// Called from the Init_Device() function
void PCA_Init()
{
    PCA0MD    &= ~0x40;
    PCA0MD    = 0x00;
}

void Timer_Init()
{
    TMR2CN    = 0x04;
    TMR2RLL   = 0x06;
    TMR2RLH   = 0xF8;
    TMR2L     = 0x06;
    TMR2H     = 0xF8;
    TMR3CN    = 0x04;
    TMR3RLL   = 0x06;
    TMR3RLH   = 0xF8;
    TMR3L     = 0x03;
    TMR3H     = 0xFC;
}

void SPI_Init()
{
    SPI0CFG   = 0x40;
    SPI0CN    = 0x0D;
    SPI0CKR   = 0x0B;
}

void Port_IO_Init()
{
    // P0.0  -  Skipped,     Push-Pull,  Digital
    // P0.1  -  Skipped,     Push-Pull,  Digital
    // P0.2  -  Skipped,     Push-Pull,  Digital
    // P0.3  -  Skipped,     Push-Pull,  Digital
    // P0.4  -  Skipped,     Push-Pull,  Digital
    // P0.5  -  Skipped,     Push-Pull,  Digital
    // P0.6  -  Skipped,     Push-Pull,  Digital
    // P0.7  -  Skipped,     Open-Drain, Digital

    // P1.0  -  Skipped,     Open-Drain, Digital
    // P1.1  -  Skipped,     Open-Drain, Digital
    // P1.2  -  Skipped,     Open-Drain, Digital
    // P1.3  -  Skipped,     Open-Drain, Digital
    // P1.4  -  Skipped,     Open-Drain, Digital
    // P1.5  -  Skipped,     Open-Drain, Digital
    // P1.6  -  SCK  (SPI0), Push-Pull,  Digital
    // P1.7  -  MISO (SPI0), Open-Drain, Digital

    // P2.0  -  MOSI (SPI0), Push-Pull,  Digital
    // P2.1  -  NSS  (SPI0), Push-Pull,  Digital
    // P2.2  -  Skipped,     Push-Pull,  Digital
    // P2.3  -  Skipped,     Push-Pull,  Digital
    // P2.4  -  Skipped,     Open-Drain, Digital
    // P2.5  -  Skipped,     Push-Pull,  Digital
    // P2.6  -  Skipped,     Push-Pull,  Digital
    // P2.7  -  Skipped,     Open-Drain, Digital

    P0MDOUT   = 0x7F;
    P1MDOUT   = 0x40;
    P2MDOUT   = 0x6F;
    P0SKIP    = 0xFF;
    P1SKIP    = 0x3F;
    P2SKIP    = 0xFC;
    XBR0      = 0x02;
    XBR2      = 0x40;
}

void Oscillator_Init()
{
    char i;

    FLSCL     = 0x40;
    REG0CN    |= 0x10;  // Enable OSCBIAS bit in voltage regulator
    for (i = 0; i < 14; i++);  // Wait 4us before enabling oscillator
    OSCICN    |= 0x8F;
    CLKSEL    = 0x00;
}

void Interrupts_Init()
{
    IP        = 0x30;
    EIE1      = 0x80;
    IE        = 0xA0;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    PCA_Init();
    Timer_Init();
    SPI_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
}

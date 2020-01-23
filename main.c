/*
 * Program name, short description, creation date and author's name
 * Optional: Include Licence information (GPL, CC, EULA, etc)
 */

//Parameter defines
#define FCY 29491200UL

//Library includes
#include <p30F4011.h>
//#include <libpic30.h>

//Device configuration registers
#pragma config FPR      = XT_PLL16
#pragma config FOS      = PRI
#pragma config FCKSMEN  = CSW_FSCM_OFF
#pragma config WDT      = WDT_OFF
#pragma config MCLRE    = MCLR_EN
#pragma config FPWRT    = PWRT_OFF

//Device Main routine
int main() {
    TRISFbits.TRISF1    = 0;
    TRISEbits.TRISE8    = 1;
    LATFbits.LATF1      = 1;
    while (1) {
        LATFbits.LATF1  = PORTEbits.RE8;
    }           
    return 0;
}


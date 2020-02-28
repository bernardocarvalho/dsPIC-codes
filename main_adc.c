/*
 * Program name, short description, creation date and author's name
 * Optional: Include Licence information (GPL, CC, EULA, etc)
 */

//Parameter defines
#define FCY 29491200UL

//Library includes
#include <xc.h>
// xc.h finds the correct library to include based on $MCU
#include <libpic30.h>


//Device configuration registers
#pragma config FPR      = XT_PLL16
#pragma config FOS      = PRI
#pragma config FCKSMEN  = CSW_FSCM_OFF
#pragma config WDT      = WDT_OFF
#pragma config MCLRE    = MCLR_EN
#pragma config FPWRT    = PWRT_OFF
//extern  void nop_test(void);

#define LED1   _LATF0   //define port for LED1
#define LED2   _LATF1   //define port for LED1

/*This is global var to use with UART2*/
char c;
unsigned int an0pin[128];
unsigned int an1pin[128];
unsigned char count=0;
unsigned char printdata=0;

/* This is TMR1 ISR */
void __attribute__((interrupt, auto_psv)) _T1Interrupt(void){
    IFS0bits.T1IF = 0;
    LED2 = ~LED2;

    if(count >= 0 && count <= 127){
        _SAMP=0;
        while(!_DONE);
        an0pin[count]=ADCBUF1;
        an1pin[count]=ADCBUF0;
        count++;
    }
}

/* This is UART2 receive ISR */
void __attribute__((__interrupt__, __no_auto_psv__)) _U2RXInterrupt(void){
    IFS1bits.U2RXIF = 0;  //resets RX2 interrupt flag
    c = U2RXREG;          //reads the character to the c variable
    if(c=='q'){
        _T1IE = 0;
        count=0;
        T1CONbits.TON = 0;
        TMR1 = 0;
    }
    else if(c=='a'){
        _T1IE = 1;
        TMR1 = 0;
        count = 0;
        T1CONbits.TON = 1;
    }
    else {}
}


/*This is the UART2 configuration routine*/
void uart_config(void){
    /**********************************
      SerialPort configuration
    **********************************/
    U2MODEbits.UARTEN = 0;  // Bit15 TX, RX DISABLED, ENABLE at end of func
    U2MODEbits.USIDL = 0;   // Bit13 Continue in Idle
    U2MODEbits.WAKE = 0;    // Bit7 No Wake up (since we don't sleep here)
    U2MODEbits.LPBACK = 0;  // Bit6 No Loop Back
    U2MODEbits.ABAUD = 0;   // Bit5 No Autobaud (would require sending '55')
    U2MODEbits.PDSEL = 0;   // Bits1,2 8bit, No Parity
    U2MODEbits.STSEL = 0;   // Bit0 One Stop Bit

    // Load a value into Baud Rate Generator.  Example is for 11500.
    //  U2BRG = (Fcy/(16*BaudRate))-1
    //  U2BRG = (29491200/(16*115200))-1
    //  U2BRG = 15
    U2BRG = 15;

    IFS1bits.U2RXIF = 0;    // Clear the Receive Interrupt Flag
    IEC1bits.U2RXIE = 1;    // Enable Receive Interrupts

    U2MODEbits.UARTEN = 1;  // And turn the peripheral on

    __C30_UART = 2;
    /**********************************
      End of serialPort configuration
    **********************************/
}

void configure_simsamp_autosamp_adc10(){

// dsPIC30F4011 - ADC 10-bits (Simultaneous sampling AN0 and AN1 using automatic sampling and MUXA)
    //Other options are avaliable to obtain simultaneously sample two pins. This is just an example!
   // Important Notes :) :
   //
   // MUXA and BUFFER connections:
   // AN1 -(connection)-> Connected to CH0 -(converting...)-> Write Buffer 0x0 (ADCBUF0)
   // AN0 -(connection)-> Connected to CH1 -(converting...)-> Write Buffer 0x1 (ADCBUF1)
   //
   // OPERATION ROUTINE:
   // _SAMP=0; will start conversion since sampling is automatic
   // while(!_DONE); wait until conversion is done and buffers are ready
   // an0pin=ADCBUF1; get the data on the buffers corresponding to the two pins
   // an1pin=ADCBUF0;

//****************************//
//ADCON1: A/D Control Register 1
//****************************//
	ADCON1bits.ADON=0;		//**A/D Operating Mode bit**//
							// 0-A/D converter is off
							// 1-A/D converter module is operating
                            // NOTE: Remember to put the ADON register to 1 before using!

	ADCON1bits.ADSIDL=0;	//**Stop in Idle Mode bit**//
							// 0-Continue module operation in Idle mode
							// 1-Discontinue module operation when device enters Idle mode

	ADCON1bits.FORM=0;		//**Sata Output Format bits**//
							// 0-Integer
							// 1-Signed integer
							// 2-Fractional
							// 3-Singed fractional

	ADCON1bits.SSRC=0;		//**Conversion Trigger Source Select bits**//
							// 0-Clearing SAMP bit ends sampling and starts conversion
							// 1-Active transition on INT0 pin ends sampling and starts conversion
							// 2-General purpose Timer3 compare ends sampling and starts conversion
							// 3-Motor Control PWM interval ends sampling and starts conversion
							// 4-Reserved
							// 5-Reserved
							// 6-Reserved
							// 7-Internal counter ends sampling and starts conversion (auto convert)

    ADCON1bits.SIMSAM=1;    //Simultaneous Sample Select bit (only applicable when CHPS = 01 or 1x)
                            //1 = Samples CH0, CH1, CH2, CH3 simultaneously (when CHPS = 1x)
                            //or
                            //Samples CH0 and CH1 simultaneously (when CHPS = 01)
                            //0 = Samples multiple channels individually in sequence


	ADCON1bits.ASAM=1;		//**A/D Sample Auto-Start bit**//
							// 0-Sampling begins when SAMP bit set
							// 1-Sampling begins immediately after last conversion completes. SAMP bit is auto set.

	ADCON1bits.SAMP=0;		//**A/D Sample Enable bit**//
							// 0-A/D sample/hold amplifiers are holding
							// 1-At least one A/D sample/hold amplifier is sampling


//****************************//
//ADCON2: A/D Control Register 2
//****************************//
	ADCON2bits.VCFG=0;		//**Voltage Reference Configuration bits**//
							// 0- AVdd and AVss
							// 1- External Vref+_pin and AVss
							// 2- AVdd	and External Vref-_pin
							// 3- External Vref+ pin and External Vref- pin
							// (4-7)- AVdd and AVss

    ADCON2bits.CHPS = 1;    //1x = Converts CH0, CH1, CH2 and CH3
                            //01 = Converts CH0 and CH1
                            //00 = Converts CH0

	ADCON2bits.CSCNA=0;		//**Scan Input Selections for CH0+ S/H Input for MUX A Multiplexer Setting bit**/
                            // 0-Do not scan inputs
                            // 1-Scan inputs

	ADCON2bits.BUFS=0;		//**Buffer Fill Status bit**//
                            // 0-A/D is currently filling buffer 0x0-0x7, user should access data in 0x8-0xF
                            // 1-A/D is currently filling buffer 0x8-0xF, user should access data in 0x0-0x7

	ADCON2bits.SMPI=0;		//**Sample/Convert Sequences Per Interrupt Selection bits**//
                            // 0-Interrupts at the completion of conversion for each sample/convert sequence
                            // 1-Interrupts at the completion of conversion for each 2nd sample/convert sequence
                            // ...
                            // 14-Interrupts at the completion of conversion for each 15th sample/convert sequence
                            // 15-Interrupts at the completion of conversion for each 16th sample/convert sequence

	ADCON2bits.BUFM=0;		//**Buffer Mode Select bit**//
                            // 0-Buffer configured as one 16-word buffer ADCBUF(15...0)
                            // 0-Buffer configured as one 8-word buffer ADCBUF(15...8), ADCBUF(7...0)

	ADCON2bits.ALTS=0;		//**Alternate Input Sample Mode Select bit**//
                            // 0-Always use MUX A input multiplexer settings
                            /* 1-Uses MUX A input multiplexer settings for first sample, then alternate between MUX B and
                            MUX A input multiplexer settings for all subsequent samples*/

//****************************//
//ADCON3: A/D Control Register 3
//****************************//
	ADCON3bits.SAMC=3;		//**Auto Sample Time bits**// (Used when you have auto conversion mode enabled which is not the case)
                            // (0-31) Tad

	ADCON3bits.ADRC=0;		//**A/D Conversion Clock Source bit**//
                            // 0-Clock derived from system clock
                            // 1-A/D internal RC clock

	ADCON3bits.ADCS=20;		//**A/D Conversion Clock Select bits**//
                            // (1-64)*Tcy/2

//******************************//
//ADCHS: A/D Input Select Register
//******************************//
    ADCHSbits.CH123NB=0;    //(MUXB not used in this application)

    ADCHSbits.CH123SB=0;    //(MUXB not used in this application)

    ADCHSbits.CH123NA=0;    //0x -> CH1-, CH2-, CH3- = VREF- (GND)

    ADCHSbits.CH123SA=0;    //0 -> CH1+ = AN0, CH2+ = AN1, CH3+ = AN2


	ADCHSbits.CH0NB=0;		//Channel 0 Negative Input Select. Same definition as in CH0NA (MUXB not used in this application)

	ADCHSbits.CH0SB=0;      //Channel 0 Positive Input Select for MUX B Multiplexer Setting bits (MUXB not used in this application)

	ADCHSbits.CH0NA=0;      // Channel 0 Negative Input Select for MUX A Multiplexer Setting bit
                            //1 = Channel 0 negative input is AN1
                            //0 = Channel 0 negative input is VREF- (GND)

	ADCHSbits.CH0SA=1;      //Channel 0 Positive Input Select for MUX A Multiplexer Setting bits
                            //1111 = Channel 0 positive input is AN15
                            //1110 = Channel 0 positive input is AN14
                            //1101 = Channel 0 positive input is AN13
                            //...
                            //0001 = Channel 0 positive input is AN1
                            //0000 = Channel 0 positive input is AN0

//*************************************//
//ADPCFG: A/D Port Configuration Register
//*************************************//

	ADPCFG = 0xFFFC;        //AN0 and AN1 selected as ADC input (No need to define TRIS for these pins!)
                            //1 = Analog input pin in Digital mode, port read input enabled, A/D input multiplexer input connected to AVSS
                            //0 = Analog input pin in Analog mode, port read input disabled, A/D samples pin voltage

//*************************************//
//ADCSSL: A/D Input Scan Select Register
//*************************************//

	ADCSSL = 0x0000;        //AN0 and AN1 - A/D Input Pin Scan Selection bits (for sequential sampling only!)
                            //1 = Select ANx for input scan
                            //0 = Skip ANx for input scan

// AD Interrupts
    _ADIE=0;                //No interruptions are required for this application. Use your own timer.
                            //Otherwise set ADIE register to 1 and check register SMPI value!


// Start the ADC module! (Keep it here OR comment and set the bit to 1 on a specific position of your code)
    ADCON1bits.ADON=1;
}

//Device Main routine
int main() {
    short int ii;
    long  il=5, ll =2, lk;
    float a=1.1, b=1.1, c;
    TRISFbits.TRISF1    = 0;
    TRISEbits.TRISE8    = 1;
    LATFbits.LATF1      = 1;

    //nop_test(); //assembly function call example
   /* Beggining of execution */

    printf("Serial port says: Hello\n\r");  //checks if UART2 is working fine

    _TRISF0 = 0;   //Configure led port as output
    _TRISF1 = 0;   //Configure led port as output

    LED1 = 1;
    LED2 = 1;

    /*Timer_1 config*/
    T1CONbits.TON = 0;      //Timer_1 is OFF
    TMR1 = 0;               //resets Timer_1
    PR1 = 29490;            //sets the maximum count for Timer_1 to generate an interrupt every 1ms
    T1CONbits.TCS = 0;      //choose FCY as clock source for Timer_1
    T1CONbits.TCKPS = 0x0;  //sets the Timer-1 pre-scaler at 1
    IFS0bits.T1IF = 0;      //clears the Timer_1 interrupt flag
    _T1IE = 0;              //disables Timer_1 Interrupts
    T1CONbits.TON = 0;      //turns Timer_1 ON

    uart_config();
    configure_simsamp_autosamp_adc10();
    
    printf("Serial port says: Hello\n\r");  //checks if UART2 is working fine
    LED1 = 0;
    /*main cycle*/
    while(1)
    {
        if(count>=128){
            _T1IE = 0;
            count=0;
            T1CONbits.TON = 0;
            TMR1 = 0;
            printf("Acquisition Results:\n\r");
            for(count=0;count<128;count++){
                __delay_ms(50);
                printf("%03d\t%04d\t%04d\n\r", count,an0pin[count],an1pin[count]);
            }
            count=0;
            printf("END\n\r");
        }
        else{}

    }
    return 0;
}


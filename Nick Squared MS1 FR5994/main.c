#include <msp430.h>
#include <config.h>


unsigned int NRB=0;//not received bytes
int BIP=0; //bytes in pocket
unsigned char message[76];

void initialize();

void main(void)
{
    initialize();

    _BIS_SR(LPM0_bits+GIE);
    while(1);
}

//USCI (UART) interrupt
#pragma vector=USCI_A0_VECTOR
__interrupt void uart(void)
{
    LED1ON;
if (NRB==0)
{
    //For the first byte
    BIP=UCA0RXBUF;
    while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = UCA0RXBUF-3;
    NRB++;
}
else if (0<NRB && 3>=NRB)
{
    switch(NRB)
    {
    case 1:
    {
        TB0CCR3=UCA0RXBUF;//red, P3.4
        //break;
    }
    case 2:
    {
        TB0CCR4=UCA0RXBUF;//green, P3.5
        //break;
    }
    case 3:
    {
        TB0CCR5=UCA0RXBUF;//blue, P3.5
        //break;
    }
    }
    NRB++;
}
else if(4<=NRB && BIP>NRB)
{
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = UCA0RXBUF;
        NRB++;
}
else
{
   // while (!(UCA0IFG&UCTXIFG)); // USCI_A0 TX buffer ready?
   // UCA0TXBUF=message[NRB];
    NRB=0;
    //UCA0IFG=0;
}

//UCA0IFG=0;//Clears UART IFG
}//End uart ISR

void initialize()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5; //Disables high impedence mode

    TB0CTL=TASSEL_2+ID_3+MC_1; //Sets timer A0 to SMCLK/8 in upmode

    TB0CCR0 = 255; //Sets CCR0

    TB0CCTL3 = OUTMOD_7; //Sets CCR4 to Set/Reset
    TB0CCR3 = 2; //Sets CCR4, higher number= lower brightness

    TB0CCTL4 = OUTMOD_7; //Sets CCR5 to Set/Reset
    TB0CCR4 = 2; //Sets CCR5

    TB0CCTL5 = OUTMOD_7; //Sets CCR3 to Set/Reset
    TB0CCR5 = 254; //Sets CCR3


    //Configure PWM bits(Pin 3.4, 3.5, 3.6)
    P3SEL0 |=BIT4|BIT5|BIT6;
    P3SEL1 &=~(BIT4|BIT5|BIT6);
    P3DIR |=BIT4|BIT5|BIT6;
    //Led goes: Red, ground, Blue, Green
    //Ground is the longest pin

    // Configure GPIO
    P2SEL0 &= ~(BIT0 | BIT1);
    P2SEL1 |= BIT0 | BIT1;                  // USCI_A0 UART operation

    // Startup clock system with max DCO setting ~8MHz
    CSCTL0_H = CSKEY_H;                     // Unlock CS registers
    CSCTL1 = DCOFSEL_3 | DCORSEL;           // Set DCO to 8MHz
    CSCTL2 = SELA__VLOCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3 = DIVA__1 | DIVS__1 | DIVM__1;   // Set all dividers
    CSCTL0_H = 0;                           // Lock CS registers

    // Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                    // Put eUSCI in reset
    UCA0CTLW0 |= UCSSEL__SMCLK;             // CLK = SMCLK
    UCA0BRW = 52;                           // 8000000/16/9600
    UCA0MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA0CTLW0 &= ~UCSWRST;                  // Initialize eUSCI
    UCA0IE |= UCRXIE;                       // Enable USCI_A0 RX interrupt

    LED1INIT;
    LED1OFF;

}


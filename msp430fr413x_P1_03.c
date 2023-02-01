#include <msp430.h>

#include "ram_isrs.h"

__interrupt void ISR_RED_LED(void);         // Prototype so we can reference it in ISR_GREEN_LED

// Interrupt service routine
__interrupt void ISR_GREEN_LED(void)
{
    P4OUT ^= BIT0;                      // Toggle LED1
    __delay_cycles(10000);              // Debounce

    P1IFG &= ~BIT2;                     // Clear P1.2 IFG

    ram_vector_PORT1 = &ISR_RED_LED;    // Switch to the other ISR, which will get called on the next interrupt. Cool, right?

    __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
}

#pragma vector = PORT1_VECTOR
__interrupt void ISR_RED_LED(void)
{
    P1OUT ^= BIT0;                      // Toggle LED1
    __delay_cycles(10000);              // Debounce

    P1IFG &= ~BIT2;                     // Clear P1.2 IFG

    ram_vector_PORT1 = &ISR_GREEN_LED;  // Switch to the other ISR, which will get called on the next interrupt. Cool, right?


    __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
}



int main(void)
{
    // Assign the GREN LED ISR to the PORT1 vector in the RAM vector table
    ram_vector_PORT1 = &ISR_GREEN_LED;

    // Switch to RAM-based vector table as per SLAU445I 1.15.1
    SYSCTL |= SYSRIVECT;

    // OK, any interrupts on PORT1 now to go to the GREEN LED ISR

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer

    // Configure GPIO RED LED
    P1DIR |= BIT0;                          // Set P1.0 to output direction
    P1OUT |= BIT0;                          // Start LED on so we know it is working

    // Configure GPIO GREEN LED
    P4DIR |= BIT0;                          // Set P4.0 to output direction
    P4OUT |= BIT0;                          // Start LED on so we know it is working


    // Configure GPIO Button 1
    P1OUT |= BIT2;                          // Configure P1.3 as pulled-up
    P1REN |= BIT2;                          // P1.3 pull-up register enable
    P1IES |= BIT2;                          // P1.3 Hi/Low edge
    P1IE  |= BIT2;                           // P1.3 interrupt enabled

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;
    
    P1IFG &= ~BIT2;                         // P1.2 IFG cleared
    
    while(1)
    {
        __bis_SR_register(LPM3_bits | GIE); // Enter LPM3 w/interrupt
        __no_operation();                   // For debugger
    }
}



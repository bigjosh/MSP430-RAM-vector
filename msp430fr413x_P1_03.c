#include <msp430.h>

// All the RAM-based interrupt vectors. Ideally, these would be pointers to `__interrupt` functions, but the compiler can't handle that.
// Note these are void pointers rather than function pointers because if you make them
// function pointers then the compiler will try to make trampolines to them and fail.
// There does not seem to be a way to specify that a pointer is "small" with the TI compiler.
// Note these need to be `volatile` so the compiler always writes the values into the actual memory locations rather than potentially caching them.

__attribute__((section(".ram_int45"))) volatile void *ram_vector_LCD_E;
__attribute__((section(".ram_int46"))) volatile void *ram_vector_PORT2;
__attribute__((section(".ram_int47"))) volatile void *ram_vector_PORT1;
__attribute__((section(".ram_int48"))) volatile void *ram_vector_ADC;
__attribute__((section(".ram_int49"))) volatile void *ram_vector_USCI_B0;
__attribute__((section(".ram_int50"))) volatile void *ram_vector_USCI_A0;
__attribute__((section(".ram_int51"))) volatile void *ram_vector_WDT;
__attribute__((section(".ram_int52"))) volatile void *ram_vector_RTC;
__attribute__((section(".ram_int53"))) volatile void *ram_vector_TIMER1_A1;
__attribute__((section(".ram_int54"))) volatile void *ram_vector_TIMER1_A0;
__attribute__((section(".ram_int55"))) volatile void *ram_vector_TIMER0_A1;
__attribute__((section(".ram_int56"))) volatile void *ram_vector_TIMER0_A0;
__attribute__((section(".ram_int57"))) volatile void *ram_vector_UNMI;
__attribute__((section(".ram_int48"))) volatile void *ram_vector_SYSNMI;
//__attribute__((section(".ram_int59"))) void *ram_vector_RESET;          // This one is dumb because the SYSRIVECT bit gets cleared on reset so this can never happen.

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


#define END_OF_FRAM 0x10000

#define P1_FRAM_VECTOR (0xFFE6)

#define P1_VECTOR_OFFSET (END_OF_FRAM - P1_FRAM_VECTOR)

#define END_OF_RAM  0x02800
#define P1_RAM_VECTOR (END_OF_RAM-P1_VECTOR_OFFSET)


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



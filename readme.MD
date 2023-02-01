# Description

The MSP430 lets you use the SYSRIVECT bit to move the table of interrupt vectors from FRAM to RAM. Unfortunately there are no examples of how to do this - I bet becuase it is a huge pain! Here is a working sample program that runs on the MSP430FR4133-EXP demo board that moves the table into RAM and then changes a vector while the program is running.

# Use

Load this onto the demo board. Each time you press the left button, you should one of the LEDs toggle, and it should alternate between LEDs on each press. 

# How

The program has two ISRs, one called `ISR_RED_LED` and one called `ISR_GREEN_LED`. It starts off with the interrupt vector pointing to `ISR_GREEN_LED` but inside each IR, the code changes the vector on the fly to point to the other ISR.  

# Why

I wanted to do this for two reasons...

1. I wanted to keep the vectors in RAM so I could implement a state machine by changing the vector so there was nolatency  overhead with checking a state variable - the inetrrupt instantly jumps right into the correct place in the state machine. I could have done this with the FRAM-based table would have required unlocking and locking the FRAM each time I change state (or leaving it unlocked all the time, which feels reckless). 

2. I wanted to comepletely turn off the FRAM controller to save power. I had already moved the ISRs into RAM, but the chip still needed to access FRAM to get the vectors. Moving the table means that there are no FRAM accesses at all in the entire ISR cycle. 

# Turning off the FRAM controller

If you move the vector table to RAM and also mark your ISR as `ramfunc` and also make sure you do not access any FRAM memory in your ISR, then you can turn off the FRAM controller with this code..

```
FRCTL0=FRCTLPW;                  // FRCTLPW password. Always reads as 96h. To enable write access to the FRCTL registers, write A5h.
GCCTL0 &= ~FRPWR ;               // FRAM power control. Writing to the register enables or disables the FRAM power supply.  0b = FRAM power supply disabled.
```

This has slight power and latency benifits. Here are measurements from my project...

| FRAM  | Power Avg | Power Max | Wake Time |
| --- | --- | --- | --- |
| ON | 11.7uA | 260uA  | 26.6us |
| OFF | 11.5uA | 344uA max | 25.5us |

I was surprised the power savings were so slight, and I am not sure why the max power is higher with RAM.

Saving 1us on wake time might be valuable for some projects. Note that I was waking from LPM3 with SVS disabled. 

# Warnings

You can not statically initialize the vectors, you can only assign them in code. While it would be possible to make it so these static initialization worked, it is too much work for me to do now. If you get it work, please send me a PR!

# Sorry

I know this is all so ugly, but I wanted to copy the style of the existing TI linker scripts which are themselves so ugly. If you want to make everything nice so that it that automatically relocates the RAM vector table to the top of RAM above the stack, copies initialized values from FRAM to RAM at startup, and also has nice macros to put functions into that table and also just generally cleans up the TI mess, then you are a better person than I am so please send me a PR!  

-josh

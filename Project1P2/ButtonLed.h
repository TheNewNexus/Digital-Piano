// ButtonLed.h: starter file for CECS447 Project 1 Part 1
// Runs on TM4C123,
// Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat
#ifndef BUTTONLED_H
#define BUTTONLED_H

#include <stdint.h>

// Constants
#define PIANO 0     // piano mode: press a key to play a tone
#define AUTO_PLAY 1 // auto play mode: automatic playing a song

#define OCTAVE_LED1 0x10 // PC4
#define OCTAVE_LED2 0x20 // PC5
#define OCTAVE_LED3 0x40 // PC6

// Function prototypes
void ButtonLed_Init(void);
void PianoKeys_Init(void);
uint8_t get_current_mode(void);

// Shared variables
// extern volatile uint8_t curr_mode;
// extern volatile uint8_t curr_song;
// extern volatile uint8_t octave;

#endif // BUTTONLED_H
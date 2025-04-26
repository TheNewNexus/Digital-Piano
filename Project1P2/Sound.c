// Sound.c
// This is the starter file for CECS 447 Project 1 Part 2
// By Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat
#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "sound.h"
#include "ButtonLed.h"

// define bit addresses for Port B bits 0,1,2,3,4,5 => DAC inputs
#define DAC (*((volatile unsigned long *)0x400050FC))

// 6-bit: value range 0 to 2^6-1=63, 64 samples
const uint8_t SineWave[64] = {32, 35, 38, 41, 44, 47, 49, 52, 54, 56, 58, 59, 61, 62, 62, 63, 63, 63, 62, 62,
                              61, 59, 58, 56, 54, 52, 49, 47, 44, 41, 38, 35, 32, 29, 26, 23, 20, 17, 15, 12,
                              10, 8, 6, 5, 3, 2, 2, 1, 1, 1, 2, 2, 3, 5, 6, 8, 10, 12, 15, 17,
                              20, 23, 26, 29};

// initial values for piano major tones.
// Assume SysTick clock frequency is 16MHz.
const uint32_t tonetab[] =
    // C, D, E, F, G, A, B
    // 1, 2, 3, 4, 5, 6, 7
    // lower C octave:130.813, 146.832,164.814,174.614,195.998, 220,246.942
    // calculate reload value for the whole period:Reload value = Fclk/Ft = 16MHz/Ft
    {                                                                                // 122137, 108844, 96970, 91429, 81633, 72727, 64777,
        30534 * 2, 27211 * 2, 24242 * 2, 22923 * 2, 20408 * 2, 18182 * 2, 16194 * 2, // C4 Major notes
        15289 * 2, 13621 * 2, 12135 * 2, 11454 * 2, 10204 * 2, 9091 * 2, 8099 * 2,   // C5 Major notes
        7645 * 2, 6810 * 2, 6067 * 2, 5727 * 2, 5102 * 2, 4545 * 2, 4050 * 2,        // C6 Major notes
        3822 * 2, 3822 * 2, 3405 * 2, 3034 * 2, 2863 * 2, 2551 * 2, 2273 * 2, 2025 * 2};

// Constants
// index definition for tones used in happy birthday.
#define C4 0
#define D4 1
#define E4 2
#define F4 3
#define G4 4
#define A4 5
#define B4 6
#define C5 7  // 0 + 7
#define D5 8  // 1 + 7
#define E5 9  // 2 + 7
#define F5 10 // 3 + 7
#define G5 11 // 4 + 7
#define A5 12 // 5 + 7
#define B5 13 // 6 + 7
#define C6 14 // 0 + 2 * 7
#define D6 15 // 1 + 2 * 7
#define E6 16 // 2 + 2 * 7
#define F6 17 // 3 + 2 * 7
#define G6 18 // 4 + 2 * 7
#define A6 19 // 5 + 2 * 7
#define B6 20 // 6 + 2 * 7

uint8_t i, j, k = 0;

#define PIANO 0
#define AUTO_PLAY 1
#define MAX_NOTES 255             // maximum number of notes for a song to be played in the program
#define NUM_SONGS 4               // number of songs in the play list.
#define SILENCE MAX_NOTES         // use the last valid index to indicate a silence note. The song can only have up to 254 notes.
#define NUM_OCT 3                 // number of octave defined in tontab[]
#define NUM_NOTES_PER_OCT 7       // number of notes defined for each octave in tonetab
#define NVIC_EN0_PORTF 0x40000000 // enable PORTF edge interrupt
#define NVIC_EN0_PORTD 0x00000008 // enable PORTD edge interrupt
#define NUM_SAMPLES 64            // number of sample in one sin wave period

// note table for Happy Birthday
// doe ray mi fa so la ti
// C   D   E  F  G  A  B
NTyp playlist[][MAX_NOTES] = {
    {// Happy Birthday
     G4, 2, G4, 2, A4, 4, G4, 4, C5, 4, B4, 4,
     SILENCE, 4, G4, 2, G4, 2, A4, 4, G4, 4, D5, 4, C5, 4,
     SILENCE, 4, G4, 2, G4, 2, G5, 4, E5, 4, C5, 4, B4, 4, A4, 8,
     SILENCE, 4, F5, 2, F5, 2, E5, 4, C5, 4, D5, 4, C5, 8, SILENCE, 0},

    {// Mary Had A Little Lamb
     E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8,
     D4, 4, D4, 4, D4, 8, E4, 4, G4, 4, G4, 8,
     E4, 4, D4, 4, C4, 4, D4, 4, E4, 4, E4, 4, E4, 8,
     D4, 4, D4, 4, E4, 4, D4, 4, C4, 8, SILENCE, 0},

    {// Twinkle Twinkle Little Star
     C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8,
     F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8,
     G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8,
     G4, 4, G4, 4, F4, 4, F4, 4, E4, 4, E4, 4, D4, 8,
     C4, 4, C4, 4, G4, 4, G4, 4, A4, 4, A4, 4, G4, 8,
     F4, 4, F4, 4, E4, 4, E4, 4, D4, 4, D4, 4, C4, 8, SILENCE, 0},

    {// Ol' MacDonald Had a Farm
     G4, 4, G4, 4, G4, 8, D4, 4, E4, 4, E4, 8, D4, 8,
     B4, 4, B4, 4, A4, 4, A4, 4, G4, 8, SILENCE, 4,
     G4, 4, G4, 4, G4, 8, D4, 4, E4, 4, E4, 8, D4, 8,
     B4, 4, B4, 4, A4, 4, A4, 4, G4, 8, SILENCE, 0}};
// File scope golbal

volatile uint8_t curr_mode = PIANO;
volatile uint8_t curr_song = 0;
volatile uint8_t next_song = 0;
volatile uint8_t octave = 0;
// volatile uint8_t curr_song = 0; // 0: Happy Birthday, 1: Mary Had A Little Lamb. 2: Twinkle Twinkle Little Stars
volatile uint8_t stop_play = 1; // 0: continue playing a song, 1: stop playing a song
// volatile uint8_t octave = 0;	// 0: lower C, 1: middle C, 2: upper C
unsigned char Index;
volatile uint32_t current_note = 0; // Global variable to store the current note being played
uint8_t octavechange[3] = {0, 7, 14};

void Delay(void);

void Update_Octave_LEDs(void)
{
    GPIO_PORTC_DATA_R &= ~0x70; // Clear the LED bits (PC4, PC5, PC6)
    switch (octave)
    {
    case 0:
        GPIO_PORTC_DATA_R |= 0x10; // Turn on PC4 (LED1)
        break;
    case 1:
        GPIO_PORTC_DATA_R |= 0x20; // Turn on PC5 (LED2)
        break;
    case 2:
        GPIO_PORTC_DATA_R |= 0x40; // Turn on PC6 (LED3)
        break;
    default:
        // Do nothing, no valid octave
        break;
    }
}

// Initialize 6-bit DAC
// Input: none
// Output: none
void DAC_Init(void)
{
    SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOB; // activate port B
    while ((SYSCTL_RCGC2_R & SYSCTL_RCGC2_GPIOB) != SYSCTL_RCGC2_GPIOB)
        ;

    GPIO_PORTB_AMSEL_R &= ~0x3F;      // no analog
    GPIO_PORTB_PCTL_R &= ~0x00FFFFFF; // regular function
    GPIO_PORTB_DIR_R |= 0x3F;         // make PB5-0 out
    GPIO_PORTB_AFSEL_R &= ~0x3F;      // disable alt funct on PB5-0
    GPIO_PORTB_DEN_R |= 0x3F;         // enable digital I/O on PB5-0
    GPIO_PORTB_DR8R_R |= 0x3F;        // enable 8 mA drive on PB5-0

    // Initialize SysTick
    Index = 0;
    NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;                        // disable Systick during setup                // disable SysTick during setup
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x20000000; // priority 1
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN;   // enable SysTick with core clock and interrupts
}

// **************Sound_Start*********************
// Set reload value and enable systick timer
// Input: time duration to be generated in number of machine cycles
// Output: none
void Sound_Start(uint32_t period)
{
    NVIC_ST_RELOAD_R = period - 1; // reload value
    NVIC_ST_CURRENT_R = 0;         // any write to current clears it
    NVIC_ST_CTRL_R |= NVIC_ST_CTRL_ENABLE;
}

void Sound_stop(void)
{
    NVIC_ST_CTRL_R &= ~NVIC_ST_CTRL_ENABLE;
}

// Interrupt service routine
// Executed based on number of sampels in each period
void SysTick_Handler(void)
{
    Index = (Index + 1) % NUM_SAMPLES; // 16 samples for each period
    DAC = SineWave[Index];             // output to DAC: 3-bit data
}

void GPIOPortF_Handler(void)
{
    Delay(); // Debouncing
    Delay(); // Debouncing
    Delay(); // Debouncing
    if (GPIO_PORTF_RIS_R & 0x10)
    {                            // SW1 pressed (Toggle Piano/Auto-Play)
        GPIO_PORTF_ICR_R = 0x10; // Clear interrupt flag for SW1
        curr_mode ^= 1;          // Toggle between Piano and Auto-Play mode

        if (curr_mode == AUTO_PLAY)
        {
            octave = 0;
            Update_Octave_LEDs();
            curr_song = 0; // Reset to first song in Auto-Play
            // play_a_song();  // Start playing the first song
        }
        else
        {
            stop_play = 1; // Stop Auto-Play mode
            Sound_stop();  // Ensure the sound stops when switching to Piano mode
        }
    }

    else if (GPIO_PORTF_RIS_R & 0x01)
    { // SW2 pressed (Cycle Songs or Octaves)
        if (curr_mode == PIANO)
        {
            octave = (octave + 1) % 3; // Cycle through octaves in Piano mode
            Update_Octave_LEDs();
        }
        else if (curr_mode == AUTO_PLAY)
        {
            octave = 0;
            Update_Octave_LEDs();
            next_song = 1;
            curr_song = (curr_song + 1) % NUM_SONGS; // Cycle through songs in Auto-Play mode
            // play_a_song();  // Play the newly selected song
        }

        GPIO_PORTF_ICR_R = 0x01; // Clear interrupt flag for SW2
    }
}

void GPIOPortA_Handler(void)
{
    Delay(); // Debouncing the input
    if (curr_mode == PIANO)
    {
        if ((GPIO_PORTA_DATA_R & 0x80) == 0)
        { // PA7 (B)
            play_a_note(6, 6);
        }
        else
        {
            Sound_stop(); // Stop sound when no key is pressed
        }
    }
    GPIO_PORTA_ICR_R = 0x80; // Clear all interrupt flags
}

void GPIOPortD_Handler(void)
{
    Delay(); // Debouncing the input
    if (curr_mode == PIANO)
    {
        if ((GPIO_PORTD_DATA_R & 0x01) == 0)
        { // PD0 (C)
            play_a_note(0, 0);
        }
        else if ((GPIO_PORTD_DATA_R & 0x02) == 0)
        { // PD1 (D)
            play_a_note(1, 1);
        }
        else if ((GPIO_PORTD_DATA_R & 0x04) == 0)
        { // PD2 (E)
            play_a_note(2, 2);
        }
        else if ((GPIO_PORTD_DATA_R & 0x08) == 0)
        { // PD3 (F)
            play_a_note(3, 3);
        }
        else if ((GPIO_PORTD_DATA_R & 0x40) == 0)
        { // PD6 (G)
            play_a_note(4, 4);
        }
        else if ((GPIO_PORTD_DATA_R & 0x80) == 0)
        { // PD7 (A)
            play_a_note(5, 5);
        }
        else
        {
            Sound_stop(); // Stop sound when no key is pressed
        }
    }

    GPIO_PORTD_ICR_R = 0xCF; // Clear all interrupt flags
}

void play_a_note(uint8_t note, uint8_t index)
{
    if (curr_mode == PIANO)
    {
        uint8_t octave_offset = 7 * octave;
        Sound_Start(tonetab[octave_offset + index] / NUM_SAMPLES); // Play the note
    }
}

void play_a_song(void)
{
    uint32_t i = 0;
    NTyp *song = playlist[curr_song];

    while (song[i].delay != 0 && curr_mode == AUTO_PLAY)
    {

        if (song[i].tone_index == SILENCE)
        {
            Sound_stop();
        }
        else
        {
            Sound_Start(tonetab[song[i].tone_index + octave * 7] / NUM_SAMPLES);
        }

        for (uint32_t j = 0; j < song[i].delay; j++)
        {
            Delay();
        }

        Sound_stop();
        i++;

        if (next_song == 1)
        {
            next_song = 0;
            break;
        }
    }
}
uint8_t get_current_mode(void)
{
    return curr_mode;
}

// Subroutine to wait 0.1 sec
void Delay(void)
{
    uint32_t volatile time;
    time = 727240 * 20 / 91; // 0.1 sec
    while (time)
    {
        time--;
    }
}

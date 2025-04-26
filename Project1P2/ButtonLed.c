// ButtonLed.c: starter file for CECS447 Project 1 Part 1
// Runs on TM4C123,
// Dr. Min He
// September 10, 2022
// Port B bits 5-0 outputs to the 6-bit DAC
// Port D bits 3-0 inputs from piano keys: CDEF:doe ray mi fa,negative logic connections.
// Port F is onboard LaunchPad switches and LED
// SysTick ISR: PF3 is used to implement heartbeat

#include "tm4c123gh6pm.h"
#include <stdint.h>
#include "ButtonLed.h"

// Constants
#define SW1 0x10                  // bit position for onboard switch 1(left switch)
#define SW2 0x01                  // bit position for onboard switch 2(right switch)
#define NVIC_EN0_PORTF 0x40000000 // enable PORTF edge interrupt
#define NVIC_EN0_PORTA 0x00000001 // enable PORTA edge interrupt
#define NVIC_EN0_PORTD 0x00000008 // enable PORTD edge interrupt

// Golbals
// volatile uint8_t curr_mode=PIANO;  // 0: piano mode, 1: auto-play mode
extern volatile uint8_t octave; // 0: lower C, 1: middle C, 2: upper C

//---------------------Switch_Init---------------------
// initialize onboard switch and LED interface
// Input: none
// Output: none
void ButtonLed_Init(void)
{
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;   // 1) activate clock for Port F
  delay = SYSCTL_RCGC2_R;         // allow time for clock to start
  GPIO_PORTF_LOCK_R = 0x4C4F434B; // 2) unlock GPIO Port F
  GPIO_PORTF_CR_R = 0x1F;         // allow changes to PF4-0
  // only PF0 needs to be unlocked, other bits can't be locked
  GPIO_PORTF_AMSEL_R &= ~0x1F;      // 3) disable analog on PF
  GPIO_PORTF_PCTL_R &= ~0x000FFFFF; // 4) PCTL GPIO on PF4-0
  GPIO_PORTF_DIR_R |= 0x0E;         // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_DIR_R &= ~0x11;        // 5) PF4,PF0 in, PF3-1 out
  GPIO_PORTF_AFSEL_R &= ~0x1F;      // 6) disable alt funct on PF7-0
  GPIO_PORTF_PUR_R |= 0x11;         // enable pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R |= 0x1F;         // 7) enable digital I/O on PF4-0
  // Configure interrupts for PF4 (SW1) and PF0 (SW2)
  GPIO_PORTF_IS_R &= ~0x11;  // PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R &= ~0x11; // PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R &= ~0x11; // PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R = 0x11;   // Clear flags 4,0
  GPIO_PORTF_IM_R |= 0x11;   // Arm interrupt on PF4,PF0

  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOC; // 1) activate clock for Port C
  delay = SYSCTL_RCGC2_R;               // allow time for clock to start
  GPIO_PORTC_DIR_R |= 0x70;             // set PC4, PC5, PC6 as output
  GPIO_PORTC_AFSEL_R &= ~0x70;          // disable alt funct on PC4, PC5, PC6
  GPIO_PORTC_PUR_R |= 0x70;
  GPIO_PORTC_DEN_R |= 0x70;                              // enable digital I/O on PC4, PC5, PC6
  GPIO_PORTC_DATA_R &= ~0x70;                            // Ensure all LEDs are off initially
  GPIO_PORTC_DATA_R |= 0x10;                             // Enable Octave 0
  NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF1FFFFF) | 0x00A00000; // priority 5
  NVIC_EN0_R |= NVIC_EN0_PORTF;
}

//---------------------PianoKeys_Init---------------------
// initialize onboard Piano keys interface: PORT D 0 - 3 are used to generate
// tones: CDEF:doe ray mi fa
// No need to unlock. Only PD7 needs to be unlocked.
// Input: none
// Output: none
void PianoKeys_Init(void)
{
  volatile unsigned long delay;
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOD; // 1) activate clock for Port D
  // Activate clock for Port A
  SYSCTL_RCGC2_R |= SYSCTL_RCGC2_GPIOA; // 1) activate clock for Port A
  delay = SYSCTL_RCGC2_R;               // allow time for clock to start
  // GPIO_PORTD_LOCK_R = 0x4C4F434B;   // 2) unlock GPIO Port F
  GPIO_PORTD_LOCK_R = GPIO_LOCK_KEY; // Unlock GPIO Port D

  GPIO_PORTD_CR_R = 0xFF;           // Allow changes to PD7-0    // unlock GPIO Port F
  GPIO_PORTD_AMSEL_R &= ~0xCF;      // 3) disable analog on PD
  GPIO_PORTD_PCTL_R &= ~0x0000FFFF; // 4) PCTL GPIO on PF4-0
  GPIO_PORTD_DIR_R &= ~0xCF;        // 5) PD in
  GPIO_PORTD_AFSEL_R &= ~0xCF;      // 6) disable alt funct on PD
  GPIO_PORTD_PUR_R |= 0xCF;         // enable pull-up on PD0-3
  GPIO_PORTD_DEN_R |= 0xCF;         // 7) enable digital I/O on PD

  // Configure Port A7
  // GPIO_PORTA_CR_R = 0x80;           // allow changes to PA7
  GPIO_PORTA_AMSEL_R &= ~0x80;      // 3) disable analog on PA7
  GPIO_PORTA_PCTL_R &= ~0xF0000000; // 4) PCTL GPIO on PA7
  GPIO_PORTA_DIR_R &= ~0x80;        // 5) PA7 out
  GPIO_PORTA_AFSEL_R &= ~0x80;      // 6) disable alt funct on PA7
  GPIO_PORTA_PUR_R |= 0x80;
  GPIO_PORTA_DEN_R |= 0x80; // 7) enable digital I/O on PA7

  GPIO_PORTD_IS_R &= ~0xCF;  //  PD0-3, 6-and 7 is edge-sensitive
  GPIO_PORTD_IBE_R |= 0xCF;  //  PD0-3, 6-and 7  is not both edges
  GPIO_PORTD_IEV_R &= ~0xCF; //  PD0-3, 6-and 7  falling edge event
  GPIO_PORTD_ICR_R = 0xCF;   //  Clear flags PD0-3, 6-and 7
  GPIO_PORTD_IM_R |= 0xCF;   //  Arm interrupt on PD0-3, 6-and 7

  // Interrupt for Port A7
  GPIO_PORTA_IS_R &= ~0x80;  //  PA7 is edge-sensitive
  GPIO_PORTA_IBE_R |= 0x80;  //  PA7 is not both edges
  GPIO_PORTA_IEV_R &= ~0x80; //  PA7 falling edge event
  GPIO_PORTA_ICR_R = 0x80;   //  Clear flags PA7
  GPIO_PORTA_IM_R |= 0x80;   //  Arm interrupt on PA7

  // Set priority for Port D and Port A
  NVIC_PRI0_R = (NVIC_PRI0_R & 0xFFFFFF0F) | 0x000000E0;
  NVIC_PRI0_R = (NVIC_PRI0_R & 0x0FFFFFFF) | 0xA0000000;
  NVIC_EN0_R |= NVIC_EN0_PORTA;
  // NVIC_PRI0_R = (NVIC_PRI0_R&0x1FFFFFFF)|0xA0000000;
  NVIC_EN0_R |= NVIC_EN0_PORTD;
}

// uint8_t get_current_mode(void) {
//     return curr_mode;
// }

uint8_t get_current_octave(void)
{
  return octave;
}
# Digital Piano Project

## Overview

This project implements a digital piano and music box using a TM4C123 microcontroller. It features both manual play and auto-play modes, with the ability to switch between different octaves and songs.

## Key Features

1. **Piano Mode**
   - Seven piano keys (C, D, E, F, G, A, B) using push buttons
   - Three octaves (lower C, middle C, upper C) with LED indicators
   - Real-time sound generation using a 6-bit R/2R DAC
   - 64-sample sinusoid wave for tone generation

2. **Auto-Play Mode**
   - Four pre-programmed songs:
     - Happy Birthday
     - Mary Had a Little Lamb
     - Twinkle Twinkle Little Star
     - Ol' MacDonald Had a Farm
   - Ability to switch between songs during playback

3. **Hardware Integration**
   - Uses onboard switches for mode selection and control
   - Implements GPIO interrupts for button presses
   - Utilizes SysTick timer for precise tone generation
   - Three LEDs to indicate the current octave

4. **Advanced Features**
   - Octave selection in Piano mode with visual LED feedback
   - Smooth transition between modes and songs

## Why This Project is Useful

- **Educational Value**: Covers embedded systems programming topics (GPIO, timers, DAC, interrupts)
- **Practical Application**: Demonstrates building a functional electronic musical instrument
- **Extensibility**: Modular design allows for easy feature additions
- **Hardware-Software Integration**: Showcases interaction between software and hardware components
- **Real-time Systems**: Provides hands-on experience with real-time audio generation

## Hardware Requirements

- TM4C123 Launchpad
- 7 push buttons (C, D, E, F, G, A, B)
- 7 10kΩ resistors for the push buttons
- Audio amplifier
- Speaker
- Resistors for 6-bit R2R DAC circuit
- 3 LEDs for octave indication

## Pin Connections

- Piano Keys:
  - PD0: C
  - PD1: D
  - PD2: E
  - PD3: F
  - PD6: G
  - PD7: A
  - PA7: B
- Octave LEDs:
  - PC4: Octave 0 (Lower C)
  - PC5: Octave 1 (Middle C)
  - PC6: Octave 2 (Upper C)
- Onboard Switches:
  - PF4: SW1 (left switch) - Toggle between Piano and Auto-Play mode
  - PF0: SW2 (right switch) - Cycle through octaves (in Piano mode) or songs (in Auto-Play mode)
- DAC Output:
  - PB5-PB0: 6-bit DAC output

## Usage

1. Power on the device. It starts in Piano mode with the lower C octave selected.
2. In Piano mode:
   - Press the piano key buttons to play notes
   - Use SW2 to cycle through octaves (observe the LED indicators)
3. Press SW1 to switch to Auto-Play mode
4. In Auto-Play mode:
   - The device will start playing songs automatically
   - Use SW2 to cycle through different songs
5. Press SW1 again to return to Piano mode

## Getting Started

[Include instructions on how to set up the project, compile the code, and flash it to the TM4C123 microcontroller]

## Contributors
Joseph G

Jonathan C

Robby R

Afzal H



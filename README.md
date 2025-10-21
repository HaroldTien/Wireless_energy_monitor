# Energy Monitor - ATmega328PB Implementation

## Overview
This project implements a single-phase Energy Monitor using an ATmega328PB microcontroller. It measures AC voltage, current, and power through external interrupt-triggered sampling, calculates average power, RMS voltage, and peak current, drives a 4-digit 7-segment display with scrolling values, and transmits formatted data via UART.

## Hardware Configuration

### Microcontroller
- **ATmega328PB**, 2 MHz, 5 V operation
- ADC: 10-bit, 8 channels (using ADC0, ADC1, ADC2)
- UART: 9600 bps, 8-N-1 (TX only)
- Timer0 → Display multiplexing (~10ms intervals)
- Timer1 → ADC auto-trigger setup (108μs intervals, no ISR)
- External Interrupt INT0 → Triggers new ADC sampling sequences

### Analog Front-End (LM324 Quad Op-Amp)
| Section | Function | Output Signal | Destination |
|----------|-----------|----------------|--------------|
| U3A | Current amplifier | Imeas | ADC1 (PC1) |
| U3B | Voltage amplifier | Vmeas | ADC0 (PC0) |
| U3C | Comparator | Vzc (zero-cross digital) | PD2 (INT0) |
| U3D | Mid-supply reference | Offset (~2.5 V) | ADC2 (PC2) |

### Display (P4 header)
- 4-digit 7-segment LED display
- Driven through 74HC595 shift registers
- Control lines: SHIFT_CLOCK (PC3), SHIFT_DATA (PC4), SHIFT_LATCH (PC5)
- Digit enable lines: DS1–DS4 (PD4–PD7)

### UART (P3 header)
- TX → PD1 (UART transmit)
- RX → PD0 (UART receive, optional)
- Baud = 9600 bps

## Software Architecture

### File Structure
```
├── main.c              # Main application loop and system initialization
├── config.h            # Pin definitions and system constants
├── adc.c/h             # ADC configuration and 24-sample collection routines
├── timer.c/h           # Timer configuration for display multiplexing and ADC setup
├── display.c/h         # 7-segment display driver with scrolling functionality
├── uart.c/h            # UART transmit functions with formatted data output
├── powercalc.c/h       # Power calculations (average power, RMS voltage, peak current)
└── int0.c/h            # External interrupt handler for triggering ADC sequences
```

### Key Features

#### ADC Configuration
- Reference: AVCC (5V)
- Resolution: 10-bit
- Prescaler: 16 (125kHz ADC clock)
- Channels: ADC0 (Vmeas), ADC1 (Imeas), ADC2 (Offset)
- Triggered by external interrupt (INT0) on zero-crossing detection
- Collects 24 samples per channel per measurement cycle

#### Timer Usage
- **Timer0**: Display multiplexing (~10ms interrupts for 7-segment refresh)
- **Timer1**: ADC auto-trigger setup (108μs intervals for rapid channel switching)

#### Power Calculations
- **Average Power**: (1/N) × Σ[(V[i] - offset[i]) × (I[i] - offset[i])] for N=24 samples
- **RMS Voltage**: √[(1/N) × Σ(V[i] - offset[i])²] for N=24 samples
- **Peak Current**: Maximum signed current value across 24 samples
- Thread-safe display buffer with atomic updates

#### Display Functionality
- 4-digit 7-segment display with 74HC595 shift register control
- Scrolls between three values every second:
  - Average Power (W) with 1 decimal place
  - RMS Voltage (V) with 1 decimal place
  - Peak Current (mA) with 1 decimal place
- Shows "no signal" state (four decimal points) when no data available

#### UART Data Transmission
- Format:
  ```
  Average Power = 5.2 W
  RMS Voltage = 14.1 V
  Peak Current = 712.5 mA
  ---
  ```
- Transmission rate: Every 1 second
- Shows "No Signal Detected" when no measurement data available

## Calibration Constants

```c
#define VOLTAGE_DIVIDER_RATIO 21.0f
#define CURRENT_SHUNT_RESISTOR 0.545f  // Ω
#define OPAMP_GAIN_VOLTAGE 82.0f
#define OPAMP_GAIN_CURRENT 2.1f
```

## Pin Mapping

| Function | MCU Pin | Direction | Description |
|-----------|----------|-----------|-------------|
| Vmeas | PC0 (ADC0) | Input | Voltage measurement |
| Imeas | PC1 (ADC1) | Input | Current measurement |
| Offset | PC2 (ADC2) | Input | Mid-supply reference |
| Zero-cross | PD2 (INT0) | Input | Zero-crossing detection |
| Shift clock | PC3 | Output | Shift register clock |
| Shift data | PC4 | Output | Shift register data |
| Shift latch | PC5 | Output | Shift register latch |
| DS1–DS4 | PD4–PD7 | Output | Digit enable lines |
| UART TX | PD1 | Output | UART transmit |
| UART RX | PD0 | Input | UART receive (optional) |

## Operation Sequence

1. **System Initialization**
   - Configure GPIO pins, ADC, UART, timers, and external interrupts
   - Initialize display driver and power calculation modules
   - Set up Timer0 for display multiplexing and Timer1 for ADC auto-trigger

2. **Wait for External Trigger**
   - System waits for zero-crossing signal on PD2 (INT0)
   - External interrupt triggers new ADC measurement sequence

3. **ADC Sampling Sequence** (Triggered by INT0)
   - Collect 24 samples each of voltage, current, and offset
   - Timer1 auto-trigger switches between ADC channels every 108μs
   - ADC ISR handles channel switching and sample storage

4. **Power Calculations** (After 24 samples collected)
   - Calculate average power using: (1/24) × Σ[(V[i]-offset[i]) × (I[i]-offset[i])]
   - Calculate RMS voltage using: √[(1/24) × Σ(V[i]-offset[i])²]
   - Calculate peak current: max(|I[i]-offset[i]|) across all samples

5. **Display Updates** (Every 1 second)
   - Scroll between three measured values on 4-digit display
   - Thread-safe buffer prevents display corruption during calculations

6. **UART Transmission** (Every 1 second)
   - Send formatted measurement data via UART
   - Show "No Signal Detected" when no measurements available

## Output Units
- Average Power → Watts (W) with 1 decimal place
- RMS Voltage → Volts (V) with 1 decimal place
- Peak Current → Milliamps (mA) with 1 decimal place

## Key Implementation Details

### Thread Safety
- Display buffer uses atomic operations (`cli()`/`sei()`) to prevent race conditions
- Calculations run in main loop while display continues showing previous values

### ADC Channel Switching Strategy
- Timer1 generates 108μs interrupts for rapid ADC channel switching
- ADC ISR handles automatic progression: Voltage → Current → Offset → Voltage
- External interrupt (INT0) triggers new 24-sample collection cycle

### Display Multiplexing
- Timer0 generates ~10ms interrupts for 7-segment display refresh
- Each interrupt updates one digit to create persistence of vision effect
- Display scrolls between three measured values every second

### Zero-Crossing Triggered Operation
- System waits for zero-crossing pulse on PD2 (INT0) to start measurements
- Ensures phase-locked sampling for accurate power calculations
- Prevents aliasing and improves measurement consistency



## Updates:
	#### Removed timer1 and adopt free running mode to continuouly get 24 samples
	of the 3 values(voltage, offset, currect). The INT0 interrupt trigger the
	ADC conversion start.
	

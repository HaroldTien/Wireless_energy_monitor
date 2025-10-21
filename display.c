#include "display.h"
#include "powercalc.h"
#include "config.h"
#include <avr/interrupt.h>

// 4 characters to be displayed on Ds1 to Ds4
static volatile uint8_t disp_characters[4] = {0, 0, 0, 0};
static volatile uint8_t disp_position = 0;

// Scrolling display variables
static volatile uint8_t scroll_mode = 0;  // 0=avg_power, 1=rms_voltage, 2=peak_current
static volatile uint32_t scroll_timer = 0;
static volatile uint32_t last_scroll_update = 0;

// 7-segment patterns for digits 0-9
const uint8_t seg_pattern[10] = {
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,           // 0
    SEG_B | SEG_C,                                           // 1
    SEG_A | SEG_B | SEG_G | SEG_E | SEG_D,                   // 2
    SEG_A | SEG_B | SEG_G | SEG_C | SEG_D,                   // 3
    SEG_F | SEG_G | SEG_B | SEG_C,                           // 4
    SEG_A | SEG_F | SEG_G | SEG_C | SEG_D,                   // 5
    SEG_A | SEG_F | SEG_G | SEG_E | SEG_D | SEG_C,           // 6
    SEG_A | SEG_B | SEG_C,                                   // 7
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,   // 8
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G            // 9
};

// Display initialization
void init_display(void)
{
    // Configure shift register control pins as outputs
    // SH_ST, SH_DS, SH_CP pins
    SHIFT_CLOCK_DDR |= (1 << SHIFT_CLOCK_BIT);   // SH_CP (PC3) as output
    SHIFT_DATA_DDR |= (1 << SHIFT_DATA_BIT);     // SH_DS (PC4) as output  
    SHIFT_LATCH_DDR |= (1 << SHIFT_LATCH_BIT);   // SH_ST (PC5) as output
    
    // Initialize shift register pins to LOW
    SHIFT_CLOCK_PORT &= ~(1 << SHIFT_CLOCK_BIT);
    SHIFT_DATA_PORT &= ~(1 << SHIFT_DATA_BIT);
    SHIFT_LATCH_PORT &= ~(1 << SHIFT_LATCH_BIT);
    
    // Configure digit selection pins as outputs
    // Ds1, Ds2, Ds3, Ds4 pins
    DIGIT1_DDR |= (1 << DIGIT1_BIT);     // Ds1 (PD4) as output
    DIGIT2_DDR |= (1 << DIGIT2_BIT);     // Ds2 (PD5) as output
    DIGIT3_DDR |= (1 << DIGIT3_BIT);     // Ds3 (PD6) as output
    DIGIT4_DDR |= (1 << DIGIT4_BIT);     // Ds4 (PD7) as output
    
    // Initialize all digit selection pins to HIGH (disabled)
    DIGIT1_PORT |= (1 << DIGIT1_BIT);    // Ds1 = HIGH (disabled)
    DIGIT2_PORT |= (1 << DIGIT2_BIT);    // Ds2 = HIGH (disabled)
    DIGIT3_PORT |= (1 << DIGIT3_BIT);    // Ds3 = HIGH (disabled)
    DIGIT4_PORT |= (1 << DIGIT4_BIT);    // Ds4 = HIGH (disabled)
}
// =============================================================================
// SHIFT REGISTER CONTROL - HELPER FUNCTIONS
// =============================================================================

/*
 * Shifts out one byte (8 bits) to the 74HC595 shift register
 * 
 * This function sends data serially to the shift register by:
 * 1. Setting the data pin (SH_DS) HIGH or LOW for each bit
 * 2. Pulsing the clock pin (SH_CP) to shift the bit into the register
 * 3. Repeating for all 8 bits, starting from MSB (bit 7) to LSB (bit 0)
 * 
 * @param data: 8-bit value representing segment pattern (a-g + decimal point)
 * 
 * Note: Data is sent MSB first to match the segment wiring
 */
static void shift_out_byte(uint8_t data)
{
    // Send 8 bits, starting from MSB (bit 7) down to LSB (bit 0)
    for (int i = 7; i >= 0; i--) {
        // Set data pin based on current bit value
        if (data & (1 << i)) {
            SHIFT_DATA_PORT |= (1 << SHIFT_DATA_BIT);   // Set data HIGH
        } else {
            SHIFT_DATA_PORT &= ~(1 << SHIFT_DATA_BIT);  // Set data LOW
        }
        
        // Generate clock pulse to shift the bit into the register
        SHIFT_CLOCK_PORT |= (1 << SHIFT_CLOCK_BIT);     // Clock HIGH
        SHIFT_CLOCK_PORT &= ~(1 << SHIFT_CLOCK_BIT);    // Clock LOW
    }
}

/*
 * Latches (transfers) data from shift register to output pins
 * 
 * The 74HC595 has two stages:
 * 1. Shift register (where data is loaded serially)
 * 2. Storage register (output pins)
 * 
 * This function transfers data from stage 1 to stage 2 by pulsing SH_ST pin.
 * After latching, the new segment pattern becomes visible on the display.
 */
static void latch_shift_register(void)
{
    SHIFT_LATCH_PORT |= (1 << SHIFT_LATCH_BIT);        // Latch HIGH
    SHIFT_LATCH_PORT &= ~(1 << SHIFT_LATCH_BIT);       // Latch LOW
}

/*
 * Disables all 4 digit enable pins
 * 
 * Sets all digit select pins (Ds1-Ds4) to HIGH, which turns OFF all digits.
 * This prevents ghosting/bleeding between digits during multiplexing.
 * 
 * Should be called BEFORE changing segment data and BEFORE enabling a new digit.
 */
static void disable_all_digits(void)
{
    DIGIT1_PORT |= (1 << DIGIT1_BIT);    // Ds1 = HIGH (disabled)
    DIGIT2_PORT |= (1 << DIGIT2_BIT);    // Ds2 = HIGH (disabled)
    DIGIT3_PORT |= (1 << DIGIT3_BIT);    // Ds3 = HIGH (disabled)
    DIGIT4_PORT |= (1 << DIGIT4_BIT);    // Ds4 = HIGH (disabled)
}

/*
 * Enables a specific digit based on position
 * 
 * Sets the corresponding digit select pin to LOW, which turns ON that digit.
 * The segment data from the shift register will be displayed on this digit.
 * 
 * Digit mapping:
 * - Position 0 (Thousands) -> Ds1 (leftmost digit)
 * - Position 1 (Hundreds)  -> Ds2
 * - Position 2 (Tens)      -> Ds3
 * - Position 3 (Units)     -> Ds4 (rightmost digit)
 * 
 * @param position: Which digit to enable (0-3)
 */
static void enable_digit(uint8_t position)
{
    switch (position) {
        case 0:  // Thousands digit -> Ds1
            DIGIT1_PORT &= ~(1 << DIGIT1_BIT);  // Ds1 = LOW (active)
            break;
        case 1:  // Hundreds digit -> Ds2
            DIGIT2_PORT &= ~(1 << DIGIT2_BIT);  // Ds2 = LOW (active)
            break;
        case 2:  // Tens digit -> Ds3
            DIGIT3_PORT &= ~(1 << DIGIT3_BIT);  // Ds3 = LOW (active)
            break;
        case 3:  // Units digit -> Ds4
            DIGIT4_PORT &= ~(1 << DIGIT4_BIT);  // Ds4 = LOW (active)
            break;
    }
}

/*
 * Advances to the next digit position for multiplexing
 * 
 * Increments the display position counter (0 -> 1 -> 2 -> 3 -> 0 -> ...)
 * This creates a circular scanning pattern through all 4 digits.
 * 
 * Called after each digit is displayed to prepare for the next one.
 */
static void multiplexing_display(void)
{
    disp_position++;
    if (disp_position > 3) {
        disp_position = 0;  // Wrap around to first digit
    }
}

// =============================================================================
// MAIN DISPLAY MULTIPLEXING FUNCTION
// =============================================================================

/*
 * Displays the next digit in the multiplexing sequence
 * 
 * This is the main function called from Timer ISR to implement display multiplexing.
 * It cycles through all 4 digits sequentially, displaying one digit at a time.
 * When called repeatedly at high frequency (e.g., every 1-5ms), persistence of
 * vision makes all 4 digits appear to be lit simultaneously.
 * 
 * Operation sequence:
 * 1. Get segment pattern for current digit from buffer
 * 2. Send segment pattern to shift register
 * 3. Disable all digits (prevent ghosting)
 * 4. Latch new segment data to output
 * 5. Enable only the current digit
 * 6. Advance to next digit position
 * 
 * Called from: Timer ISR (typically every 1-5ms)
 * 
 * Example timeline:
 * - Call 1: Display thousands digit (Ds1) with its segments
 * - Call 2: Display hundreds digit (Ds2) with its segments
 * - Call 3: Display tens digit (Ds3) with its segments
 * - Call 4: Display units digit (Ds4) with its segments
 * - Call 5: Back to thousands digit (cycle repeats)
 */
void send_next_character_to_display(void)
{
    // Step 1: Get the segment pattern for the current digit position
    uint8_t segments = disp_characters[disp_position];
    
    // Step 2: Send segment data to shift register (but not yet visible)
    shift_out_byte(segments);
    
    // Step 3: Turn off all digits first to prevent ghosting
    // (Important: Do this BEFORE latching new data)
    disable_all_digits();
    
    // Step 4: Latch the new segment pattern to the output pins
    // (Segments are now ready on the shift register outputs)
    latch_shift_register();
    
    // Step 5: Enable only the current digit to show the new segments
    // (Now the digit lights up with the correct pattern)
    enable_digit(disp_position);
    
    // Step 6: Move to next digit for the next function call
    multiplexing_display();
}

 
// =============================================================================
// DIGIT SELECTION
// =============================================================================

/*
 * Populate the array 'disp_characters[]' by separating the four digits of 'number' 
 * and then looking up the segment pattern from 'seg_pattern[]'
 * 
 * This function extracts individual digits from a number and converts them to 
 * 7-segment patterns. It also adds a decimal point at the specified position.
 * 
 * @param number: 16-bit number to display (0-9999)
 * @param decimal_pos: Position of decimal point (0 = no decimal, 1-3 = after that digit from right)
 * 
 * Decimal point position examples:
 * - decimal_pos = 0: No decimal point → "1234"
 * - decimal_pos = 1: After units digit → "123.4"
 * - decimal_pos = 2: After tens digit → "12.34"
 * - decimal_pos = 3: After hundreds digit → "1.234"
 * 
 * Display positions:
 * - Position 0 (Ds1): Thousands digit (leftmost)
 * - Position 1 (Ds2): Hundreds digit
 * - Position 2 (Ds3): Tens digit
 * - Position 3 (Ds4): Units digit (rightmost)
 */
void seperate_and_load_characters(uint16_t number, uint8_t decimal_pos)
{
    uint8_t digit;
    
    // Separate each digit from 'number' (e.g., 1230 yields '1', '2', '3', '0')
    // Correct mapping: Ds4=units, Ds3=tens, Ds2=hundreds, Ds1=thousands
    
    // Thousands digit -> Ds1 (position 0)
    digit = (number / 1000) % 10;
    disp_characters[0] = seg_pattern[digit];
    
    // Hundreds digit -> Ds2 (position 1)
    digit = (number / 100) % 10;
    disp_characters[1] = seg_pattern[digit];
    
    // Tens digit -> Ds3 (position 2)
    digit = (number / 10) % 10;
    disp_characters[2] = seg_pattern[digit];
    
    // Units digit -> Ds4 (position 3)
    digit = number % 10;
    disp_characters[3] = seg_pattern[digit];
    
    // Add decimal point if specified (decimal_pos counts from the right)
    // decimal_pos = 1: Add DP to units digit (position 3) - rightmost
    // decimal_pos = 2: Add DP to tens digit (position 2)
    // decimal_pos = 3: Add DP to hundreds digit (position 1)
    // decimal_pos = 4: Add DP to thousands digit (position 0) - leftmost
    if (decimal_pos > 0 && decimal_pos <= 4) {
        // Calculate which position gets the decimal point
        // decimal_pos=1 → position 3 (units) → dp_position = 3
        // decimal_pos=2 → position 2 (tens) → dp_position = 2
        // decimal_pos=3 → position 1 (hundreds) → dp_position = 1
        // decimal_pos=4 → position 0 (thousands) → dp_position = 0
        uint8_t dp_position = 4 - decimal_pos;  // Convert from right-count to position index
        
        // Add the decimal point segment to that digit
        disp_characters[dp_position] |= SEG_DP;
    }
}

// Initialize scrolling display
void init_scrolling_display(void)
{
    scroll_mode = 0;
    scroll_timer = 0;
    last_scroll_update = 0;
}

// Update scrolling display - call this every 1 second
void update_scrolling_display(void)
{
    // Check if display data is ready
    if (is_display_data_ready()) {
        // Show actual power data
        uint16_t display_value;
        uint8_t decimal_pos;
        
        switch (scroll_mode) {
            case 0: // Average Power
                display_value = (uint16_t)(get_display_power() * 10); // Show with 1 decimal place
                decimal_pos = 1;
                break;
                
            case 1: // RMS Voltage
                display_value = (uint16_t)(get_display_voltage() * 10); // Show with 1 decimal place
                decimal_pos = 1;
                break;
                
            case 2: // Peak Current
                display_value = (uint16_t)(get_display_current() * 100); // Show with 2 decimal places
                decimal_pos = 2;
                break;
                
            default:
                display_value = 0;
                decimal_pos = 0;
                break;
        }
        
        // Load the value to display
        seperate_and_load_characters(display_value, decimal_pos);
        
        // Move to next mode
        scroll_mode = (scroll_mode + 1) % 3;
    } else {
        // Show "no signal" status (four decimal points)
        display_no_signal();
    }
}

// Get current scroll mode for external use
uint8_t get_scroll_mode(void)
{
    return scroll_mode;
}

// Display "no signal" status (four decimal points)
void display_no_signal(void)
{
    disp_characters[0] = SEG_DP;  // First digit: decimal point only
    disp_characters[1] = SEG_DP;  // Second digit: decimal point only
    disp_characters[2] = SEG_DP;  // Third digit: decimal point only
    disp_characters[3] = SEG_DP;  // Fourth digit: decimal point only
}
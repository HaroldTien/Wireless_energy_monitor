/*
 * seven_seg.h
 * 
 * Header file for 7-segment display constants for numbers 0-9
 * Contains segment patterns for digits only
 * 
 * Created: 9/27/2025
 * Author: harol
 */

#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <stdint.h>

// =============================================================================
// 7-SEGMENT DISPLAY NUMBER PATTERNS
// =============================================================================

// Segment patterns for numbers 0-9
// Bit mapping: bit 0 = segment A, bit 1 = segment B, ..., bit 7 = decimal point
// Based on our shift register mapping: Q0->A, Q1->B, Q2->C, Q3->D, Q4->E, Q5->F, Q6->G, Q7->DP

#define SEG_0    0x3F    // 00111111 - segments A,B,C,D,E,F
#define SEG_1    0x06    // 00000110 - segments B,C
#define SEG_2    0x5B    // 01011011 - segments A,B,G,E,D
#define SEG_3    0x4F    // 01001111 - segments A,B,G,C,D
#define SEG_4    0x66    // 01100110 - segments F,G,B,C
#define SEG_5    0x6D    // 01101101 - segments A,F,G,C,D
#define SEG_6    0x7D    // 01111101 - segments A,F,G,E,D,C
#define SEG_7    0x07    // 00000111 - segments A,B,C
#define SEG_8    0x7F    // 01111111 - segments A,B,C,D,E,F,G
#define SEG_9    0x6F    // 01101111 - segments A,B,C,D,F,G

// Special patterns
#define SEG_BLANK   0x00    // 00000000 - all segments off


#endif // SEVEN_SEG_H

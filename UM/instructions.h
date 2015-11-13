/*
 * Mengtian Li
 * 11/12/2014
 * instructions.h 
 * contains 14 instructions
 */
#ifndef INSTRUCTIONS_INCLUDED
#define INSTRUCTIONS_INCLUDED
#include "memseg.h"
#include <bitpack.h>
#include <stdlib.h>
#include "um.h"

/* 
 * Takes in a piece of instruction, and perform instruction on my_UM
 * if $r[C] != 0 then $r[A] := $r[B]
 */
void conditional_move(uint32_t instruction, UM_T my_UM);

/* 
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $r[A] := $m[$r[B]][$r[C] *
 */
void segmented_load(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $m[$r[A]][$r[B]] := $r[C]
 */
void segmented_store(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $r[A] := ($r[B] + $r[C]) mod 2^32
 */
void addition(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $r[A] := ($r[B] × $r[C]) mod 2^32
 */
void multiplication(uint32_t instruction, UM_T my_UM);

/* 
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $r[A] := $r[B] ÷ $r[C]
 */
void division(uint32_t instruction, UM_T my_UM);

/* 
 * Takes in a piece of instruction, and perform instruction on my_UM
 * $r[A] := ¬($r[B] ∧ $r[C])
 */
void bitwise_NAND(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * halt the program
 */
void halt(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * A new segment is created with a number of
 * words equal to the value in $r[C]. Each word in
 * the new segment is initialized to 0. A bit pattern
 * that is not all zeroes and that does not identify
 * any currently mapped segment is placed in $r[B].
 * The new segment is mapped as $m[$r[B]].
 */
void map_segment(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * The segment $m[$r[C]] is unmapped. Future Map
 * Segment instructions may reuse the
 * identiﬁer $r[C].
 */
void unmap_segment(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * The value in $r[C] is displayed on the I/O device
 * immediately. Only values from 0 to 255 are
 * allowed.
 */
void output(uint32_t instruction, UM_T my_UM);

/* 
 * Takes in a piece of instruction, and perform instruction on my_UM
 * The universal machine waits for input on the
 * I/O device. When input arrives, $r[C] is loaded
 * with the input, which must be a value from
 * 0 to 255. If the end of input has been signaled,
 * then $r[C] is loaded with a full 32-bit word in
 * which every bit is 1.
 */
void input(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * Segment $m[$r[B]] is duplicated, and the
 * duplicate replaces $m[0], which is abandoned.
 * The program counter is set to point to
 * $m[0][$r[C]]. If $r[B] = 0, the load-program
 * operation is expected to be extremely quick.
 */
void load_program(uint32_t instruction, UM_T my_UM);

/*
 * Takes in a piece of instruction, and perform instruction on my_UM
 * the three bits immediately less signiﬁcant than opcode describe a single
 * register A. The remaining 25 bits
 * indicate a value, which is loaded into $r[A].
 */
void load_value(uint32_t instruction, UM_T my_UM);
#endif

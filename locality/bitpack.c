/* 
 * This is the bitpack implementation 
 * by Mingzhe Li and Mengtian Li, 10/15/14
 */

#include "bitpack.h"
#include "assert.h"
#include "except.h"
#include <stdio.h>

Except_T Bitpack_Overflow = { "Overflow packing bits"};

static const unsigned WORD_SIZE = 64; 

/* This function performs a left shift on unsigned bits. It checks if the
 * shift is within the scale of the bit length
 */
static inline uint64_t shift_left(uint64_t word, unsigned shift)
{
        assert(shift <= WORD_SIZE);
        if (shift == WORD_SIZE)
                return 0;
        else 
                return word << shift;
}

/* This function performs a right shift on unsigned bits. It checks if the
 * shift is within the scale of the bit length
 */
static inline uint64_t shift_right(uint64_t word, unsigned shift)
{
        assert(shift <= WORD_SIZE);
        if (shift == WORD_SIZE)
                return 0;
        else 
                return word >> shift;
}

/* This function performs a left shift on signed bits. It checks if the
 * shift is within the scale of the bit length
 */
static inline int64_t shift_lefts(int64_t word, unsigned shift)
{
        assert(shift <= 64);
        if (shift == WORD_SIZE)
                return 0;
        else 
                return word << shift;
}

/* This function performs a right shift on signed bits. It checks if the
 * shift is within the scale of the bit length
 */
static inline int64_t shift_rights(int64_t word, unsigned shift)
{
        assert(shift <= 64);
        if (shift == WORD_SIZE)
                return 0;
        else 
                return word >> shift;
}


/* This function checks if the given unsigned int can fit in bits of width */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        assert(width <= WORD_SIZE);
        if (width == 0 || shift_left(1, width) - 1 < n)
                return false;

        return true;
}

/* This function checks if the given signed int can fit in bits of width */
bool Bitpack_fitss(int64_t n, unsigned width)
{
        if (width <= 1)
                return false;

        uint64_t n_abs = (uint64_t)n;
        if (n < 0)
                n_abs = ~n;

        return Bitpack_fitsu(n_abs, width - 1);
}

/* The get functions return bits starting from lsb to length of width bits
 * We used an algorithm provided by Norman Ramsey to extract the bits,
 * which works on both signed and unsigned integers
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= WORD_SIZE && width + lsb <= WORD_SIZE);
        return shift_right(shift_left(word, WORD_SIZE - (lsb + width))
                           , (WORD_SIZE -width));

}

int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        assert(width <= WORD_SIZE && width + lsb <= WORD_SIZE);
        return shift_rights(shift_lefts(word, WORD_SIZE - (lsb + width))
                           , (WORD_SIZE -width));
}


/* This function sets value into the word from lsb to width length of bits
 * If the value cannot fit in, it raises the overflow exception
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb,
                      uint64_t value)
{
        assert(width <= WORD_SIZE && width + lsb <= WORD_SIZE);
        if (!Bitpack_fitsu(value, width))
                RAISE(Bitpack_Overflow);

        /* shifting the bits to the left first and then to the right
         * and flips the whole in the end makes a mask of width length of 
         * 0s at the position where new value is to be set
         */
        uint64_t ALL_ONE = ~0;
        uint64_t step1 = shift_left(ALL_ONE, WORD_SIZE - width);
        uint64_t step2 = shift_right(step1, WORD_SIZE - (width + lsb));
        uint64_t mask = ~step2;

        /* setting the new value */
        return (word & mask) | shift_left(value, lsb);
}

uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb,  
                      int64_t value)
{
        assert(width <= WORD_SIZE && width + lsb <= WORD_SIZE);
        if (!Bitpack_fitss(value, width)) 
                RAISE(Bitpack_Overflow);

        uint64_t ALL_ONE = ~0;
        uint64_t step1 = shift_left(ALL_ONE, WORD_SIZE - width);
        uint64_t step2 = shift_right(step1, WORD_SIZE - (width + lsb));
        uint64_t mask = ~step2;
        
        /* The only difference for setting signed value is that we
         * will have to make the new value to unsigned first
         */
        uint64_t uvalue =
            shift_right(shift_left(value, WORD_SIZE - width), 
                        WORD_SIZE - (width + lsb));
        
        return (word & mask) | uvalue;
}

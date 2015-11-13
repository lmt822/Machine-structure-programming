/*
 * Mengtian Li
 * 11/3/2014
 * segments.h
 */

#ifndef MEMSEG_H
#define MEMSEG_H

#include <stdint.h>
#include "uarray.h"
#include "assert.h"
#include <stdlib.h>
#include "seq.h"

typedef struct Memseg_T {
        Seq_T segments;
        int lowest_counter;
} *Memseg_T;

/*
 * Initializes each element of segments to NULL
 */
Memseg_T Memseg_new(int length);

/*
 * Store the value at segment index and word index
 */
void Memseg_store(Memseg_T mem, uint32_t word, uint32_t sg,
        uint32_t value);

/*
 * Load word from word position and specified segment
 * Return the word at the segment index and word index
 */
uint32_t Memseg_load(Memseg_T mem, uint32_t word, uint32_t sg);

/*
 * Add new segment to Memseg_T at lowest_counter with space word_counter
 * return the lowest counter of the memory
 */
uint32_t Memseg_map(Memseg_T mem, int word_counter);

/*
 * Set segment(at index sg)'s elements to NULL and free the segment
 */
void Memseg_unmap(Memseg_T mem, uint32_t sg);

/*
 * replace the old segment at index given with a copy of new segment at index
 * given
 */
void Memseg_replace(Memseg_T mem, uint32_t new_sg,
                uint32_t old_sg);

/*
 * Free all segments
 */
void Memseg_free(Memseg_T *mem);

/* additional helper func that protect secret */
int Memseg_length(Memseg_T mem, uint32_t sg);

/* additional helper func that protect secret */
uint32_t Memseg_lowest_counter(Memseg_T mem);
#endif

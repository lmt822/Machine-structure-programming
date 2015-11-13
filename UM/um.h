/*
 * Mengtian Li
 * 11/10/2014
 * um.h
 */

#ifndef UM_H
#define UM_H
#include <stdlib.h>
#include "uarray.h"
#include <stdio.h>
#include "memseg.h"
#include <bitpack.h>
#include "seq.h"


typedef struct UM_T { 
        Memseg_T mem;                       
        UArray_T registers;
        uint32_t program_counter; 
        unsigned program_length;                        
}* UM_T;

/*
 * Initialize a new UM program with a input file pointer. 
 * CRE if FILE is illegal
 */
UM_T new_UM(FILE* fp);

/*
 * Run the given UM code with instructions
 * CRE if my_UM is illegal
 */
void execute_UM(UM_T my_UM);

/*
 * Free the UM memory
 */
 void free_UM(UM_T *my_UM);
 #endif

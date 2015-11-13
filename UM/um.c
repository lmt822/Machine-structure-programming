/*
 * Mengtian Li
 * 11/11/2014
 * UM.c that implements UM.h
 */

#include "um.h"
#include <mem.h>
#include "instructions.h"

#define NUM_REGISTERS 8

const int SEQ_SIZE = 1000;

UM_T new_UM(FILE* fp) 
{
	/* read in instructions from fp pointer and store to a Seq_T */
	assert(fp != NULL);
	int c = fgetc(fp);
	Seq_T read_in = Seq_new(SEQ_SIZE);
	uint32_t *instruction;
	while (c != EOF) {
		instruction = malloc(sizeof(*instruction));
		*instruction = 0;
		*instruction = Bitpack_newu(*instruction, 8, 24, (uint64_t) c);
		for (int i = 16; i >= 0; i -= 8){
			c = fgetc(fp);
			assert(c != EOF);
			*instruction = Bitpack_newu(*instruction, 8, i, 
							(uint64_t) c);
		}
		Seq_addhi(read_in, instruction);
		c = fgetc(fp);
	}
	/* start build the UM */
	UM_T to_rtn = malloc(sizeof(*to_rtn));
	to_rtn->registers = UArray_new(NUM_REGISTERS, sizeof(uint32_t));
	uint32_t *temp_reg;
	for (int i = 0; i < NUM_REGISTERS; i++){
		temp_reg = (uint32_t *)UArray_at(to_rtn->registers, i);
		*temp_reg = 0;
	}
	to_rtn->mem = Memseg_new(SEQ_SIZE);
	to_rtn->program_counter = 0;
	/* start store the program at memseg 0 */
	int length = Seq_length(read_in);
	Memseg_map(to_rtn->mem, length);
	uint32_t to_store;
	for (int i = 0; i < length; i ++){
		to_store =  *((uint32_t *)(Seq_get(read_in, i)));
		Memseg_store(to_rtn->mem, i, 0, to_store);
	}
	to_rtn->program_length = Memseg_length(to_rtn->mem, 0);
	Seq_free(&read_in);
	free(instruction);	
	return to_rtn;
}
void execute_UM(UM_T my_UM)
{
	assert(my_UM != NULL);
	uint64_t op = 0;
	uint32_t instr;
	while (my_UM->program_counter < my_UM->program_length) {
	    	instr = Memseg_load(my_UM->mem, my_UM->program_counter, 0);
		(my_UM->program_counter)++;
		op = Bitpack_getu(instr, 4, 28);
		assert(op < 14);
		switch (op){
		        case 0: conditional_move(instr, my_UM);break;
			case 1: segmented_load(instr, my_UM);break;
			case 2: segmented_store(instr, my_UM);break;
			case 3: addition(instr, my_UM);break;
			case 4: multiplication(instr, my_UM);break;
			case 5: division(instr, my_UM);break;
			case 6: bitwise_NAND(instr, my_UM);break;
			case 7: return;
			case 8: map_segment(instr, my_UM);break;
			case 9: unmap_segment(instr, my_UM);break;
			case 10: output(instr, my_UM);break;
			case 11: input(instr, my_UM);break;
			case 12: load_program(instr, my_UM);break;
			case 13: load_value(instr, my_UM);break;
		}
	}
}

void free_UM(UM_T *my_UM)
{
	assert(*my_UM != NULL);
	Memseg_free(&((*my_UM)->mem));
	UArray_free(&((*my_UM)->registers));
	free(*my_UM);
}

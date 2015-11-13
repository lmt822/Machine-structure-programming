/*
 * Mengtian Li
 * 11/12/2014
 * instructions.c 
 * contains 14 instructions
 */
#include "instructions.h"

/*
 * Return register pointer for given instruction
 * inline for speed purpose
 */
static inline uint32_t* get_reg(UArray_T registers, uint32_t instruction, 
			int length, int lsb )
{
	uint64_t temp = Bitpack_getu(instruction, length, lsb);
	return (uint32_t *)UArray_at(registers, 
			(int)temp);
}

void conditional_move(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	if (*rC != 0)
		*rA = *rB;
}

void segmented_load(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = Memseg_load(my_UM->mem, *rC, *rB);
}

void segmented_store(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	Memseg_store(my_UM->mem, *rB, *rA, *rC);
}

void addition(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = (*rB + *rC);
}

void multiplication(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = ((*rB) * (*rC));	
}

void division(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	assert(*rC != 0);
	*rA = (*rB) / (*rC);
}

void bitwise_NAND(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = ~((*rB) & (*rC));
}

void halt(uint32_t instruction, UM_T my_UM)
{
	(void)instruction;
	(void)my_UM;
	return;
}

void map_segment(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rB = Memseg_map(my_UM->mem, *rC);
}

void unmap_segment(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	assert(*rC != 0);
	Memseg_unmap(my_UM->mem, *rC);
}

void output(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	assert(*rC < 256);
	putchar(*rC);
}

void input(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	int c = getchar();
	if (c == EOF)
		*rC = ~0;
	else{
		assert(c < 256 && c >= 0);
		*rC = c;
	}
}

void load_program(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	my_UM->program_counter = *rC;
	if (*rB != 0){
		my_UM->program_length = Memseg_length(my_UM->mem, *rB);
		Memseg_replace(my_UM->mem, *rB, 0);
	} else {
		return;
	}
}

void load_value(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 25);
	uint64_t value = Bitpack_getu(instruction, 25, 0);
	*rA = value;
}

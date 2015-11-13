
/*
 * Mengtian Li
 * 11/11/2014
 * main.c that execute UM
 * optimized
 */
#include <stdio.h>
#include <stdlib.h>
#include "inttypes.h"
#include <string.h>
#include <mem.h>
#include "seq.h"
typedef struct UArray_T *UArray_T;
struct UArray_T{
	int length;
	int size;
	char *elems;
};
inline void UArrayRep_init(UArray_T uarray, int length, int size, void *elems)
{
	uarray->length = length;
	uarray->size = size;
	uarray->elems = elems;
}
inline UArray_T UArray_new(int length, int size)
{
	UArray_T array = malloc(sizeof(array));
	UArrayRep_init(array, length, size, CALLOC(length, size));
	return array;
}
inline void *UArray_at(UArray_T uarray, int i)
{
	return uarray->elems + i*uarray->size;
}
inline int UArray_length(UArray_T uarray)
{
	return uarray->length;
}
inline UArray_T UArray_copy(UArray_T uarray, int length)
{
	UArray_T copy = UArray_new(length, uarray->size);
	if (copy->length >= uarray->length && uarray->length > 0) 
		memcpy(copy->elems, uarray->elems, 
                       uarray->length*uarray->size);
	else if (uarray->length > copy->length && copy->length > 0)
		memcpy(copy->elems, uarray->elems, copy->length*uarray->size);
	return copy;
}
//////////////////////////////////////////////////////////////
typedef struct Memseg_T {
        Seq_T segments;
        int lowest_counter;
} *Memseg_T;

inline Memseg_T Memseg_new(int length)
{
        Memseg_T to_rtn = malloc(sizeof(to_rtn));
        to_rtn->segments = Seq_new(100);
        to_rtn->lowest_counter = 0;
        for (int i = 0; i < length; i++) {
                Seq_addhi(to_rtn->segments, NULL);
        }
        return to_rtn;
}

inline void Memseg_store(Memseg_T mem, uint32_t word, uint32_t sg,
        uint32_t value)
{
        UArray_T to_store = (UArray_T)(Seq_get(mem->segments, (int)sg));
        uint32_t *temp = (uint32_t *) UArray_at(to_store, (int)word);
        *temp = value;
}

inline uint32_t Memseg_load(Memseg_T mem, uint32_t word, uint32_t sg)
{
        UArray_T to_load = (UArray_T) Seq_get(mem->segments, (int)sg);
        return (*((uint32_t *)(UArray_at(to_load, (int)word))));
}

inline uint32_t Memseg_map(Memseg_T mem, int word_counter)
{
        uint32_t to_rtn = mem->lowest_counter;
        UArray_T to_map = UArray_new(word_counter,
                                        sizeof(uint32_t));
        int length = Seq_length(mem->segments);
	uint32_t *tempword;
        int templength = UArray_length(to_map);

        for (int i = 0; i < templength; i++){
	        tempword = (uint32_t *)UArray_at(to_map, i);
                *tempword = 0;
        }

        Seq_put(mem->segments, mem->lowest_counter,to_map);

        for (int i = mem->lowest_counter; i < length; i++){
                if (Seq_get(mem->segments, i) == NULL){
                        mem->lowest_counter = i;
                        return to_rtn;
                }
        }
	mem->lowest_counter = length;
        Seq_addhi(mem->segments, NULL);
        return to_rtn;
}

inline void Memseg_unmap(Memseg_T mem, uint32_t sg)
{
        Seq_put(mem->segments, (int)sg, NULL);
        if(mem->lowest_counter > (int) sg)
                mem->lowest_counter = sg;
}

inline void Memseg_replace(Memseg_T mem, uint32_t new_sg, uint32_t old_sg)
{       
    
        UArray_T to_put = (UArray_T)Seq_get(mem->segments, (int)new_sg);
        UArray_T tmp_array = UArray_copy(to_put, UArray_length(to_put));
        Seq_put(mem->segments, old_sg, tmp_array);
}


inline int Memseg_length(Memseg_T mem, uint32_t sg)
{
    	return UArray_length((UArray_T)Seq_get(mem->segments, (int)sg));
}

inline uint32_t Memseg_lowest_counter(Memseg_T mem)
{
        return mem->lowest_counter;
}

typedef struct UM_T { 
        Memseg_T mem;                       
        UArray_T registers;
        uint32_t program_counter; 
        unsigned program_length;                        
}* UM_T;
static inline uint64_t shl(uint64_t word, unsigned bits)
{
        if (bits == 64)
                return 0;
        else
                return word << bits;
}

/*
 * shift R logical
 */
static inline uint64_t shr(uint64_t word, unsigned bits)
{
        if (bits == 64)
                return 0;
        else
                return word >> bits;
}
static inline uint64_t Bitpack_newu(uint64_t word, unsigned width, 
                                    unsigned lsb, uint64_t value)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        return shl(shr(word, hi), hi)                 /* high part */
                | shr(shl(word, 64 - lsb), 64 - lsb)  /* low part  */
                | (value << lsb);                     /* new part  */
}

static inline int64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        unsigned hi = lsb + width; /* one beyond the most significant bit */
        /* different type of right shift */
        return shr(shl(word, 64 - hi), 64 - width); 
}
static inline uint32_t* get_reg(UArray_T registers, uint32_t instruction, 
			int length, int lsb )
{
	uint64_t temp = Bitpack_getu(instruction, length, lsb);
	return (uint32_t *)UArray_at(registers, 
			(int)temp);
}

static inline void conditional_move(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	if (*rC != 0)
		*rA = *rB;
}

static inline void segmented_load(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = Memseg_load(my_UM->mem, *rC, *rB);
}

static inline void segmented_store(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	Memseg_store(my_UM->mem, *rB, *rA, *rC);
}

static inline void addition(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = (*rB + *rC);
}

static inline void multiplication(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = ((*rB) * (*rC));	
}

static inline void division(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = (*rB) / (*rC);
}

static inline void bitwise_NAND(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 6);
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rA = ~((*rB) & (*rC));
}

static inline void halt(uint32_t instruction, UM_T my_UM)
{
	(void)instruction;
	(void)my_UM;
	return;
}

static inline void map_segment(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rB = get_reg(my_UM->registers, instruction, 3, 3);
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	*rB = Memseg_map(my_UM->mem, *rC);
}

static inline void unmap_segment(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	Memseg_unmap(my_UM->mem, *rC);
}

static inline void output(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	putchar(*rC);
}

static inline void input(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rC = get_reg(my_UM->registers, instruction, 3, 0);
	int c = getchar();
	if (c == EOF)
		*rC = ~0;
	else{
		*rC = c;
	}
}

static inline void load_program(uint32_t instruction, UM_T my_UM)
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

static inline void load_value(uint32_t instruction, UM_T my_UM)
{
	uint32_t *rA = get_reg(my_UM->registers, instruction, 3, 25);
	uint64_t value = Bitpack_getu(instruction, 25, 0);
	*rA = value;
}

int main(int argc, char *argv[])
{
    	(void)argc;
	FILE* fp = fopen(argv[1], "rb");
	if(fp){
	        /* execute UM */
	int c = fgetc(fp);
	Seq_T read_in = Seq_new(1000);
	uint32_t *instruction = 0;
	while (c != EOF) {
		instruction = malloc(sizeof(*instruction));
		*instruction = 0;
		*instruction = Bitpack_newu(*instruction, 
				8, 24, (uint64_t) c);
		for (int i = 16; i >= 0; i -= 8){
			c = fgetc(fp);
			*instruction = Bitpack_newu(*instruction, 8, i, 
							(uint64_t) c);
		}
		Seq_addhi(read_in, instruction);
		c = fgetc(fp);
	}
	UM_T my_UM = malloc(sizeof(*my_UM));
	my_UM->registers = UArray_new(8, sizeof(uint32_t));
	uint32_t *temp_reg;
	for (int i = 0; i < 8; i++){
		temp_reg = (uint32_t *)UArray_at(my_UM->registers, i);
		*temp_reg = 0;
	}
	my_UM->mem = Memseg_new(1000);
	my_UM->program_counter = 0;
	/* start store the program at memseg 0 */
	int length = Seq_length(read_in);
	Memseg_map(my_UM->mem, length);
	uint32_t to_store;
	for (int i = 0; i < length; i ++){
		to_store =  *((uint32_t *)(Seq_get(read_in, i)));
		Memseg_store(my_UM->mem, i, 0, to_store);
	}
	my_UM->program_length = Memseg_length(my_UM->mem, 0);
	uint64_t op = 0;
	uint32_t instr;
	while (my_UM->program_counter < my_UM->program_length) {
	    	instr = Memseg_load(my_UM->mem, my_UM->program_counter, 0);
		(my_UM->program_counter)++;
		op = Bitpack_getu(instr, 4, 28);
		switch (op){
		        case 0: conditional_move(instr, my_UM);break;
			case 1: segmented_load(instr, my_UM);break;
			case 2: segmented_store(instr, my_UM);break;
			case 3: addition(instr, my_UM);break;
			case 4: multiplication(instr, my_UM);break;
			case 5: division(instr, my_UM);break;
			case 6: bitwise_NAND(instr, my_UM);break;
			case 7: exit(EXIT_SUCCESS);
			case 8: map_segment(instr, my_UM);break;
			case 9: unmap_segment(instr, my_UM);break;
			case 10: output(instr, my_UM);break;
			case 11: input(instr, my_UM);break;
			case 12: load_program(instr, my_UM);break;
			case 13: load_value(instr, my_UM);break;
		}
	}
       	} else {
		fprintf(stderr, "fail to open file.\n");
		fclose(fp);
		exit(1);
	}
	exit(0);
}

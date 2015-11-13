/*
 * Mengtian Li
 * 11/3/2014
 * segments.c
 */

#include "memseg.h"
#include <stdio.h>

Memseg_T Memseg_new(int length)
{
        assert(length > 0);
        Memseg_T to_rtn = malloc(sizeof(*to_rtn));
        to_rtn->segments = Seq_new(100);
        to_rtn->lowest_counter = 0;
        for (int i = 0; i < length; i++) {
                Seq_addhi(to_rtn->segments, NULL);
        }
        return to_rtn;
}

void Memseg_store(Memseg_T mem, uint32_t word, uint32_t sg,
        uint32_t value)
{
        assert((mem != NULL) && ((int)sg < Seq_length(mem->segments)));
        UArray_T to_store = (UArray_T)(Seq_get(mem->segments, (int)sg));
        assert((to_store) && ((int)word < UArray_length(to_store)));
        uint32_t *temp = (uint32_t *)UArray_at(to_store, (int) word);
        *temp = value;
}

uint32_t Memseg_load(Memseg_T mem, uint32_t word, uint32_t sg)
{
        assert((mem != NULL) && ((int)sg < Seq_length(mem->segments)));
        UArray_T to_load = (UArray_T) Seq_get(mem->segments, (int)sg);
        assert((to_load != NULL) && ((int)word < UArray_length(to_load)));
        return (*((uint32_t *)(UArray_at(to_load, (int)word))));
}

uint32_t Memseg_map(Memseg_T mem, int word_counter)
{
        assert((word_counter > 0) && (mem != NULL));
        uint32_t to_rtn = mem->lowest_counter;
        UArray_T to_map = UArray_new(word_counter,
                                        sizeof(uint32_t));
        int length = Seq_length(mem->segments);
	uint32_t *tempword;
        int templength = UArray_length(to_map);
	/* initialize new segment */
        for (int i = 0; i < templength; i++){
	        tempword = (uint32_t *)UArray_at(to_map, i);
                *tempword = 0;
        }

        Seq_put(mem->segments, mem->lowest_counter,to_map);
	/* set approiate lowest free counter */
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

void Memseg_unmap(Memseg_T mem, uint32_t sg)
{
        assert((mem != NULL) && ((int)sg < Seq_length(mem->segments)));
        UArray_T to_free = (UArray_T)Seq_put(mem->segments, (int)sg, NULL);
	UArray_free(&to_free);
        if(mem->lowest_counter > (int) sg)
                mem->lowest_counter = sg;
}

void Memseg_replace(Memseg_T mem, uint32_t new_sg, uint32_t old_sg)
{       
        assert((mem != NULL) 
                && ((int)new_sg < Seq_length(mem->segments))
                && ((int)old_sg < Seq_length(mem->segments)));
        UArray_T to_throw = (UArray_T)Seq_get(mem->segments, (int) old_sg);
        assert(to_throw);
        UArray_T to_put = (UArray_T)Seq_get(mem->segments, (int)new_sg);
        assert(to_put);
        UArray_T tmp_array = UArray_copy(to_put, UArray_length(to_put));
        Seq_put(mem->segments, old_sg, tmp_array);
        UArray_free(&to_throw);
}

void Memseg_free(Memseg_T *mem)
{
        assert(mem);
        UArray_T temp_free;
	int templength = Seq_length((*mem)->segments);
        for (int i = 0; i < templength; i++){
	        temp_free = Seq_get((*mem)->segments, i);
	        if (temp_free != NULL)
		        UArray_free(&temp_free);
        }
        Seq_free(&((*mem)->segments));
        free(*mem);
}

int Memseg_length(Memseg_T mem, uint32_t sg)
{
    	assert(mem && ((int)sg < Seq_length(mem->segments)));
    	return UArray_length((UArray_T)Seq_get(mem->segments, (int)sg));
}

uint32_t Memseg_lowest_counter(Memseg_T mem)
{
        assert(mem);
        return mem->lowest_counter;
}

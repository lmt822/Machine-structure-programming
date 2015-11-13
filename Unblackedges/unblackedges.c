/*
 * unblackedges
 * by Mingzhe Li and Mengtian Li, 9/22/14
 *
 * Summary: this program reads in a portable bitmap image and remove all the
 *	    black edges and print the plain text of that image
 *
 *
 */


#include <pnmrdr.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <except.h>
#include <math.h>
#include <set.h>
#include "mem.h"
#include "bit2.h"
#include "assert.h"
#include "stack.h"
#include "uarray.h"

typedef struct {
        int row;
        int col;
        int bit;
} bit_info;

Bit2_T read_in_data(FILE *input);
void build_map(Bit2_T toBuild, Pnmrdr_T reader);
void put_bit(int row, int col, Bit2_T a, int bit, void *cl);
void clear_black_edges(Bit2_T input_map);
void push_sided_black_bits(Stack_T fringe, Bit2_T input_map, 
			   int width,int height);
void push_to_stack(int row, int col, Stack_T fringe);
void dfs_clear(Stack_T fringe, Bit2_T visited, Bit2_T input_map);
void push_neibhors(Stack_T fringe, Bit2_T input_map,
		   bit_info *curr_bit, Bit2_T visited);
void pbm_write(FILE *fp, Bit2_T bit_map);
void print_bit(int row, int col, Bit2_T bit_map, int bit, void *cl);


int main(int argc, char* argv[])
{
    assert(argc <= 2);
    FILE *input;
        if (argc == 1) {
                input = stdin;
                assert(input != NULL);
        } else {
                input = fopen(argv[1], "rb");
		assert(input);                
        } 
        Bit2_T input_map = read_in_data(input);    
	clear_black_edges(input_map);
	pbm_write(input, input_map);
   
	if (input != stdin)
	    fclose(input);
        
	Bit2_free(&input_map);     
	exit(0);
}

/* 
 * Taken an input file and read it as pbm file. If the file is not a pbm file
 * assert a checked runtime error. The function returns a bitmap that
 * represents the pbm file 
 */
Bit2_T read_in_data(FILE *input) 
{
        TRY
        Pnmrdr_T reader = Pnmrdr_new(input);
        Pnmrdr_mapdata data = Pnmrdr_data (reader);
        assert(data.type == Pnmrdr_bit);
        /* Build the map */
        Bit2_T toRtn = Bit2_new(data.width, data.height);
        build_map(toRtn, reader);
        Pnmrdr_free(&reader);
        return toRtn;

	EXCEPT(Pnmrdr_Badformat)
	    assert(0);
	END_TRY;

	return NULL;
}
/*
 * Apply function for the mapping function that put a given bit into the 
 * give bitmap. If Pnmrdr_Count is not correct, raise a check runtime error
 */
void put_bit(int row, int col, Bit2_T a, int bit, void *cl) 
{
        (void) bit;
        Pnmrdr_T reader = cl;
	TRY
        int to_put = (int) Pnmrdr_get(reader);
        Bit2_put(a, row, col, to_put);

	EXCEPT(Pnmrdr_Count)
    		assert(0);
	END_TRY;
}
/*
 * Take a bitmap and a PNM reader as argument, build the map with data in the
 * reader by using row major function.
 */
void build_map(Bit2_T toBuild, Pnmrdr_T reader) 
{
        Bit2_map_row_major(toBuild, put_bit, reader);
}
/*
 * Take a bitmap as argument and clear all the black edges in that bitmap 
 * by using a DFS algorithm to find black edges and remove them all.
 */
void clear_black_edges(Bit2_T input_map) 
{
        Stack_T fringe = Stack_new();
        int width = Bit2_width(input_map);
        int height = Bit2_height(input_map);
        Bit2_T visited = Bit2_new(width, height);
        push_sided_black_bits(fringe, input_map, width, height);
        dfs_clear(fringe, visited, input_map);
        Stack_free(&fringe);
        Bit2_free(&visited);
}
/*
 * Initially, all black bits on edges of the bitmap are black edges. So push
 * all black bits on edges of the bitmap to the stack.
 */
void push_sided_black_bits(Stack_T fringe, Bit2_T input_map, int width, 
				int height)
{
        for (int i = 0; i < width; i ++) {
                if (Bit2_get(input_map, i, 0) == 1) {
                        push_to_stack(i, 0, fringe);
                }
                if (Bit2_get(input_map, i, height - 1) == 1) {
                        push_to_stack(i, height - 1, fringe);
                }
        }
        for (int i = 1; i < height - 1; i++) {
                if (Bit2_get(input_map, 0, i) == 1) {
                        push_to_stack(0, i, fringe);
                }
                if (Bit2_get(input_map, width - 1, i) == 1) {
                        push_to_stack(width - 1, i, fringe);
                }
        }
}
/*
 * Helper function to push a give black edge to the stack
 */
void push_to_stack(int row, int col, Stack_T fringe) 
{
        bit_info *to_push = malloc(sizeof(bit_info));
        to_push->row = row;
        to_push->col = col;
        to_push->bit = 1;
        Stack_push(fringe, to_push);
}
/*
 * Clear every black edges in the bitmap by DFS
 */
void dfs_clear(Stack_T fringe, Bit2_T visited, Bit2_T input_map) 
{
        while (!Stack_empty(fringe)) {
                bit_info *curr_bit = (bit_info*)Stack_pop(fringe);
                push_neibhors(fringe, input_map, curr_bit, visited);
                Bit2_put(visited, curr_bit->row, curr_bit->col, 1);
                Bit2_put(input_map, curr_bit->row, curr_bit->col, 0);
                free(curr_bit);
        }
}
/*
 * Helper function for dfs_clear. Given a blackedge bit, push its black 
 * neighbors to the stack
 */
void push_neibhors(Stack_T fringe, Bit2_T input_map, 
                        bit_info *curr_bit, Bit2_T visited) 
{
        int row = curr_bit->row;
        int col = curr_bit->col;
        if ((row + 1 < Bit2_width(input_map)) && 
                (Bit2_get(input_map, row + 1, col) == 1) && 
                (Bit2_get(visited, row + 1, col) == 0)) {
                push_to_stack(row + 1, col, fringe);
        }
        if ((row - 1 > 0) &&
                (Bit2_get(input_map, row - 1, col) == 1) &&
                (Bit2_get(visited, row - 1, col) == 0)) {
                push_to_stack(row - 1, col, fringe);
        }
        if ((col + 1 < Bit2_height(input_map)) &&
                (Bit2_get(input_map, row, col + 1)) &&
                (Bit2_get(visited, row, col + 1) == 0)) {
                push_to_stack(row, col + 1, fringe);
        }
        if ((col - 1 > 0) &&
                (Bit2_get(input_map, row, col - 1)) &&
                (Bit2_get(visited, row, col - 1) == 0)) {
                push_to_stack(row, col - 1, fringe);
        }
}
/*
 * Print the given bitmap as a plain text for pbm on stdout
 */
void pbm_write(FILE *fp, Bit2_T bit_map)
{
	(void)fp;
	printf("")
	printf("P1\n# image with unblacked edges\n%d %d\n",
	   Bit2_width(bit_map), Bit2_height(bit_map));
    
	Bit2_map_row_major(bit_map, print_bit, NULL);

}
/*
 * Apply function for print out the bits in bitmap by row major
 */
void print_bit(int row, int col, Bit2_T bit_map, int bit, void *cl)
{
    (void)col;
    (void)cl;
    printf("%d ", bit);
      
    if (row == Bit2_width(bit_map) - 1)
	printf("\n");
}

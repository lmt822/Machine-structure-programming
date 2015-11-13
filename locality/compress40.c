/*
 * This is the implementation for compress40
 * It reads from file, compresses the image or decompresses the 
 * COMP40 compressed format data, and prints the result to the stdout
 * 
 * It utilizes useful function from compute40 module and bitpack
 *
 * submitted by Mingzhe Li and Mengtian Li, 10/15/14
 */

#include "compress40.h"
#include "compute40.h"
#include "bitpack.h"

/* This function copies the RGB info from the source pixel to the target */
void copy(int i, int j, A2 array2, void *data, void *cl)
{
        (void)array2;
        (void)data;
        A2 orig = cl;
        assert(orig);
        
        A2Methods_T methods = uarray2_methods_blocked;
        
        *(Pnm_rgb)data = *(Pnm_rgb)methods->at(orig, i , j);

}

/* This function returns a pixel matrix that has been trimmed down to
 * even number of width and height
 */
A2 image_trimmer(Pnm_ppm image)
{
        assert(image);
        int w = image->width;
        int h = image->height;

        if (w % 2)
                w--;
        if (h % 2)
                h--;

        A2 result = image->methods->
                new_with_blocksize(w, h, sizeof(struct Pnm_rgb), BLOCKSIZE);

        image->methods->map_block_major(result, copy, image->pixels);

        image->width = w;
        image->height = h;

        image->methods->free(&image->pixels);

        return result;
}

/* This function takes in the pixel matrix where each cell has been
 * compressed into a 64-bit codeword, and prints the compressed file
 * in COMP40 Compressed image format to stdout
 */
void print_result(A2 result, A2Methods_T methods)
{
        assert(result);
        assert(methods);
        int width = methods->width(result) * 2;
        int height = methods->height(result) * 2;
        printf("COMP40 Compressed image format 2\n%u %u\n", width, height);
        
        for (int i = 0; i < width / 2; i++)
                for (int j = 0; j < height / 2; j++) {
                        uint64_t *tmp = methods->at(result, i, j);
                        putchar(Bitpack_getu(*tmp, BYTE_LENGTH, 24));
                        putchar(Bitpack_getu(*tmp, BYTE_LENGTH, 16));
                        putchar(Bitpack_getu(*tmp, BYTE_LENGTH, 8));
                        putchar(Bitpack_getu(*tmp, BYTE_LENGTH, 0));
                }
}

/* This function takes in the file and utilizes functions in compute40
 * module to perform the compression and outputing, and frees the memory
 */
void compress40(FILE *input)
{
        A2Methods_T methods = uarray2_methods_blocked;
        Pnm_ppm image = Pnm_ppmread(input, methods);

        assert(image);
        assert(image->width > 1 && image->height > 1);
        
        image->pixels = image_trimmer(image);
 

        A2 result = methods->new_with_blocksize(image->width / BLOCKSIZE,
                                                image->height / BLOCKSIZE, 
                                                sizeof(uint64_t), 1);
 
        methods->map_block_major(result, compress, image);
        
        assert(result);
        print_result(result, methods);

        methods->free(&result);
        FREE(result);
        Pnm_ppmfree(&image);
        
}


/* This function reads the codewords from file and puts them into 
 * each cell in the pixel map
 */
void read_compressed(int i, int j, A2 array2, void *data, void *cl)
{
        (void)i;
        (void)j;
        (void)array2;

        FILE *input = cl;
        assert(input);

        uint64_t pixel = 0;
        pixel = Bitpack_newu(pixel, BYTE_LENGTH, 24, getc(input));
        pixel = Bitpack_newu(pixel, BYTE_LENGTH, 16, getc(input));
        pixel = Bitpack_newu(pixel, BYTE_LENGTH, 8, getc(input));
        pixel = Bitpack_newu(pixel, BYTE_LENGTH, 0, getc(input));

        *(uint64_t*)data = pixel;
}


/* This function takes in the file, reads the header, utilizes
 * functions in compute40 module to perform the decompress, completes the
 * decompressed ppm, and calls ppmwrite() to write the image to the 
 * stdout */
void decompress40(FILE *input)
{
        unsigned height, width;
        int read = fscanf(input, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        int c = getc(input);
        assert(c == '\n');

        A2Methods_T methods = uarray2_methods_blocked;
        A2 array = methods->new_with_blocksize(width / 2, height / 2, 
                                               sizeof(uint64_t), 1);
        assert(array);
        A2 result = methods->new_with_blocksize(width, height,
                                                sizeof(struct Pnm_rgb), 2);
        assert(result);
        
        methods->map_block_major(array, read_compressed, input);
 
        methods->map_block_major(array, decompress, result);

        assert(result);

        struct Pnm_ppm pixmap = { .width = width, .height = height,
                                  .denominator = DENOMINATOR, .pixels = result,
                                  .methods = uarray2_methods_blocked}; 

        Pnm_ppmwrite(stdout, &pixmap);

        methods->free(&result);
        methods->free(&array);
}

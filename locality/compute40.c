#include "compute40.h"
#include "arith40.h"
#include "bitpack.h"

/* This function takes in RGB values of a pixel and the denominator,
 * and returns its video component format
 */
Component rgb_to_comp(Pnm_rgb pixel, float denominator)
{
        Component to_rtn;
        float r = pixel->red / denominator;
        float g = pixel->green / denominator;
        float b = pixel->blue / denominator;
        
        to_rtn.y = 0.299 * r + 0.587 * g + 0.114 * b;
        to_rtn.pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
        to_rtn.pr = 0.5 * r - 0.418688 * g - 0.081312 * b;

        return to_rtn;
}

/* This function takes in the cosine coefficients, pb, pr indices,
 * packs them into the bit codeword, and returns that word
 */
uint64_t pack(Cos_coefficient cc, unsigned pb_index, unsigned pr_index)
{
        uint64_t word = 0;

        word = Bitpack_newu(word, A_WIDTH, A_LSB, cc.a);
        word = Bitpack_news(word, B_WIDTH, B_LSB, cc.b);
        word = Bitpack_news(word, C_WIDTH, C_LSB, cc.c);
        word = Bitpack_news(word, D_WIDTH, D_LSB, cc.d);
        word = Bitpack_newu(word, BLUE_WIDTH, BLUE_LSB, pb_index);
        word = Bitpack_newu(word, RED_WIDTH, RED_LSB, pr_index);
 
        return word;
}

/* This function quantizes the b, c, d values to the scale */
signed quantize(float x)
{
        if (x > FLOAT_RANGE_UPP) 
                return SIGNED_RANGE_UPP;
        if (x < FLOAT_RANGE_LOW)
                return SIGNED_RANGE_LOW;
        return roundf(SIGNED_RANGE_UPP / FLOAT_RANGE_UPP * x);
}

/* This function performs the DCT; it returns the cosine coefficients */
Cos_coefficient DCT(UArray_T y_array)
{
        float y1 = *(float*)UArray_at(y_array, 0);
        float y2 = *(float*)UArray_at(y_array, 1);
        float y3 = *(float*)UArray_at(y_array, 2);
        float y4 = *(float*)UArray_at(y_array, 3);
        
        Cos_coefficient to_rtn;
        to_rtn.a = roundf((y4 + y3 + y2 + y1) / 4.0 * (pow(2, A_WIDTH) - 1));
        to_rtn.b = quantize((y4 + y3 - y2 - y1) / 4.0);
        to_rtn.c = quantize((y4 - y3 + y2 - y1) / 4.0);
        to_rtn.d = quantize((y4 - y3 - y2 + y1) / 4.0);

        return to_rtn;
}

/* This function performs the JPEG-like compression for each block of pixels.
 * It takes in the image, computes the average of Y, Pb, Pr values of 
 * a block of pixel, packs them into codewords, and puts them into the
 * matrix of decompressed data 
 */
void compress(int i, int j, A2 uarray2, void *data, void *cl)
{
        (void)uarray2;
        Pnm_ppm image = (Pnm_ppm)cl;
        assert(image);
        A2 pixels = image->pixels;
        unsigned denominator = image->denominator;
        A2Methods_T methods = uarray2_methods_blocked;
        
        int count = 0;
        float sum_pb = 0;
        float sum_pr = 0;
        UArray_T y_array = UArray_new(BLOCKSIZE * BLOCKSIZE, sizeof(float));
        for (int row = j * BLOCKSIZE; row < (j + 1) * BLOCKSIZE; row++)
                for (int col = i * BLOCKSIZE; col < (i + 1) * BLOCKSIZE; col++)
                {
                        Component temp = rgb_to_comp((Pnm_rgb)methods->
                                at(pixels, col, row),(float)denominator);

                        *(float*)UArray_at(y_array, count) = temp.y;
                        count++;
                        sum_pb = sum_pb + temp.pb;
                        sum_pr = sum_pr + temp.pr;
                }

        Cos_coefficient cc = DCT(y_array);
        unsigned pb_index = Arith40_index_of_chroma(sum_pb / count);
        unsigned pr_index = Arith40_index_of_chroma(sum_pr / count);

        uint64_t word = pack(cc, pb_index, pr_index);
 
        *(uint64_t*)data = word;

        UArray_free(&y_array);
}


/* This function unpacks the Y and Pb, Pr values from a codeword */
void unpack(uint64_t word, UArray_T *y_array, float *avg_pb, float *avg_pr)
{
        assert(y_array);
        assert(avg_pb);
        assert(avg_pr);

        float a = Bitpack_getu(word, A_WIDTH, A_LSB) / (pow(2, A_WIDTH) - 1);
        
        float b = Bitpack_gets(word, B_WIDTH, B_LSB) / 
                (SIGNED_RANGE_UPP / FLOAT_RANGE_UPP);
        
        float c = Bitpack_gets(word, C_WIDTH, C_LSB) / 
                (SIGNED_RANGE_UPP / FLOAT_RANGE_UPP);
        
        float d = Bitpack_gets(word, D_WIDTH, D_LSB) / 
                (SIGNED_RANGE_UPP / FLOAT_RANGE_UPP);
        
        unsigned pb_index = Bitpack_getu(word, BLUE_WIDTH, BLUE_LSB);
        unsigned pr_index = Bitpack_getu(word, RED_WIDTH, RED_LSB);

        DCT_inverse(*y_array, a, b, c, d);
        *avg_pb = Arith40_chroma_of_index(pb_index);
        *avg_pr = Arith40_chroma_of_index(pr_index);
}

/* This function performs the inverse DCT */
void DCT_inverse(UArray_T y_array, float a, float b, float c, float d)
{
        *(float*)UArray_at(y_array, 0) = a - b - c + d;
        *(float*)UArray_at(y_array, 1) = a - b + c - d;
        *(float*)UArray_at(y_array, 2) = a + b - c - d;
        *(float*)UArray_at(y_array, 3) = a + b + c + d;
}

/* This function makes calculated RGB values within the scale of
 * the denominator
 */ 
unsigned scale_to_int(float x)
{
        if (x < 0)
            return 0;

        unsigned to_rtn = roundf(x * DENOMINATOR);
        if (to_rtn > DENOMINATOR)
                return DENOMINATOR;

        return to_rtn;
}

/* This function takes in the video component format and returns 
 * the correct scaled RGB values
 */
struct Pnm_rgb comp_to_rgb(Component to_be_scaled)
{
        float y = to_be_scaled.y;
        float pb = to_be_scaled.pb;
        float pr = to_be_scaled.pr;

        struct Pnm_rgb to_rtn;
        to_rtn.red = scale_to_int(y + 1.402 *pr);
        to_rtn.green = scale_to_int(y - 0.344136 * pb - 0.714136 * pr);
        to_rtn.blue = scale_to_int(y + 1.772 * pb);

        return to_rtn;
}

/* This function performs the decompression.
 * It reads and unpacks data from each codeword, transforms it to RGB values,
 * and puts them into the result pixel map
 */
void decompress(int i, int j, A2 uarray2, void *data, void *cl)
{
        (void)uarray2;
        A2 result = cl;
        assert(result);
        A2Methods_T methods = uarray2_methods_blocked;

        UArray_T y_array = UArray_new(4, sizeof(float));
        uint64_t word = *(uint64_t*)data;
        assert(word);

        float pb;
        float pr;
        
        unpack(word, &y_array, &pb, &pr);
        
        int count = 0;
        for (int row = j * BLOCKSIZE; row < (j + 1) * BLOCKSIZE; row++)
                for (int col = i * BLOCKSIZE; col < (i + 1) * BLOCKSIZE; col++) 
                {       
                    float y = *(float*)UArray_at(y_array, count);

                    Component comp = {.y = y, .pb = pb, .pr = pr};

                    struct Pnm_rgb to_rgb = comp_to_rgb(comp);

                    *(Pnm_rgb)methods->at(result, col, row) = to_rgb;

                    count++;
                }

        UArray_free(&y_array);
        FREE(y_array);
}

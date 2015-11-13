#include <stdio.h>
#include <stdlib.h>
#include "pnm.h"
#include "uarray.h"
#include "uarray2b.h"
#include "a2methods.h"
#include "a2plain.h"
#include "a2blocked.h"
#include "mem.h"
#include "math.h"
#include "assert.h"

static const unsigned BYTE_LENGTH = 8;

/* These constant values are defined according to the assignment specs */
static const unsigned A_LSB = 26;
static const unsigned B_LSB = 20;
static const unsigned C_LSB = 14;
static const unsigned D_LSB = 8;
static const unsigned A_WIDTH = 6;
static const unsigned B_WIDTH= 6;
static const unsigned C_WIDTH = 6;
static const unsigned D_WIDTH= 6;
static const unsigned BLUE_LSB = 4;
static const unsigned RED_LSB = 0;
static const unsigned BLUE_WIDTH = 4;
static const unsigned RED_WIDTH = 4;

/* For quantization */
static const float FLOAT_RANGE_UPP = 0.3;
static const float FLOAT_RANGE_LOW = -0.3;
static const signed SIGNED_RANGE_UPP = 31;
static const signed SIGNED_RANGE_LOW = -31;

static const int BLOCKSIZE = 2;

/* The denominator for the decompressed image */
static const unsigned DENOMINATOR = 255;

typedef A2Methods_UArray2 A2;

/* This struct stores data in video component format */
typedef struct Component {
        float y;
        float pb;
        float pr;
} Component;

/* This struct stores the cosine coefficients */
typedef struct Cos_coefficient {
        unsigned a;
        signed b;
        signed c;
        signed d;
} Cos_coefficient;


Component rgb_to_comp(Pnm_rgb pixel, float denominator);

uint64_t pack(Cos_coefficient cc, unsigned pb_index, unsigned pr_index);

void compress(int i, int j, A2 uarray2, void *data, void *cl);

signed quantize(float x);

void unpack(uint64_t word, UArray_T *y_array, float *avg_pb, float *avg_pr);

Cos_coefficient DCT(UArray_T y_array);

void DCT_inverse(UArray_T y_array, float a, float b, float c, float d);

unsigned scale_to_int(float to_be_scaled);

struct Pnm_rgb comp_to_rgb(Component to_be_scaled);

void decompress(int i, int j, A2 uarray2, void *data, void *cl);

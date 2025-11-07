/**************************************************************
 *
 *                     ry_conversion.h
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     3/1/25
 *
 *     Summary:
 * 
 *     This header file defines functions and data structures for converting 
 *     between RGB and YPbPr color spaces, performing both compression and 
 *     decompression processes. 
 *     
 *
 **************************************************************/

#ifndef RY_CONVERSION
#define RY_CONVERSION

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include "mem.h"
#include <except.h>

#include <pnm.h>
#include <a2methods.h>
#include "a2plain.h"
#include "a2blocked.h"
#include "uarray2b.h"
#include "uarray2.h"

/* structs */
typedef struct closure closure;
typedef struct Y_Pb_Pr *Y_Pb_Pr;


/* compression functions */
A2Methods_UArray2 rgb_to_ypbpr (Pnm_ppm ppm);
void to_ypbpr_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl);

/* decompression functions */
A2Methods_UArray2 ypbpr_to_rgb(A2Methods_UArray2 ypbpr_pixels, 
        A2Methods_T methods, int denominator);
void to_rgb_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl);
void convert_ypbpr_to_rgb(Y_Pb_Pr ypbpr, int denominator, Pnm_rgb rgb);


/* getters, setters, size, and new */
Y_Pb_Pr new_Y_Pb_Pr(float Y, float Pb, float Pr);
int Y_Pb_Pr_size();
float getY(Y_Pb_Pr ypbpr);
float getPb(Y_Pb_Pr ypbpr);
float getPr(Y_Pb_Pr ypbpr);
void setY(Y_Pb_Pr ypbpr, float Y);
void setPb(Y_Pb_Pr ypbpr, float Pb);
void setPr(Y_Pb_Pr ypbpr, float Pr);


#endif
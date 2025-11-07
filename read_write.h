/**************************************************************
 *
 *                     read_write.h
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), 
 *     Date:     2/21/25
 *
 *     Summary:
 * 
 *     This file provides contains function declerations for read_write.c. 
 *     It also contains the function decleration for shift_left which is 
 *     defined in bitpack.c but could not be linked using bitpack.h because 
 *     we do not have access to that file.  
 * 
 *     
 *
 **************************************************************/
 #ifndef READ_WRITE
 #define READ_WRITE

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
#include "bitpack.h"

/* compression */
Pnm_ppm read_and_trim_ppm(FILE *input); 
void update_ppm_trimmed(Pnm_ppm *ppm, A2Methods_T methods, 
                        int width, int height);
void trimmed_pixels_apply(int colx, int rowy, A2Methods_UArray2 old_pixels, 
                        void *elem, void *cl);
void print_compressed(A2Methods_UArray2 words, A2Methods_T methods);

/* decompression */
void print_decompressed(A2Methods_UArray2 pixels, A2Methods_T methods, 
                        int denominator); 
A2Methods_UArray2 read_compressed_to_words(FILE *file);

/* bitpack.c shift */
uint64_t shift_left(uint64_t word, unsigned shift);

#endif
/**************************************************************
 *
 *                           word.h
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     3/1/25
 *
 *     Summary:
 * 
 *     This header file defines functions and data structures for processing 
 *     compressed image data at the word level. It includes functions for 
 *     creating, packing, and unpacking 32-bit words that store encoded 
 *     pixel information. 
 *     
 *
 **************************************************************/

#ifndef WORD
#define WORD

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
#include "arith40.h"
#include "ry_conversion.h"
#include "bitpack.h"

/*structs*/
typedef struct word *word;
typedef struct word_closure word_closure;


/*compression*/
A2Methods_UArray2 make_word_array(A2Methods_UArray2 pixels, 
                                        A2Methods_T methods);
void word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl);
void ypbpr_to_word(word w, Y_Pb_Pr ypbpr1, Y_Pb_Pr ypbpr2, Y_Pb_Pr ypbpr3, 
                        Y_Pb_Pr ypbpr4);
int quantize_bcd(float bcd);

A2Methods_UArray2 pack_word(A2Methods_UArray2 word_structs, 
                                        A2Methods_T methods);
void pack_word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
                        void *cl); 
uint32_t pack_single_word (word w);


/*decompression*/
A2Methods_UArray2 decompress_words(A2Methods_UArray2 words, 
                                        A2Methods_T methods);
void ypbpr_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
                        void *cl);
void word_to_ypbpr(word w, Y_Pb_Pr ypbpr1, Y_Pb_Pr ypbpr2, Y_Pb_Pr ypbpr3, 
                        Y_Pb_Pr ypbpr4);
void set_ypbpr(Y_Pb_Pr ypbpr, float Y, float Pb, float Pr);

A2Methods_UArray2 unpack_word(A2Methods_UArray2 word_bits, A2Methods_T methods);
void unpack_word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
                       void *cl);
void unpack_single_word(word w, uint32_t packed_word);

#endif
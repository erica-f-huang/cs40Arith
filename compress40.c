/**************************************************************
 *
 *                     compress40.c
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     2/21/25
 *
 *     Summary: implements provided compress40.h found at
 *              /comp/40/build/include/compress40.h
 *     
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "assert.h"
#include <except.h>
#include <a2methods.h>
#include <pnm.h>
#include <a2blocked.h>
#include <a2plain.h>
#include "uarray2b.h"
#include "uarray2.h"
#include <string.h>
#include "read_write.h"
#include "ry_conversion.h"
#include "word.h"

const int DENOM = 225; 

/********** compress40 ********
 * 
 * Purpose: Compresses a given PPM image and writes the compressed output to 
 *          standard output in binary format
 *
 * Parameters:
 *      - input: A file pointer to the .ppm file to be compressed
 *
 * Return: none
 *
 * Expects:
 *      - input is a valid open file pointer (not NULL)
 *      - The image follows the standard PPM format
 *
 * CRE: input is null, methods are null, ppm is null, the pixels of ppm is null,
 *      ypbpr_pixels is null, word_structs is null, and word_bits is null.
 *      More assert statements in the used functions
 *
 * Notes: 
 *      - Utilizes functions from read_write.h, ry_conversion.h, word.h
 *      - information is lost here, more specification in the headers of 
 *              functions used
 *      - memory is allocted and freed for each of the A2Methods_UArray2s
 *              defined in this function. Memory is allocated and freed for the
 *              ppm defined in this function.
 */
extern void compress40(FILE *input)
{
        assert(input != NULL);

        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* step 1 - create ppm from input*/
        Pnm_ppm ppm = read_and_trim_ppm(input); 
        assert(ppm != NULL); 
        assert(ppm->pixels != NULL);

        /* step 2 - RGB to Y/Pb/Pr values*/ 
        /*info is lost here due to floats*/
        A2Methods_UArray2 ypbpr_pixels = rgb_to_ypbpr(ppm); 
        assert(ypbpr_pixels != NULL);
        
        /*step 3 - turn into 2D array of word structs*/
        /*info is lost here due to averaging and compressing information*/
        A2Methods_UArray2 word_structs = make_word_array(ypbpr_pixels, methods);
        assert(word_structs != NULL);
        
        /*step 4 - create a 2D array of 32-bit words*/
        A2Methods_UArray2 word_bits = pack_word(word_structs, methods);
        assert(word_bits != NULL);

        /*step 5 - print compressed image*/
        print_compressed(word_bits, methods);
        
        /*step 6 - cleanup*/
        Pnm_ppmfree(&ppm);
        methods->free(&ypbpr_pixels);
        methods->free(&word_structs);
        methods->free(&word_bits);
}

/********** decompress40 ********
 * 
 * Purpose: decompresses the given compressed image and writes to 
 *          standard output 
 *
 * Parameters:
 *      input: the compressed file to decompress
 * 
 * Return: void
 *
 * Expects: 
 *      - input is a valid, open file pointer (not NULL)
 *      - A decompressed version of input written to standard output
 *      - The input file follows the "COMP40 Compressed image format 2" format
 *
 * CRE: input is null, methods is null, word_bits is null, word_structs is null,
 *      ypbpr_pixels is null, and pixels is null
 *
 * Notes: 
 *      - utilizes functions from read_write.h, ry_conversion.h, word.h
 *      - memory is allocted and freed for each of the A2Methods_UArray2s
 *              defined in this function. Memory is allocated and freed for the
 *              ppm defined in this function.
 */
extern void decompress40(FILE *input)
{
        assert(input != NULL);
        /* for rn testing of step 1 bc we havent made code to read in *input*/

        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /*step 1 - create 2D array of 32-bit words from input*/
        A2Methods_UArray2 word_bits = read_compressed_to_words(input);
        assert(word_bits != NULL);

        /*step 3 - turn into 2D array of word structs*/
        A2Methods_UArray2 word_structs = unpack_word(word_bits, methods);
        assert(word_structs != NULL);

        /*step 4 - unpack words into Y/Pb/Pr pixels*/
        A2Methods_UArray2 ypbpr_pixels = decompress_words(word_structs, 
                                                                methods);
        assert(ypbpr_pixels != NULL);
        
        /*step 5 - Y/Pb/Pr value to RGB values*/
        A2Methods_UArray2 pixels = ypbpr_to_rgb(ypbpr_pixels, 
                                                methods, DENOM);
        assert(pixels != NULL);
        
        /*step 6 - print decompressed image*/
        print_decompressed(pixels, methods, DENOM);

        /*step 7 - cleanup*/
        methods->free(&word_bits);
        methods->free(&word_structs);
        methods->free(&ypbpr_pixels);
        /*"pixels" freed in print_decompressed*/
}




/**************************************************************
 *
 *                     read_write.c
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     2/21/25
 *
 *     Summary:
 * 
 *     This file handles reading and writing images in both compressed and 
 *     uncompressed formats. It includes functions for trimming PPM images, 
 *     storing packed words, and reconstructing images from compressed data. 
 *     The file ensures correct formatting, efficient memory usage, and proper 
 *     handling of pixel and word data during compression and decompression.
 *     
 *
 **************************************************************/

#include "read_write.h"

/* 
 * struct trimmed_pixels_closure stores information needed for trimming an 
 * image, including a pointer to a 2D array of pixels, a set of methods for  
 * handling the array, and trimmed width and height variables. It is used in 
 * apply functions that modify images, ensuring correct access to pixel data
 * while maintaining the correct dimensions
 */
typedef struct {
        A2Methods_UArray2 *new_pixels;
        A2Methods_T methods;
        int trim_width;
        int trim_height;
} trimmed_pixels_closure;


/**************************/
/*       Compression      */
/**************************/


/********** read_and_trim_ppm ********
 * 
 * Reads a PPM image from a file and trims it to ensure its width 
 * and height are even numbers
 *
 * Parameters:
 *      - input: A pointer to an open file containing a PPM image
 *
 * Return:
 *      - A Pnm_ppm struct containing the image data, with trimmed 
 *        dimensions if necessary
 *     
 * Expects:
 *      - input is a valid and open PPM file stream (not NULL)
 *      - The file is formatted correctly as a PPM image
 *     
 * CRE: input is null, methods is null, or ppm is null
 *
 * Notes:
 *      - If trimming is required, the update_ppm_trimmed() function is called 
 *        to update the pixel data
 *      - Memory for the Pnm_ppm struct is dynamically allocated
 *      - Information is lost here if width or height is odd because we remove
 *              part of the image to get even dimensions
 */
Pnm_ppm read_and_trim_ppm(FILE *input)
{
        assert(input != NULL);

        /* Initialize the method for handling UArray2 operations */
        A2Methods_T methods = uarray2_methods_plain; 
        assert(methods != NULL);

        /* Read the ppm image onto a Pnm_ppm struct */
        Pnm_ppm ppm = Pnm_ppmread(input, methods);
        assert(ppm != NULL);

        /* Check the OG width and height - update if odd */
        unsigned width = ppm->width; 
        unsigned height = ppm->height; 
        if (width % 2 == 1) {
                width--; 
        } 
        if (height % 2 == 1) {
                height--; 
        }

        /* Trim the image if width or height was trimmed */
        if ((width != ppm->width) || (height != ppm->height)) {
                update_ppm_trimmed(&ppm, methods, width, height);
        }

        return ppm;
}

/********** update_ppm_trimmed ********
 *
 * Purpose:
 *    Updates the ppm struct to hold a new, trimmed pixel array with even 
 *    width and height
 *
 * Parameters:
 *    - ppm: A pointer to a Pnm_ppm struct that holds the image data
 *    - methods: The methods object for handling UArray2 operations
 *    - width: The new width after trimming (must be even)
 *    - height: The new height after trimming (must be even)
 *
 * Return: None 
 *
 * Expects:
 *    - ppm is a valid pointer to an allocated Pnm_ppm struct (not NULL)
 *    - methods is a valid methods object for UArray2 operations (not NULL)
 *    - width and height are both even and valid for the new pixel array
 *     
 * CRE: *ppm is null, ppm is null, pixels of ppm is null, methods is null,
 *      width or height is less than 0, new_pixels is null, map is null, the
 *      address of PaM is null, or the old_pixels is null 
 *
 * Notes:
 *    - The function dynamically allocates a new UArray2 for the trimmed pixels
 *      and ensures that the old pixel array is properly freed to avoid 
 *      memory leaks
 *    - The function calls trimmed_pixels_apply() via a mapping function to copy 
 *      valid pixels into the new trimmed UArray2
 *    - Information is lost here if width or height is odd because we remove
 *              part of the image to get even dimensions
 */
void update_ppm_trimmed(Pnm_ppm *ppm, A2Methods_T methods, 
                        int width, int height)
{
        assert(ppm != NULL);
        assert(*ppm != NULL);
        assert((*ppm)->pixels != NULL);
        assert(methods != NULL);
        assert(width >= 0);
        assert(height >= 0);
        
        /*define new 2D array to hold the trimmed pixels*/
        A2Methods_UArray2 new_pixels = methods->new(width, height, 
                methods->size((*ppm)->pixels));
        assert(new_pixels != NULL);

        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        /*create closure argument for map function*/
        trimmed_pixels_closure PaM = {&new_pixels, methods, 
        width != (int)(*ppm)->width, height != (int)(*ppm)->height};
        assert(&PaM != NULL);

        /*make trimmed A2_UA2*/
        map((*ppm)->pixels, trimmed_pixels_apply, &PaM);

        /*update parameters in pnm_ppm*/
        (*ppm)->width = width; 
        (*ppm)->height = height;

        /*update ppm so it contains the trimmed pixels*/
        A2Methods_UArray2 old_pixels = (*ppm)->pixels;
        assert(old_pixels != NULL);

        /* free old_pixel memory */
        methods->free(&old_pixels);

        /* replace old pixel array with trimmed version */
        (*ppm)->pixels = new_pixels;
}

/********** trimmed_pixels_apply ********
 * 
 * Copies pixel data from the original old_pixels UArray2 into a new new_pixels
 * UArray2, ensuring that any pixels that have been trimmed (off the edges) are
 * not included
 *
 * Parameters:
 *      - colx: The column index of the pixel in the original image
 *      - rowy: The row index of the pixel in the original image
 *      - old_pixels: The original image pixel array
 *      - elem: A pointer to the pixel data at (colx, rowy) in old_pixels
 *      - cl: A closure struct containing:
 *              - new_pixels: The trimmed pixel array
 *              - methods: Function pointers for array operations
 *              - int trim_width: The adjusted width for trimming
 *              - int trim_height: The adjusted height for trimming
 *    
 * Return: None  
 *
 * Expects:
 *    - old_pixels is valid (not NULL)
 *    - elem is a valid pointer to a pixel struct
 *    - cl is a valid pointer to a trimmed_pixels_closure struct
 *    - new_pixels within cl is a properly allocated UArray2
 *    - The image dimensions are even
 *
 * CRE: old_pixels is null, closure is null, new_pixels from closure is null,
 *      or methods from closure is null 
 *
 * Notes:
 *    - The function skips pixels on the rightmost and bottom edges if the
 *      width or height needs to be trimmed
 *    - The function assumes that trimming is only necessary if the original
 *      width or height is odd
 *    - Uses A2Methods_T function pointers to access and modify pixels in the
 *      UArray2 
 *    - Information is lost here if width or height is odd because we remove
 *              part of the image to get even dimensions
 */
void trimmed_pixels_apply(int colx, int rowy, A2Methods_UArray2 old_pixels, 
                          void *elem, void *cl)
{
        assert(old_pixels != NULL);

        /* get closure struct contining the trimmed pixel array */
        trimmed_pixels_closure *PaM = (trimmed_pixels_closure *)cl;
        assert(PaM != NULL); 

        /* get the new trimmed pixel array */
        A2Methods_UArray2 *new_pixels = PaM->new_pixels;
        assert(new_pixels != NULL);

        /* get methods for handling UArray2 */
        A2Methods_T methods = PaM->methods;
        assert(methods != NULL);

        /* get trimming info */
        int trim_width = PaM->trim_width;
        int trim_height = PaM->trim_height;

        /* Skip pixels on the right edge if width was trimmed */
        if ((trim_width != 0) && (colx == (methods->width(old_pixels) - 1))) {
                return;
        }
        /* Skip pixels on the bottom edge if height was trimmed*/
        if((trim_height != 0) && (rowy == (methods->height(old_pixels) - 1))) {
                return;
        }

        /*move value from untrimmed pixels to trimmed pixels*/
        Pnm_rgb new_val = methods->at(*new_pixels, colx, rowy);
        Pnm_rgb og_val = elem;
        new_val->red = og_val->red;
        new_val->green = og_val->green;
        new_val->blue = og_val->blue;
}

/********** print_compressed ********
 * 
 * Purpose: Writes a compressed image to standard output in binary format, 
 *          including a header and 32-bit packed words
 *
 * Parameters:
 *     - words: A 2D array containing 32-bit packed words
 *     - methods: Function pointers for handling UArray2 operations
 *
 * Return: None 
 *
 * Expects:
 *     - words is not NULL and contains valid packed words
 *     - methods is a valid A2Methods_T
 *
 * CRE: words is null, methods is null, or any word inside words is null
 *
 * Notes:
 *     - Prints the header in human-readable format
 *     - Writes each 32-bit word as four bytes in Big-Endian order
 *     - Uses Bitpack_getu() to extract 8-bit segments from each word
 */
void print_compressed(A2Methods_UArray2 words, A2Methods_T methods)
{
        assert(words != NULL);
        assert(methods != NULL);

        /* get dimensions of the compressed word array */
        int width = methods->width(words);
        int height = methods->height(words);

        /* print the compressed image header */
        printf("COMP40 Compressed image format 2\n%u %u", width, height);
        printf("\n");

        /* Iterate through the 2D array of 32-bit words in row-major */
        for(int row = 0; row < height; row++){
                for(int col = 0; col < width; col++){
                        
                        /* get the 32-bit compressed word */
                        uint32_t *word = methods->at(words, col, row);
                        assert(word != NULL);

                        /* extract and print each byte in big-endian order*/
                        for(int lsb = 24; lsb >= 0; lsb = lsb - 8){
                                uint64_t byte = Bitpack_getu(*word, 8, lsb);
                                
                                /* output byte as a char */
                                putchar(byte);
                        }
                }
        }
}


/****************************/
/*       Decompression      */
/****************************/


/********** print_decompressed ********
 * 
 * Purpose: Outputs an uncompressed PPM image to standard output
 *
 * Parameters:
 *     - pixels: A 2D array containing the RGB pixel data
 *     - methods: A function table for handling UArray2 operations
 *     - denominator: The maximum color value for scaling pixels
 *
 * Return: None 
 *
 * Expects:
 *     - pixels is not NULL and contains valid pixel data
 *     - methods is a valid A2Methods_T
 *     - denominator is a positive integer
 *
 * CRE: pixels is null, methods is null, denominator is less than or equal to 0,
 *      or the final_image is null
 *
 * Notes:
 *     - Creates a temporary Pnm_ppm struct for printing
 *     - Calls Pnm_ppmwrite() to write the image in PPM format
 *     - The final_image struct is dynamically allocated and freed properly
 */
void print_decompressed(A2Methods_UArray2 pixels, A2Methods_T methods, 
                        int denominator)
{
        assert(pixels != NULL);
        assert(methods != NULL); 
        assert(denominator > 0);
        
        /*create a ppm from the pixels*/
        Pnm_ppm final_image;
        NEW(final_image);
        assert(final_image != NULL);

        /* Set image properties using the pixel data */
        final_image->width = methods->width(pixels);
        final_image->height = methods->height(pixels);
        final_image->denominator = denominator;
        final_image->pixels = pixels;
        final_image->methods = methods; 

        /* Output the image */
        Pnm_ppmwrite(stdout, final_image);

        /* so don't free "pixels" in decmopress40() in compress40.c*/
        Pnm_ppmfree(&final_image);
}

/********** read_compressed_to_words ********
 * 
 * Purpose: Reads a compressed image file, extracts packed 32-bit words,  
 *          and stores them in a 2D array
 *
 * Parameters:
 *     - file: A pointer to the FILE stream containing the compressed image
 *
 * Return:
 *     - A newly allocated A2Methods_UArray2 containing 32-bit packed words
 *
 * Expects:
 *     - file is a valid, open file pointer (not NULL)
 *     - The file follows the "COMP40 Compressed image format 2" format
 *     - Width and height are properly specified in the header
 *
 * CRE: input is null, methods is null, or ppm is null
 *
 * Notes:
 *     - Reads the header to extract image dimensions
 *     - Allocates a 2D array to store packed words
 *     - Reads 4-byte words in Big-Endian order and stores them in the array
 *     - Uses getc() for reading individual bytes
 *     - Calls shift_left() to assemble 32-bit words correctly
 */
A2Methods_UArray2 read_compressed_to_words(FILE *file)
{
        assert(file != NULL);

        A2Methods_T methods = uarray2_methods_plain;
        assert(methods != NULL); 

       /* Step 1: read header*/
        unsigned height, width;
        int read = fscanf(file, "COMP40 Compressed image format 2\n%u %u", 
                          &width, &height);
        assert(read == 2);
        /*make sure last charac*/
        int c = getc(file);
        assert(c == '\n');

        /* Step 2: Allocate UArrray2 for packed words */
        A2Methods_UArray2 packed_words = methods->new(width, height, 
                                                      sizeof(uint32_t));
        assert(packed_words != NULL);

        /* Step 3: read and store words */
        for (unsigned row = 0; row < height; row++) {
                for (unsigned col = 0; col < width; col++) {
                        uint32_t word = 0; 
                        
                        /* Step 4: build word (4 bytes) (Big-Endian Order) */
                        for (int i = 0; i < 4; i++){
                                uint64_t byte = getc(file);
                                
                                /* check for end of file */
                                assert((int64_t)byte != EOF);

                                word = shift_left(word, 8);
                                word = (word | byte);
                        }
                        
                        /* store the packed word in UArray2 */
                        uint32_t *word_ptr = methods->at(packed_words, 
                                                        col, row);
                        assert(word_ptr != NULL); 
                        *word_ptr = word;
                }
        }

        return packed_words;
}
/**************************************************************
 *
 *                     word.c
 *
 *     Assignment: HW 4 Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     3/2/25
 *
 *     Summary:
 * 
 *     word.c handles the compression and decompression of image data 
 *     at the word level by packing and unpacking 32-bit words. It includes 
 *     functions for converting pixel data into compressed words (pack_word) 
 *     and reconstructing pixel values from these words (unpack_word).
 *     
 *
 **************************************************************/

#include "word.h"

/* 
 * struct word stores compressed image data, including averaged chroma values 
 * (Pb and Pr) and brightness values (a, b, c, d). Pb_avg and Pr_avg 
 * are 4-bit unsigned values, a is a 9-bit unsigned value, and b, c, and d are 
 * 5-bit signed values.
 */
struct word{
        /* Pb and Pr avg are 4 bits */
        unsigned int Pb_avg, Pr_avg;
        
        /* bcd are 5 bits, a is 9 */
        unsigned int a;
        signed int b, c, d;
};

/* 
 * struct word_closure holds information needed for processing words in an 
 * image, including a pointer to a 2D array of words and a set of methods 
 * for handling UArray2 operations.
 */
struct word_closure{
        A2Methods_UArray2 *pixels;
        A2Methods_T methods;
};

/* Constant least significant bit values */
const unsigned a_lsb = 23;
const unsigned b_lsb = 18;
const unsigned c_lsb = 13;
const unsigned d_lsb = 8;
const unsigned pb_avg_lsb = 4;
const unsigned pr_avg_lsb = 0;


/**************************/
/*       Compression      */
/**************************/


/********** make_word_array ********
 * 
 * Purpose: Takes 2D array of Y/Pb/Pr pixels and turns it into an array of words
 *
 * Parameters:
 *      pixels: the 2D array of Y/Pb/Pr pixels
 *      methods: the methods to use to manipulate the 2D array of Y/Pb/Pr pixels
 *              and the 2D array of words
 *
 *
 * Return: a 2D array of words
 *
 * Expects: the element type of pixels is type Y_Pb_Pr
 *
 * CREs: pixels is NULL, methods is NULL, the dimensions of pixels are odd, 
 *      the 2D array we create in the function is NULL, & the map function from
 *      the methods parameter is null
 *
 * Notes: 
 *      - uses a map function with word_apply() 
 *      - Information is lost here due to helper function ypbpr_to_word()
 */
A2Methods_UArray2 make_word_array(A2Methods_UArray2 pixels, A2Methods_T methods)
{
        assert(pixels != NULL);
        assert(methods != NULL);
        
        /* Retreive the dimensions of the input pixel array */
        int width = methods->width(pixels);
        int height = methods->height(pixels);
        
        /* ensure the width and height are even */
        assert((width % 2) == 0);
        assert((height % 2) == 0);
        
        /* create a new UArray2 to store word structs */
        A2Methods_UArray2 words = methods->new(width / 2, height / 2, 
                                               sizeof(struct word));
        assert(words != NULL);
        
        /* default mapping function */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        /* create a closure struct */
        word_closure cl = {&words, methods}; 

        /* apply the mapping function to convert pixels into words */
        map(pixels, word_apply, &cl);
        
        return words; 
}

/********** word_apply ********
 * 
 * Purpose: apply function that takes 4 Y/Pb/Pr pixels and turns it into a word 
 *
 * Parameters:
 *      col: the current pixel's column
 *      row: the current pixel's row
 *      array2: the current 2D array being iterated through
 *      elem: the current pixel
 *      cl: the closure argument (a struct that contains a pointer to a 2D
 *              array and A2Methods_T)       
 *
 * Return: none     
 *
 * Expects: The 2D array in the closure argument gets updated as the function
 *      gets mapped over the 2D array of Y/Pb/Pr pixels. the 2D array in the 
 *      closure argument is a 2D array of      
 *
 * CREs: closure argument is null, the 2D array in the closure argument is null,
 *      the methods in the closure argument is null
 *
 * Notes: 
 *      - uses ypbpr_to_word()    
 *      - Information is lost here due to helper function ypbpr_to_word()
 */
void word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl)
{
        assert(array2 != NULL);

        /* only go to the top left pixel of every 2x2 block */
        if((col % 2 == 1) || (row % 2 == 1)){
                return;
        }

        /* extract closure struct */
        word_closure *closure = (word_closure *)cl; 
        assert(closure != NULL);
        
        /* retrieve the destination word array */
        A2Methods_UArray2 *word_pixels = closure->pixels;
        assert(word_pixels !=NULL);
        assert(*word_pixels != NULL);
        
        /* retrieve methods */
        A2Methods_T methods = closure->methods;
        assert(methods != NULL);
        
        /* retrieve the 4 Y/Pb/Pr pixels forming the 2x2 block */
        Y_Pb_Pr ypbpr1 = elem;
        Y_Pb_Pr ypbpr2 = methods->at(array2, col + 1, row);
        Y_Pb_Pr ypbpr3 = methods->at(array2, col, row + 1);
        Y_Pb_Pr ypbpr4 = methods->at(array2, col + 1, row + 1);

        /* store the compressed word at the correct index */
        word curr_word = methods->at(*word_pixels, col / 2, row / 2);
        ypbpr_to_word(curr_word, ypbpr1, ypbpr2, ypbpr3, ypbpr4); 
}

/********** ypbpr_to_word ********
 * 
 * Purpose: Takes 4 Y_Pb_Pr structs and turns it into a word struct
 *
 * Parameters:
 *      w: the word to fill
 *      ypbpr1-4: the 4 Y_Pb_Pr structs to turn into a word   
 *
 * Return: none    
 *
 * Expects: none    
 *
 * CREs: w is null, ypbpr1-4 is null
 *
 * Notes: 
 *      - uses getPb(), getPr(), Arith40_index_of_chroma(), getY(), 
 *        quantize_bcd()   
 *      - Information is lost here in the conversion of Y/Pb/Pr pixel to word.
 *              Values are quantized, and floating point arithmetic is used.
 *              Quantization occurs directly and through quantize_bcd().    
 */
void ypbpr_to_word(word w, Y_Pb_Pr ypbpr1, Y_Pb_Pr ypbpr2, Y_Pb_Pr ypbpr3, 
                   Y_Pb_Pr ypbpr4)
{
        assert(w != NULL);
        assert(ypbpr1 != NULL);
        assert(ypbpr2 != NULL);
        assert(ypbpr3 != NULL);
        assert(ypbpr4 != NULL);
        
        /* compute the average Pb and Pr values for the 2x2 block */
        float Pb_avg_float = (getPb(ypbpr1) + getPb(ypbpr2) + getPb(ypbpr3) + 
                              getPb(ypbpr4)) / 4.0;
        float Pr_avg_float = (getPr(ypbpr1) + getPr(ypbpr2) + getPr(ypbpr3) + 
                              getPr(ypbpr4)) / 4.0;

        /* Quantize Pb and Pr to 4-bit values */
        w->Pb_avg = Arith40_index_of_chroma(Pb_avg_float);
        w->Pr_avg = Arith40_index_of_chroma(Pr_avg_float);

        /* Extract Y values from each pixel */
        float Y1 = getY(ypbpr1);
        float Y2 = getY(ypbpr2);
        float Y3 = getY(ypbpr3);
        float Y4 = getY(ypbpr4);

        /* Compute DCT */
        float a_float = (Y4 + Y3 + Y2 + Y1) / 4.0;
        float b_float = (Y4 + Y3 - Y2 - Y1) / 4.0;
        float c_float = (Y4 - Y3 + Y2 - Y1) / 4.0;
        float d_float = (Y4 - Y3 - Y2 + Y1) / 4.0;

        /* Scale and quantize DCT */
        w->a = (unsigned int)(a_float * 511);
        /* If more than 9 bits bring back to 9 bits */
        if (w->a > 511) {
                w->a = 511;
        }
        
        w->b = quantize_bcd(b_float); 
        w->c = quantize_bcd(c_float);
        w->d = quantize_bcd(d_float);
}

/********** quantize_bcd ********
 * 
 * Purpose: Converts a floating-point bcd value into a quantized 5-bit signed 
 *          integer ranging from -15 to 15 for compression
 *
 * Parameters:
 *     - bcd: A floating-point value representing b, c, or d
 *
 * Return:
 *     - A signed integer between -15 and 15 representing the quantized value
 *
 * Expects:
 *     - bcd is a valid floating-point number
 *
 * CREs: none
 *
 * Notes:
 *     - Values outside the range of -0.3 to 0.3 are clamped to the nearest 
 *       boundary
 *     - Rounding down is used to prevent adding information during compression
 *      - Information is lost here due to rounding and quantizing
 */
int quantize_bcd(float bcd)
{
        /* clamp the input value to the correct range [-0.3, 0.3] */
        float val = bcd;
        if (bcd < -0.3){
                val = -0.3;
        }        
        if(bcd > 0.3){
                val = 0.3;
        }

        /* scale and convert to integer (range -15 to 15)*/
        signed int bucket = val * 50;

        return bucket;
}

/********** pack_word ********
 * 
 * Purpose: Packs a 2D array of word structs into 32-bit compressed words 
 *          and stores them in a new UArray2
 *
 * Parameters:
 *     - word_structs: A 2D array containing word structs with encoded 
 *                     pixel data
 *     - methods: Function pointers for handling UArray2 operations
 *
 * Return: A new A2Methods_UArray2 containing packed 32-bit words
 *
 * Expects:
 *     - word_structs is not NULL and contains valid word data
 *     - methods is a valid A2Methods_T
 *
 * CREs: word_structs is null, methods is null, word_bits is null, or map is
 *      null
 *
 * Notes:
 *     - Uses pack_word_apply() to iterate over word_structs and pack each word
 *     - The resulting array contains 4-byte (32-bit) packed words in 
 *       Big-Endian order
 */
A2Methods_UArray2 pack_word(A2Methods_UArray2 word_structs, A2Methods_T methods)
{
        assert(word_structs != NULL); 
        assert(methods != NULL);

        /* Retrieve dimensions of the word array */
        int width = methods->width(word_structs);
        int height = methods ->height(word_structs);

        /* Allocate a new UArray2 for storing 32-bit words */
        A2Methods_UArray2 word_bits = methods->new(width, height, 4);
        assert(word_bits != NULL);

        /* Create closure for storing word array */
        word_closure cl = {&word_bits, methods};

        /* Apply packing function to eqch word struct */
        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);
        map(word_structs, pack_word_apply, &cl);

        return word_bits;
}

/********** pack_word_apply ********
 * 
 * Purpose: Apply function for pack_word() that packs a single word struct into 
 *          a 32-bit integer and stores it in the output UArray2
 *
 * Parameters:
 *     - col: The column index of the word being processed
 *     - row: The row index of the word being processed
 *     - array2: The original 2D array of word struct
 *     - elem: A pointer to the current word struct being processed
 *     - cl: A closure containing the output UArray2 and A2Methods_T functions
 *
 * Return: None
 *
 * Expects:
 *     - array2 is a valid UArray2
 *     - elem is a valid pointer to a word struct
 *     - cl is a valid pointer to a word_closure struct
 *
 * CREs: closure is null, word_bits from closure is null, pointer to word_bits
 *      is null, methods from closure is null, current element is null, or the
 *      bit_word from word_bits is null
 *
 * Notes:
 *     - Calls pack_single_word() to convert the word struct into a 32-bit value
 *     - Uses methods->at() to store the packed word in the correct location
 */
void pack_word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl)
{
        /* unused parameter */
        (void)array2;
        
        /* extract closure strcut containing word storage */
        word_closure *closure = (word_closure *)cl;
        assert(closure != NULL);

        /* retrieve the output UArray2 for storing packed words */
        A2Methods_UArray2 *word_bits = closure->pixels;
        assert(word_bits != NULL);
        assert(*word_bits != NULL);

        /* retrieve methods */
        A2Methods_T methods = closure->methods;
        assert(methods != NULL);

        /* retrieve the word struct containg pixel data */
        word w = elem;
        assert(w != NULL);

        /* retrieve the location the packed word array */
        uint32_t *bit_word = methods->at(*word_bits, col, row);
        assert(bit_word != NULL);
        
        /* pack the word struct into a 32-bit compression */
        *bit_word = pack_single_word(w);
}

/********** pack_single_word ********
 * 
 * Purpose: Converts a word struct into a 32-bit compressed word by 
 *          encoding its components using bit-packing functions
 *
 * Parameters:
 *     - w: A pointer to the word struct to be packed
 *
 * Return:
 *     - A 32-bit packed word containing the compressed data
 *
 * Expects:
 *     - w is not NULL and contains valid values for a, b, c, d, Pb_avg, and 
 *       Pr_avg
 *
 * CRE: input is null
 *
 *
 * Notes:
 *     - Uses Bitpack_newu() and Bitpack_news() to insert each field
 *     - Ensures correct LSB positions for each component
 *     - Stores data in Big-Endian order to maintain compatibility
 */
uint32_t pack_single_word (word w)
{
        assert(w != NULL);

        uint32_t packed_word = 0;

        /* pack unsigned a */
        packed_word = Bitpack_newu(packed_word, 9, a_lsb, w->a);  
        
        /* pack signed b */
        packed_word = Bitpack_news(packed_word, 5, b_lsb, w->b);

        /* pack signed c */
        packed_word = Bitpack_news(packed_word, 5, c_lsb, w->c);

        /* pack signed d */
        packed_word = Bitpack_news(packed_word, 5, d_lsb, w->d);

        /* pack unsigned Pb */
        packed_word = Bitpack_newu(packed_word, 4, pb_avg_lsb, w->Pb_avg);
        
        /* pack unsigned Pr */
        packed_word = Bitpack_newu(packed_word, 4, pr_avg_lsb, w->Pr_avg);
        
        return packed_word;
}


/****************************/
/*       Decompression      */
/****************************/


/********** decompress_words ********
 * 
 * Purpose: Converts a 2D array of compressed words into a 2D array of 
 *          Y_Pb_Pr pixels
 *
 * Parameters:
 *     - words: A 2D array containing packed 32-bit words
 *     - methods: Function table for handling UArray2 operations
 *
 * Return: A new 2D array containing Y_Pb_Pr pixel values
 *
 * Expects:
 *     - words is not NULL and contains valid packed words
 *     - methods is a valid A2Methods_T
 *
 * CREs: words is null, methods is null, ypbpr_pixels is null, or map is null 
 *
 * Notes:
 *     - The output image has twice the width and height of the input 
 *       compressed words
 *     - Uses ypbpr_apply() to unpack words into Y_Pb_Pr values
 *     - Information is lost here due to floating point arithmetic in 
 *              helper function word_to_ypbpr()
 */
A2Methods_UArray2 decompress_words(A2Methods_UArray2 words, A2Methods_T methods)
{
        assert(words != NULL);
        assert(methods != NULL);
        
        /*decompressed image is double width & height of compressed word array*/
        int width = methods->width(words) * 2 ;
        int height = methods->height(words) * 2 ;
        
        /* Allocate a UArray2 for storing decompressed Y/Pb/Pr pixels */
        A2Methods_UArray2 ypbpr_pixels = methods->new(width, height, 
                                                      Y_Pb_Pr_size());
        assert(ypbpr_pixels != NULL);
        
        /* retrieve the mapping function */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        /* create closure */
        word_closure cl = {&ypbpr_pixels, methods}; 

        /* apply the decompression function to each compressed word */
        map(words, ypbpr_apply, &cl);

        return ypbpr_pixels;
}

/********** ypbpr_apply ********
 * 
 * Purpose: Extracts a 32-bit compressed word and converts it into four 
 *          Y_Pb_Pr pixels, placing them in the correct locations within the 
 *          output image
 *
 * Parameters:
 *     - col: The column index of the compressed word in the words array
 *     - row: The row index of the compressed word in the words array
 *     - array2: The 2D array of compressed words
 *     - elem: A pointer to the packed word being processed
 *     - cl: A closure containing the output pixel array and methods
 *
 * Return: None 
 *
 * Expects:
 *     - array2 is a valid A2Methods_UArray2 object
 *     - elem is a valid pointer to a packed word
 *     - cl is a valid pointer to a word_closure struct
 *
 * CREs: closure is null, ypbpr_pixels from closure is null, pointer to 
 *      ypbpr_pixels is null, methods from closure is null, or current element
 *      is null
 *
 * Notes:
 *     - Uses word_to_ypbpr() to extract Y_Pb_Pr values and distribute them
 *     - Expands each compressed word into a 2x2 block of pixel
 *     - Information is lost here due to floating point arithmetic in 
 *              helper function word_to_ypbpr()
 */
void ypbpr_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl)
{
        (void)array2; 
        
        /* extract closure struct */
        word_closure *closure = (word_closure *)cl; 
        assert(closure != NULL);
        
        /* retrieve the output Y/Pb/Pr pixel array */
        A2Methods_UArray2 *ypbpr_pixels = closure->pixels;
        assert(ypbpr_pixels != NULL);
        assert(*ypbpr_pixels != NULL);
        
        /* retrieve methods */
        A2Methods_T methods = closure->methods;
        assert(methods != NULL);

        /* retrieve the compressed word */
        word w = elem;
        assert(w != NULL);

        /* Compute corresponding pixel locations in the decompressed image */
        int ypbpr_col = col * 2;
        int ypbpr_row = row * 2;
        Y_Pb_Pr ypbpr_val1 = methods->at(*ypbpr_pixels, ypbpr_col, ypbpr_row);
        Y_Pb_Pr ypbpr_val2 = methods->at(*ypbpr_pixels, ypbpr_col + 1, 
                                         ypbpr_row);        
        Y_Pb_Pr ypbpr_val3 = methods->at(*ypbpr_pixels, ypbpr_col, 
                                         ypbpr_row + 1);
        Y_Pb_Pr ypbpr_val4 = methods->at(*ypbpr_pixels, ypbpr_col + 1, ypbpr_row 
                                         + 1);

        /* Convert the word back into four Y/Pb/Pr pixels */
        word_to_ypbpr(w, ypbpr_val1, ypbpr_val2, ypbpr_val3, ypbpr_val4);
}

/********** word_to_ypbpr ********
 * 
 * Purpose: Converts a compressed word into four Y_Pb_Pr pixel values
 *
 * Parameters:
 *     - w: A pointer to the compressed word struct
 *     - ypbpr1: A pointer to the Y_Pb_Pr struct for the top-left pixel
 *     - ypbpr2: A pointer to the Y_Pb_Pr struct for the top-right pixel
 *     - ypbpr3: A pointer to the Y_Pb_Pr struct for the bottom-left pixel
 *     - ypbpr4: A pointer to the Y_Pb_Pr struct for the bottom-right pixel
 *
 * Return: None 
 *
 * Expects:
 *     - w is a valid pointer to a word struct
 *     - ypbpr1, ypbpr2, ypbpr3, and ypbpr4 are valid pointers
 *
 * CREs: w is null, ypbpr1-4 is null
 *
 * Notes:
 *     - Converts Pb and Pr indices to floating-point values using 
 *       Arith40_chroma_of_index
 *     - Uses DCT to reconstruct Y values
 *     - The computed Y values are stored in the respective Y_Pb_Pr structs
 *     - Information is lost here due to floating point arithmetic
 */
void word_to_ypbpr(word w, Y_Pb_Pr ypbpr1, Y_Pb_Pr ypbpr2, Y_Pb_Pr ypbpr3, 
                   Y_Pb_Pr ypbpr4)
{
        assert(w != NULL);
        assert(ypbpr1 != NULL);
        assert(ypbpr2 != NULL);
        assert(ypbpr3 != NULL);
        assert(ypbpr4 != NULL);
        
        /* Convert quantized Pb and Pr back to floating-point */
        float Pb_avg = Arith40_chroma_of_index(w->Pb_avg);
        float Pr_avg = Arith40_chroma_of_index(w->Pr_avg);

        /* decode DCT */
        float a = w->a / 511.0;
        float b = w->b / 50.0;
        float c = w->c / 50.0;
        float d = w->d / 50.0;

        /* reconstruct the four Y values from DCT */
        float Y1 = a - b - c + d;
        float Y2 = a - b + c - d;
        float Y3 = a + b - c - d;
        float Y4 = a + b + c + d;  

        /* Store the computed Y/Pb/Pr values in the correct pixels */
        set_ypbpr(ypbpr1, Y1, Pb_avg, Pr_avg);
        set_ypbpr(ypbpr2, Y2, Pb_avg, Pr_avg);
        set_ypbpr(ypbpr3, Y3, Pb_avg, Pr_avg);
        set_ypbpr(ypbpr4, Y4, Pb_avg, Pr_avg);
}

/********** unpack_word ********
 * 
 * Purpose: Converts a 2D array of 32-bit packed words into a 2D array of 
 *          word structs
 *
 * Parameters:
 *     - word_bits: A 2D array containing packed 32-bit words
 *     - methods: Function table for handling UArray2 operations
 *
 * Return:
 *     - A new 2D array containing decompressed word structs
 *
 * Expects:
 *     - word_bits is not NULL and contains valid packed words
 *     - methods is a valid A2Methods_T
 *
 * CREs: word_bits is null, methods is null, word_structs is null, 
 *      or map is null
 *
 * Notes:
 *     - Uses unpack_word_apply() to extract and store word struct values
 *     - The resulting array contains decompressed word structs with 
 *       Y/Pb/Pr data
 */
A2Methods_UArray2 unpack_word(A2Methods_UArray2 word_bits, A2Methods_T methods)
{
        assert(word_bits != NULL);
        assert(methods != NULL);

        /* Retrieve dimensions of the packed word array */
        int width = methods->width(word_bits);
        int height = methods ->height(word_bits);

        /* Allocate UArray2 for storing unpacked word structs */
        A2Methods_UArray2 word_structs = methods->new(width, height, 
                                                      sizeof(struct word));
        assert(word_structs != NULL);

        /* create closure for mapping function */
        word_closure cl = {&word_structs, methods};

        
        A2Methods_mapfun *map = methods->map_default;
        assert(map != NULL);

        /* apply the function to unpack each compressed word */
        map(word_bits, unpack_word_apply, &cl);

        return word_structs;
}

/********** unpack_word_apply ********
 * 
 * Purpose: Extracts a 32-bit packed word and converts it into a word struct,
 *          storing the result in the output array.
 *
 * Parameters:
 *     - col: The column index of the word in the words array
 *     - row: The row index of the word in the words array
 *     - array2: The 2D array of packed words
 *     - elem: A pointer to the packed 32-bit word being processed
 *     - cl: A closure containing the output word struct array and methods
 *
 * Return: None 
 *
 * Expects:
 *     - array2 is a valid A2Methods_UArray2 object
 *     - elem is a valid pointer to a packed 32-bit word
 *     - cl is a valid pointer to a word_closure struct
 *
 * CREs: closure is null, word_structs from closure is null, pointer to 
 *      word_structs is null, methods from closure is null, current element is 
 *      null, or the word from word_structs is null
 *
 * Notes:
 *     - Calls unpack_single_word() to extract values from the packed word
 *     - Stores the extracted values in the appropriate location in 
 *       word_structs
 */
void unpack_word_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
        void *cl)
{
        /* unused parameter */
        (void)array2;
        
        /* Extract the closure struct  */
        word_closure *closure = (word_closure *)cl;
        assert(closure != NULL);

        /* retrieve the output word struct array */
        A2Methods_UArray2 *word_structs = closure->pixels;
        assert(word_structs != NULL);
        assert(*word_structs != NULL);

        /* retrieve methods */
        A2Methods_T methods = closure->methods;
        assert(methods != NULL);

        /* retrieve the compressed 32-bit word */
        uint32_t *bit_word = elem;
        assert(bit_word != NULL);

        /* retrieve the corresponding word struct loactaion */
        word w = methods->at(*word_structs, col, row);
        assert(w != NULL);

        /* Unpack the compressed word into its individual components */
        unpack_single_word(w, *bit_word);
}

/********** unpack_single_word ********
 * 
 * Purpose: Converts a 32-bit packed word into its individual components and
 *          stores them in a word struct
 *
 * Parameters:
 *     - w: A pointer to the word struct to store the unpacked values
 *     - packed_word: The 32-bit packed word containing compressed image data
 *
 * Return: None 
 *
 * Expects:
 *     - w is a valid pointer to a word struct
 *
 * CREs: w is null
 * 
 * Notes:
 *     - Uses Bitpack_getu() and Bitpack_gets() to extract unsigned and 
 *       signed values
 *     - Extracts values for a, b, c, d, Pb_avg, and Pr_avg
 */
void unpack_single_word(word w, uint32_t packed_word)
{
        assert(w != NULL);

        /* Extract each component using bit-packing get functions */
        w->a = Bitpack_getu(packed_word, 9, a_lsb);
        w->b = Bitpack_gets(packed_word, 5, b_lsb);
        w->c = Bitpack_gets(packed_word, 5, c_lsb);
        w->d = Bitpack_gets(packed_word, 5, d_lsb);
        w->Pb_avg = Bitpack_getu(packed_word, 4, pb_avg_lsb);
        w->Pr_avg = Bitpack_getu(packed_word, 4, pr_avg_lsb);
}


/**********************************/
/*          Getters/Setter        */
/**********************************/


/********** set_ypbpr ********
 * 
 * Purpose: Sets the Y, Pb, and Pr values in a Y_Pb_Pr struct
 *
 * Parameters:
 *     - ypbpr: A pointer to the Y_Pb_Pr struct to update
 *     - Y: The new Y value
 *     - Pb: The new Pb value
 *     - Pr: The new Pr value
 *
 * Return: None
 *
 * Expects:
 *     - ypbpr is not NULL
 *
 * Notes:
 *     - Calls setY, setPb, and setPr to update the values
 *     - The function ensures all three values are updated at once
 */
void set_ypbpr(Y_Pb_Pr ypbpr, float Y, float Pb, float Pr)
{
        assert(ypbpr != NULL);
        
        /* set each component seperately */
        setY(ypbpr, Y);
        setPb(ypbpr, Pb);
        setPr(ypbpr, Pr);
}
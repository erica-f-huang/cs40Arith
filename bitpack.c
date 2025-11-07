/**************************************************************
 *
 *                     bitpack.c
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     2/21/25
 *
 *     Summary: 
 *     
 *     bitpack.c handles inserting and extracting values from specific bit 
 *     fields in a 64-bit word. It includes functions for working with both 
 *     signed and unsigned values while ensuring they fit within the given bit 
 *     width. The file also provides helper functions for masking, shifting, 
 *     and error handling.
 *     
 *
 **************************************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "except.h"
#include "bitpack.h"
#include "assert.h"

/* helper functions */
uint64_t shift_left(uint64_t word, unsigned shift);
uint64_t shift_rightu(uint64_t word, unsigned shift);
int64_t shift_rights(int64_t word, unsigned shift);
uint64_t mask(unsigned width, unsigned lsb);

/* except statement from the spec */
Except_T Bitpack_Overflow = { "Overflow packing bits" };

/************* Bitpack_fitsu ********
 * 
 * Purpose: Checks if unsigned int n can be represented by "width" number 
 *          of bits
 * 
 * Parameters:
 *      n: the unsigned integer we want to represent
 *      width: the number of bits
 * 
 * Return: True if n can be represented by "width" number of bits, false 
 *         otherwise
 * 
 * Expecations: none
 * 
 * CRE: width > 64
 * 
 * Notes: uses helper function shift_left();
 */
bool Bitpack_fitsu(uint64_t n, unsigned width)
{
        /* Make sure width is correct */
        assert (width <= 64);
        if (width == 64){
                return true; 
        }
        if (width == 0){
                return n == 0;
        }

        uint64_t one = 1;
        uint64_t upper_val = shift_left(one, width);

        return (n < upper_val);
}

/************* Bitpack_fitss ********
 * 
 * Purpose: Checks if signed int n can be represented by "width" number of bits
 * 
 * Parameters:
 *      n: the signed integer we want to represent
 *      width: the number of bits
 * 
 * Return: True if n can be represented by "width" number of bits, false 
 *         otherwise
 * 
 * Expecations: none
 * 
 * CRE: width > 64
 * 
 * Notes: uses Bitpack_fitsu() and helper function shift_left();
*/
bool Bitpack_fitss( int64_t n, unsigned width)
{
        /* Make sure width is correct */
        assert (width <= 64);
        if (width == 64) {
                return true;
        }
        if(width == 0){
                return n == 0;
        }

        /*check upper limit*/
        if((int64_t)n >= 0){
                return Bitpack_fitsu(n, width - 1);
        }

        /*check lower limit*/
        int64_t neg_one = -1;
        int64_t min_val = shift_left(neg_one, width - 1);
        return(n >= min_val); 
}

/************* Bitpack_getu ********
 * 
 * Purpose: Get the field of the word starting at bit # "lsb" and is width 
 *          "width" Field is represented as an unsigned int
 * 
 * Parameters:
 *       word: the word to extract the field from
 *       width: the width of the field
 *       lsb: the least significant bit of the field (the leftmost bit)
 * 
 * Return: the extracted bit field (as an unsigned int)
 * 
 * Expecations: none
 * 
 * CRE: width > 64, lsb > 63, width + lsb > 64. Also some in mask()
 * 
 * Notes: uses helper functions mask() and shift_rightu()
 */
uint64_t Bitpack_getu(uint64_t word, unsigned width, unsigned lsb)
{
        if(width == 0 || word == 0){
                return 0;
        }
        assert(width <= 64);
        assert(lsb <= 63);
        assert(width + lsb <= 64);

        /* Create a mask and shift to line up with the field*/
        uint64_t word_mask = mask(width, lsb);
        uint64_t extraction = (word & word_mask);

        return shift_rightu(extraction, lsb); 
}

/************* Bitpack_gets ********
 * Purpose: Get the field of the word starting at bit # "lsb" and is width 
 *          "width", Field is represented as a signed int
 *
 * Parameters:
 *      - word: the word to extract the field from
 *      - width: the width of the field
 *      - lsb: the least significant bit of the field (the leftmost bit)
 * 
 * Return: the extracted bit field (as a signed int)
 * 
 * Expecations: none
 * 
 * CRE: none directly. Helper functions contain some asserts.
 * 
 * Notes: uses Bitpack_getu() and helper function mask()
 */
int64_t Bitpack_gets(uint64_t word, unsigned width, unsigned lsb)
{
        if(width == 0 || word == 0){
                return 0;
        }
        
        uint64_t extracted = Bitpack_getu(word, width, lsb);

        /*add leading 1s if extracted is negative*/
        /* explaination becuase this confused us:
        the extracted value is all the way at to the right in the 64 bits
        after using Bitpack_getu() (ex. 0000...1101, where 1101 is the 
        extracted field) and the rest of the 64 bits are 0s becuase it is 
        unsigned. If the highest order bit of the extracted value is 1 
        (negative), then we need to turn the rest of the 64 bits from 0s to 1s 
        so it keeps its negative-ness (ex unsigned 0000...1101 to signed 
        1111...1101)*/
        uint64_t HOB = mask(1, width - 1);
        if((extracted & HOB) != 0){
                uint64_t leadingOnes = mask(64 - width, width);
                extracted = (leadingOnes | extracted);
        }

        return (int64_t)extracted;
}

/************* Bitpack_newu ********
 * 
 * Purpose: Inserts an unsigned integer value into a specific bit field 
 *          within a 64-bit word
 *
 * Parameters:
 *     - word: The original word to modify
 *     - width: The number of bits allocated for the field (must be ≤ 64)
 *     - lsb: The least significant bit (LSB) position of the field
 *     - value: The new unsigned value to insert
 *
 * Return: A new 64-bit word with the updated field
 *
 * Expects:
 *     - width + lsb <= 64 (does not exceed word bounds)
 *     - width > 0 (must have at least one bit)
 *     - value fits within "width" bits (checked using Bitpack_fitsu())
 *
 * CREs:
 *     - Raises Bitpack_Overflow if value cannot fit in width bits
 *
 * Notes:
 *     - Uses Bitpack_fitsu(` to verify fit before inserting
 *     - Uses mask() to create a bitmask and shift_left() for placement
 */
uint64_t Bitpack_newu(uint64_t word, unsigned width, unsigned lsb, 
                      uint64_t value)
{
        if (Bitpack_fitsu(value, width) == false) {
              RAISE(Bitpack_Overflow);  
        }

        assert(width + lsb <= 64); 
        assert(width <= 64);
        assert(lsb <= 63);

        /* step 1: create mask */
        uint64_t word_mask = mask(width, lsb);
        word_mask = ~word_mask;
        
        /* step 2: clear the field */
        uint64_t cleared_word = (word & word_mask);

        /*shift the value so its in the position of the field*/
        uint64_t shifted_val = shift_left(value, lsb);
        
        return (cleared_word | shifted_val);
}

/************ Bitpack_news ********
 * 
 * Purpose: Inserts a signed integer value into a specific bit field 
 *          within a 64-bit word
 *
 * Parameters:
 *     - word: The original word to modify
 *     - width: The number of bits allocated for the field (must be ≤ 64)
 *     - lsb: The least significant bit (LSB) position of the field
 *     - value: The new unsigned value to insert
 *
 * Return: A new 64-bit word with the updated field
 *
 * Expects:
 *     - width + lsb ≤ 64 (does not exceed word bounds)
 *     - width > 0 (must have at least one bit)
 *     - value fits within "width" bits (checked using Bitpack_fitsu())
 *
 * CREs (Checked Runtime Errors):
 *     - Raises Bitpack_Overflow if value cannot fit in width bits
 *
 * Notes:
 *     - Uses Bitpack_fitss() to verify fit before inserting
 *     - Uses mask() to create a bitmask and shift_left() for placement
 *     - Properly handles negative values using sign 
 */
uint64_t Bitpack_news(uint64_t word, unsigned width, unsigned lsb, 
                      int64_t value)
{
        if (Bitpack_fitss(value, width) == false) {
              RAISE(Bitpack_Overflow);  
        }

        assert(width + lsb <= 64); 
        assert(width <= 64);
        assert(lsb <= 63);

        /* step 1: create mask */
        uint64_t word_mask = mask(width, lsb);
        word_mask = ~word_mask;
        
        /* step 2: clear the field */
        uint64_t cleared_word = (word & word_mask);

        /*shift the value so its in the position of the field*/
        uint64_t shifted_val = shift_left(value, lsb);
        shifted_val = ((~word_mask) & shifted_val);
        
        return (cleared_word | shifted_val);       
}

/*********** shift_left ********
 * 
 * Purpose: shifts the word to the left "shift" # of times
 * 
 * Parameters:
 *       - word: the word to shift left
 *       - shift: the number of times to shift the word left
 * 
 * Return: the shifted word
 * 
 * Expecations: none
 * 
 * CRE: none
 * 
 * Notes: none
 */
uint64_t shift_left(uint64_t word, unsigned shift)
{
        if (shift >= 64){
                return 0;
        }

        return word << shift;
}

/*********** shift_rightu ********
 * 
 * Purpose: shifts the word to the right "shift" # of times
 * 
 * Parameters:
 *       - word: the word to shift right
 *       - shift: the number of times to shift the word right
 * 
 * Return: the shifted word
 * 
 * Expecations: none
 * 
 * CRE: none
 * 
 * Notes: will always propogate 0
 */
uint64_t shift_rightu(uint64_t word, unsigned shift)
{
        if (shift >= 64){
                return 0;
        }

        return word >> shift;
}

/*********** shift_rights **********
 * 
 * Purpose: shifts the word to the right "shift" # of times
 * 
 * Parameters:
 *       - word: the word to shift right
 *       - shift: the number of times to shift the word right
 * 
 * Return: the shifted word
 * 
 * Expecations: none
 * 
 * CRE: none
 * 
 * Notes: will propogate 0 if the word's highest order bit is on. Will 
 *      propogate 1 otherwise
 */
int64_t shift_rights(int64_t word, unsigned shift)
{
        if (shift >= 64){
                return 0;
        }

        return word >> shift;

}

/*********** mask ********
 * 
 * Purpose: create a 64 bit mask of the given width starting at bit lsb
 * 
 * Parameters:
 *      - width: the width of the 1s in the mask
 *      - lsb: the index of bits the mask starts at (the leftmost 1)
 * 
 * Return: the new mask
 * 
 * Expecations: non
 * 
 * CRE: width > 64, lsb > 63, width + lsb > 64
 * 
 * Notes: 
 *      - uses helper functions shift_rightu() & shift_left()
 *      - we think we could have only used shifts 3 times but haven't figured 
 *        out the right formula for it. It works with 4 but is just a little
 *        redundant
 */
uint64_t mask(unsigned width, unsigned lsb){
        assert(width <= 64);
        assert(lsb <= 63);
        assert(width + lsb <= 64);
        
        uint64_t mask = ~0;
        /*push off ones to the right of the field*/
        mask = shift_rightu(mask, lsb); 
        /*push off ones to the left of the field*/
        mask = shift_left(mask, 64 - width);

        /*bring back to correct location*/
        mask = shift_rightu(mask, 64 - width);
        mask = shift_left(mask, lsb);

        return mask;
}
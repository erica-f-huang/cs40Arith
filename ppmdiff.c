/**************************************************************
 *
 *                     ppmdiff.c
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), 
 *     Date:     2/21/25
 *
 *     Summary:
 * 
 *     This file compares two PPM images by computing the Root Mean 
 *     Square Difference (RMSD) between their pixel values. It ensures 
 *     the images are within an allowable size difference before calculating 
 *     color differences. The program outputs a numerical RMSD value, 
 *     indicating the level of difference between the two images
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
#include "uarray2b.h"
#include "uarray2.h"
#include <math.h>
#include <string.h>

/* Struct to hold RGB values as floating points for conversion */
typedef struct {
        float red, green, blue;
} floatRGB;

/* helper function declerations */
double rmsd(A2Methods_UArray2 pixels1, A2Methods_UArray2 pixels2,
        A2Methods_T methods, int width, int height, int denom1, int denom2);
double summation_helper(A2Methods_UArray2 pixels1, A2Methods_UArray2 pixels2,
        A2Methods_T methods, int col, int row, int denom1, int denom2);

/********** main ********
 * 
 * Purpose: Reads two PPM image files, validates their dimensions, and computes 
 *          the Root Mean Square Difference (RMSD) between them
 *
 * Parameters:
 *     - argc: The number of command-line arguments
 *     - argv: An array of command-line arguments, where:
 *
 * Return:
 *     - Returns EXIT_SUCCESS (0) if successful
 *     - Terminates with an error message if the images differ in size by 
 *       more than 1 pixel
 *
 * Expects:
 *     - exactly two image files or one file and stdin
 *     - The file paths are valid and accessible
 *     - Both files are in the correct PPM format
 *
 * Notes:
 *     - Uses Pnm_ppmread() to read images into A2Methods_UArray2
 *     - Compares images based on the smaller of their dimensions
 *     - Outputs the RMSD value with four decimal places
 */
int main(int argc, char *argv[]) 
{
        assert(argc == 3);
        
        FILE *file1;
        FILE *file2;

        /* make sure both aren't "-" */
        assert(!((strcmp(argv[1], "-") == 0) && (strcmp(argv[2], "-") == 0)));

        for(int i = 1; i < 3; i++){
                if(strcmp(argv[i], "-") == 0){
                        if(i == 1){
                                file1 = stdin;
                        } else {
                                file2 = stdin;
                        }

                } else{
                        if(i == 1){
                                file1 = fopen(argv[i], "rb");
                        } else {
                                file2 = fopen(argv[i], "rb");
                        }
                }
        }

        assert(file1 != NULL);
        assert(file2 != NULL);

        A2Methods_T methods = uarray2_methods_blocked;
        Pnm_ppm ppm1 = Pnm_ppmread(file1, methods);
        Pnm_ppm ppm2 = Pnm_ppmread(file2, methods);

        assert(ppm1 != NULL);
        assert(ppm2 != NULL);

        /* Calculate the width/height difference between the two images */
        int width_diff = abs((int)ppm1->width - (int)ppm2->width);
        int height_diff = abs((int)ppm1->height - (int)ppm2->height);

        if (width_diff > 1 || height_diff > 1) {
                fprintf(stderr, 
                        "Error: Image dimensions differ by more than 1\n");
                exit(EXIT_FAILURE);
        }

        /* determine smaller image */
        unsigned width = ppm1->width; 
        if (ppm2->width < width){
                width = ppm2->width;
        }
        unsigned height = ppm1->height; 
        if (ppm2->height < height){
                height = ppm2->height;
        }

        /* Calculate the Root Mean Square Difference */
        double E = rmsd(ppm1->pixels, ppm2->pixels, methods, width, height, 
                          ppm1->denominator, ppm2->denominator);

        /* print */
        printf("%.4f\n", E);

        /*clean up*/
        Pnm_ppmfree(&ppm1);
        Pnm_ppmfree(&ppm2);
        fclose(file1);
        fclose(file2);
        return EXIT_SUCCESS; 
}

/********** rmsd ********
 * 
 * Purpose: Computes the Root Mean Square Difference (RMSD) between two images
 *
 * Parameters:
 *     - pixels1: pixel data for image 1
 *     - pixels2: pixel data for image 2
 *     - methods: A2Methods_T function for handling UArray2
 *     - width: The common width of both images (after trimming if needed)
 *     - height: The common height of both images (after trimming if needed)
 *     - denom1: The denominator used for scaling color values in image 1
 *     - denom2: The denominator used for scaling color values in image 2
 *
 * Return: The computed RMSD value as a double
 *
 * Expects:
 *     - pixels1 and pixels2 are not NULL
 *     - width and height are greater than zero
 *     - denom1 and denom2 are valid positive integers
 *
 * Notes:
 *     - Uses helper function, summation_helper, to compute color differences
 */
double rmsd(A2Methods_UArray2 pixels1, A2Methods_UArray2 pixels2,
            A2Methods_T methods, int width, int height, int denom1, int denom2)
{
        double total = 0.0;
       
        for (int rowy = 0; rowy < height; rowy++){
                for(int colx = 0; colx < width; colx++){
                        total += summation_helper(pixels1, pixels2, methods, 
                                colx, rowy, denom1, denom2);
                }
        }

        assert((width * height) != 0);
        
        double quotient = total / (3.0 * width * height);

        return sqrt(quotient);
}

/********** summation_helper ********
 * 
 * Purpose: Computes the squared difference of RGB values for a single pixel
 *
 * Parameters:
 *     - pixels1: pixel data for image 1
 *     - pixels2: pixel data for image 2
 *     - methods: A2Methods_T function for handling UArray2
 *     - width: The common width of both images (after trimming if needed)
 *     - height: The common height of both images (after trimming if needed)
 *     - denom1: The denominator used for scaling color values in image 1
 *     - denom2: The denominator used for scaling color values in image 2
 *
 * Return: The squared difference of RGB values for the given pixel
 *
 * Expects:
 *     - pixels1 and pixels2 contain valid pixel data
 *     - methods is a valid function table
 *     - col and row indices are within bounds
 *     - denom1 and denom2 are positive integers
 */
double summation_helper(A2Methods_UArray2 pixels1, A2Methods_UArray2 pixels2,
                        A2Methods_T methods, int col, int row, int denom1, 
                        int denom2)
{
        Pnm_rgb pixel1 = methods->at(pixels1, col, row);
        Pnm_rgb pixel2 = methods->at(pixels2, col, row);
        
        double red1 = pixel1->red / ((double)denom1);
        double green1 = pixel1->green / ((double)denom1);
        double blue1 = pixel1->blue / ((double)denom1);

        double red2 = pixel2->red / ((double)denom2);
        double green2 = pixel2->green / ((double)denom2);
        double blue2 = pixel2->blue / ((double)denom2);

        double rDiff = red1 - red2;
        double bDiff = blue1 - blue2;
        double gDiff = green1 - green2;

        double total = (rDiff * rDiff) + (gDiff * gDiff) + (bDiff * bDiff);

        return total;
}


/**************************************************************
 *
 *                     ry_conversion.c
 *
 *     Assignment: Arith 
 *     Authors:  Marielle Cibella (mcibel01), Erica Huang (ehuang02)
 *     Date:     3/1/25
 *
 *     Summary:
 * 
 *     This file contains functions for converting between RGB and YPbPr 
 *     color spaces, used in image compression and decompression. The file
 *     ensures proper scaling and normalization of color values while 
 *     handling floating-point inofrmation loss during conversion.
 *     
 *
 **************************************************************/

#include "ry_conversion.h"

/* 
 * struct closure stores information needed for image processing, including 
 * a pointer to a 2D array of pixels, a set of methods for handling the array, 
 * and a denominator for scaling pixel values. It is used in applu functions 
 * that modify images to ensure easy access to pixel data and processing tools.
 */
struct closure{
        A2Methods_UArray2 *pixels;
        A2Methods_T methods;
        int denominator;
};

/* 
 * struct Y_Pb_Pr represents a pixel in the YPbPr color space, where Y 
 * controls brightness, and Pb and Pr represent chroma components. 
 * All values are stored as floating-point numbers, and this struct is used 
 * for color conversion between RGB and YPbPr.
 */
struct Y_Pb_Pr{
        float Y, Pb, Pr; 
}; 

/**************************/
/*       Compression      */
/**************************/

/********** rgb_to_ypbpr ********
 * 
 * Purpose: Converts a PPM image containing RGB pixels into a 2D array of 
 *          Y/Pb/Pr pixels
 *
 * Parameters:
 *      - ppm: A pointer to the PPM image containing RGB pixel data
 *
 * Return: A new 2D array where each pixel is stored in the Y/Pb/Pr color space
 *
 * Expects:
 *      - ppm is a valid pointer to a Pnm_ppm image (not NULL)
 *      - The ppm struct contains valid methods that support accessing and 
 *        modifying pixel data
 *
 * CRE: ppm is null, pixels of ppm is null, methods is null, ypbpr_pixels is
 *      null, or map is null
 *
 * Notes:
 *      - This function creates a new A2Methods_UArray2 to store the Y/Pb/Pr 
 *        pixel values
 *      - It applies the to_ypbpr_apply function to each pixel in the image
 *      - Uses the image's denominator for normalization of RGB values
 *      - Information is lost here due to floating point arithmetic in
 *              helper function to_ypbpr_apply().
 */
A2Methods_UArray2 rgb_to_ypbpr (Pnm_ppm ppm)
{
        assert(ppm != NULL);
        assert(ppm->pixels != NULL);

        /* get the methods for UArray2 */
        A2Methods_T methods = (A2Methods_T)ppm->methods;
        assert(methods != NULL);

        /* Create UArray2 to store Y_Pb_Pr */
        A2Methods_UArray2 ypbpr_pixels = methods->new(ppm->width, ppm->height, 
                                                        sizeof(struct Y_Pb_Pr));
        assert(ypbpr_pixels != NULL);

        /* create closure struct */
        closure cl = {&ypbpr_pixels, methods, ppm->denominator};

        /* Apply function */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);
        map(ppm->pixels, to_ypbpr_apply, &cl);
        
        return ypbpr_pixels; 
}

/********** to_ypbpr_apply ********
 * 
 * An apply function used by map to convert a single pixel from RGB to Y_Pb_Pr
 * using the image's denominator for normalization
 *
 * Parameters:
 *      - col: The column index of the current pixel in the array
 *      - row: The row index of the current pixel in the array
 *      - array2: The 2D array containing the original RGB pixel data
 *      - elem: A pointer to the current pixel being processed
 *      - cl: A closure containing:
 *          - ypbpr_pixels: The output array for Y/Pb/Pr pixel values
 *          - methods: Function pointers for handling UArray2
 *          - denominator: The maximum color value used for normalization
 *
 * Return: None 
 *
 * Expects:
 *      - array2 is a valid A2Methods_UArray2 containing RGB pixel data
 *      - elem is a valid pointer to a Pnm_rgb struct (not NULL)
 *      - cl is a valid pointer to a closure struct with the necessary fields
 *
 * CRE: closure is NULL, ypbpr_pixels from closure is null, the pointer to 
 *      ypbpr_pixels is null, methods from closure is null, the denominator
 *      is less than or equal to 0, the current element is null, or ypbpr is 
 *      null
 *
 * Notes:
 *      - This function normalizes RGB values using the denominator before 
 *        converting them to Y/Pb/Pr.
 *      - It modifies the corresponding pixel in the ypbpr_pixels array
 *      - Information is lost here due to floating point arithmetic 
 *             
 */
void to_ypbpr_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
                 void *cl)
{
        (void)array2; 

        /* retrieve closure struct */
        closure *info = (closure *)cl;
        assert(info != NULL); 

        /* retrieve the destination Y Pb Pr array */
        A2Methods_UArray2 *ypbpr_pixels = info->pixels;
        assert(ypbpr_pixels !=NULL);
        assert(*ypbpr_pixels != NULL);

        A2Methods_T methods = info->methods;
        assert(methods != NULL);

        /* get the denominator for normalizing RGB values*/
        int denominator = info->denominator;
        assert(denominator > 0);

        /* Retrieve the current RGB pixel */
        Pnm_rgb rgb = elem;
        assert(rgb != NULL);

        /* Convert RGB to Floating-Point and normalize */
        /* info is lost here due to floats */
        float r = (float)rgb->red / denominator;
        float g = (float)rgb->green / denominator;
        float b = (float)rgb->blue / denominator;

        /* Calculate Y/Pb/Pr pixel location */
        Y_Pb_Pr ypbpr = methods->at(*ypbpr_pixels, col, row); 
        assert(ypbpr != NULL);
        
        /* convert RGB to Y/Pb/Pr using the formula from the spec */
        /* info is lost here due to floats */
        ypbpr->Y = 0.299 * r + 0.587 * g + 0.114 * b;
        ypbpr->Pb = -0.168736 * r - 0.331264 * g + 0.5 * b;
        ypbpr->Pr = 0.5 * r - 0.418688 * g - 0.081312 * b;
}




/******************************/
/*       Decompression        */
/******************************/


/********** ypbpr_to_rgb ********
 * 
 * Purpose: Converts a 2D array of Y/Pb/Pr pixel values back into a 2D 
 *          array of RGB pixel values
 *
 * Parameters:
 *      - ypbpr_pixels: The input 2D array containing pixels in Y/Pb/Pr
 *      - methods: Function pointers for handling UArray2 operations
 *      - denominator: The maximum color value used for scaling RGB components
 *
 * Return: A new 2D array containing pixels in RGB format
 *
 * Expects:
 *      - ypbpr_pixels is valid (not NULL)
 *      - methods is a valid pointer to an A2Methods_T structure (not NULL)
 *      - denominator is a positive integer greater than zero
 *
 * CRE: ypbpr_pixels is null, methods is null, denominator is less than or equal
 *      to 0, rgb_pixels is null, or map is null
 *
 * Notes:
 *      - This function allocates a new UArray2 to store the RGB pixel values
 *      - It applies the to_rgb_apply function to each Y/Pb/Pr pixel in the
 *        array
 *      - Information can be lost here due to floating point arithmetic
 *              in convert_ypbpr_to_rgb().
 */
A2Methods_UArray2 ypbpr_to_rgb(A2Methods_UArray2 ypbpr_pixels, 
        A2Methods_T methods, int denominator)
{
        assert(ypbpr_pixels != NULL);
        assert(methods != NULL);
        assert(denominator > 0);

        /* get the dimensions of the Y/Pb/Pr pixel array */
        int width = methods->width(ypbpr_pixels);
        int height = methods->height(ypbpr_pixels);

        /* Create UArray2 to store RGB pixel values */
        A2Methods_UArray2 rgb_pixels = methods->new(width, height, 
                                        sizeof(struct Pnm_rgb));
        assert(rgb_pixels != NULL);

        /* create closure struct */
        closure cl = {&rgb_pixels, methods, denominator};

        /* mapping and apply function */
        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);
        map(ypbpr_pixels, to_rgb_apply, &cl);

        return rgb_pixels; 
}

/********** to_rgb_apply ********
 * 
 * Purpose: Converts a Y_Pb_Pr pixel into an RGB pixel and stores it in the 
 *          corresponding location in the RGB UArray2
 *
 * Parameters:
 *      - col: The column index of the pixel in the Y_Pb_Pr UArray2.
 *      - row: The row index of the pixel in the Y_Pb_Pr UArray2.
 *      - array2: The 2D array of Y_Pb_Pr pixels.
 *      - elem: A pointer to the Y_Pb_Pr struct for the current pixel.
 *      - cl: A closure struct containing:
 *          - rgb_pixels: The output array for RGB pixel values
 *          - methods: Function pointers for handling UArray2
 *          - denominator: The maximum color value for scaling RGB components
 *
 * Return: None
 *
 * Expects:
 *      - array2 is a valid A2Methods_UArray2 object
 *      - elem is a valid pointer to a Y_Pb_Pr struct (not NULL)
 *      - cl is a valid pointer to a closure struct with the necessary fields
 *      - The methods pointer inside the closure struct is valid (not NULL)
 *
 * CRE: closure is null, rgb_pixels from the closure is null, the pointer to
 *      rgb_pixels is null, the methods are null from the closure is null,
 *      the denominator from the closure is null, ypbpr is null, or rgb is null
 *
 * Notes:
 *      - Uses convert_ypbpr_to_rgb to perform the conversion
 *      - Ensures that all RGB values are within the valid range
 *      - Information can be lost here due to floating point arithmetic in
 *              helper function convert_ypbpr_to_rgb().
 */
void to_rgb_apply(int col, int row, A2Methods_UArray2 array2, void *elem, 
                        void *cl)
{
        /* unused parameter */
        (void)array2; 

        /* extract members from closure struct*/
        closure *info = (closure *)cl;
        assert(info != NULL); 

        /* retrieve the output RGB pixel array */
        A2Methods_UArray2 *rgb_pixels = info->pixels;
        assert(rgb_pixels !=NULL);
        assert(*rgb_pixels != NULL);

        /* retrive methods */
        A2Methods_T methods = info->methods;
        assert(methods != NULL);

        /* retreive denominator for scaling */
        int denominator = info->denominator;
        assert(denominator > 0);

        /* retrieve the Y/Pb/Pr pixel to convert */
        Y_Pb_Pr ypbpr = elem;
        assert(ypbpr != NULL);

        /* Get the output pixel location in the RGB array */
        Pnm_rgb rgb = methods->at(*rgb_pixels, col, row);
        assert(rgb != NULL);
    
        /* Perform the conversion using helper */
        convert_ypbpr_to_rgb(ypbpr, denominator, rgb);
}

/********** convert_ypbpr_to_rgb ********
 * 
 * Purpose: Converts a Y_Pb_Pr pixel into an RGB pixel using a given denominator 
 *          for scaling
 *
 * Parameters:
 *      - ypbpr: The Y_Pb_Pr struct containing Y, Pb, and Pr values
 *      - denominator: The maximum value for RGB components
 *      - rgb: The struct where the converted RGB values will be stored
 *
 * Return: None
 *
 * Expects:
 *      - ypbpr is a valid Y_Pb_Pr struct 
 *      - rgb is a valid pointer to a Pnm_rgb struct
 *      - denominator is a positive integer greater than zero
 *
 * CRE: ypbpr is null, rgb is null, or denominator is less than or equal to 0 
 *
 * Notes:
 *      - Uses floating-point arithmetic to convert Y, Pb, and Pr values to RGB
 *        from the spec
 *      - Information is lost here due to floating point arithmetic
 */
void convert_ypbpr_to_rgb(Y_Pb_Pr ypbpr, int denominator, Pnm_rgb rgb)
{
        assert(ypbpr != NULL);
        assert(rgb != NULL);
        assert(denominator > 0);

        /* Extract Y, Pb, Pr values from the Y_Pb_Pr struct */
        float y = ypbpr->Y;
        float pb = ypbpr->Pb;
        float pr = ypbpr->Pr;

        /* Convert Y/Pb/Pr to RGB using the spec formula */
        /*information lost here due to rounding*/
        float rFloat = 1.0 * y + 0.0 * pb + 1.402 * pr;
        float gFloat = 1.0 * y - 0.344136 * pb - 0.714136 * pr;
        float bFloat = 1.0 * y + 1.772 * pb + 0.0 * pr;

        /* Convert floating-point values to integers and scale by denominator */
        /*information lost here bc converting from float to int*/
        int red = (unsigned int) (rFloat * denominator);
        int green = (unsigned int) (gFloat * denominator);
        int blue = (unsigned int) (bFloat * denominator);

        /* clamp values to keep them in valid range [o, denominator] */
        if(red <= 0){             red = 0;}
        if(red >= denominator){   red = denominator;}

        if(green <= 0){           green = 0;}
        if(green >= denominator){ green = denominator;}

        if(blue <= 0){            blue = 0;}
        if(blue >= denominator){  blue = denominator;}

        /* store the converted RGB values in the pixel struct */
        rgb->red = red;
        rgb->green = green;
        rgb->blue = blue;
}


/********************************************/
/*       setters, getters, size, new        */
/********************************************/


/********** new_Y_Pb_Pr ********
 * 
 * Purpose: Allocates and initializes a new Y_Pb_Pr struct with the given 
 *          Y, Pb, and Pr values
 *
 * Parameters:
 *      - Y : The brightness value
 *      - Pb: The blue-difference chroma value
 *      - Pr: The red-difference chroma value
 *
 * Return: A pointer to the newly allocated Y_Pb_Pr struct containing the values
 *
 * Expects:
 *      - Y, Pb, and Pr are valid floating-point values within their ranges
 *      - Memory allocation for the struct works correctly 
 *
 * CRE: none
 *
 * Notes:
 *      - The caller is responsible for freeing the allocated Y_Pb_Pr 
 *        struct when it is no longer needed
 */
Y_Pb_Pr new_Y_Pb_Pr(float Y, float Pb, float Pr)
{
        /* Allocate memory for a new Y_Pb_Pr struct */
        Y_Pb_Pr ypbpr;
        NEW(ypbpr);
        assert(ypbpr != NULL);

        /* Assign provided Y Pb Pr values */
        ypbpr->Y = Y;
        ypbpr->Pb = Pb;
        ypbpr->Pr = Pr;

        return ypbpr;
}

/********** Y_Pb_Pr_size ********
 * 
 * Purpose: Returns the size in bytes of the Y_Pb_Pr struct
 *
 * Parameters: None
 *
 * Return: An integer representing the size of the Y_Pb_Pr struct
 *
 * Expects:
 *      - The Y_Pb_Pr struct is defined
 *
 * CRE: none
 *
 * Notes:
 *      - This function is useful when dynamically allocating memory 
 *        for Y_Pb_Pr structures
 */
int Y_Pb_Pr_size()
{
        return sizeof(struct Y_Pb_Pr);
}

/********** getY ********
 * 
 * Purpose: Retrieves the Y value from a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *
 * Return: A float representing the Y value of the given Y_Pb_Pr struct
 *
 * Expects: ypbpr is a valid pointer (not NULL)
 *
 * CRE: none
 *
 * Notes:
 *      - This function is a getter
 */
float getY(Y_Pb_Pr ypbpr)
{
        assert(ypbpr != NULL);
        return ypbpr->Y;
}

/********** getPb ********
 * 
 * Purpose: Retrieves the Pb value from a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *
 * Return: A float representing the Pb value of the given Y_Pb_Pr struct
 *
 * Expects: ypbpr is a valid pointer (not NULL)
 *
 * CRE: none
 *
 * Notes:
 *      - This function is a getter
 */
float getPb(Y_Pb_Pr ypbpr)
{
        assert(ypbpr != NULL);
        return ypbpr->Pb;
}

/********** getPr ********
 * 
 * Purpose: Retrieves the Pr value from a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *
 * Return: A float representing the Pb value of the given Y_Pb_Pr struct
 *
 * Expects: ypbpr is a valid pointer (not NULL)
 *
 * CRE: none
 *
 * Notes:
 *      - This function is a getter
 */
float getPr(Y_Pb_Pr ypbpr)
{
        assert(ypbpr != NULL);
        return ypbpr->Pr;
}

/********** setY ********
 * 
 * Purpose: Sets the Y value in a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *      - Y: The new Y value to set
 *
 * Return: None
 *
 * Expects:
 *      - ypbpr is a valid pointer (not NULL)
 *      - Y is within the valid range for Y values
 *
 * CRE:none
 *
 * Notes:
 *      - This function is a setter 
 */
void setY(Y_Pb_Pr ypbpr, float Y)
{
        assert(ypbpr != NULL);
        ypbpr->Y = Y;
}

/********** setPb ********
 * 
 * Purpose: Sets the Pb value in a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *      - Pb: The new Pb value to set
 *
 * Return: None
 *
 * Expects:
 *      - ypbpr is a valid pointer (not NULL)
 *      - Pb is within the valid range for Pb values
 *
 * CRE: none
 *
 * Notes:
 *      - This function is a setter 
 */
void setPb(Y_Pb_Pr ypbpr, float Pb)
{
        assert(ypbpr != NULL);
        ypbpr->Pb = Pb;
}

/********** setPr ********
 * 
 * Purpose: Sets the Pr value in a Y_Pb_Pr struct
 *
 * Parameters:
 *      - ypbpr: A pointer to a Y_Pb_Pr struct
 *      - Pr: The new Pr value to set
 *
 * Return: None
 *
 * Expects:
 *      - ypbpr is a valid pointer (not NULL)
 *      - Pr is within the valid range for Pr values
 *
 * CRE: none
 *
 * Notes:
 *      - This function is a setter 
 */
void setPr(Y_Pb_Pr ypbpr, float Pr)
{
        assert(ypbpr != NULL);
        ypbpr->Pr = Pr;
}
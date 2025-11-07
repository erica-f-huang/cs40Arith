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
#include <math.h>
#include <string.h>
#include "mem.h"

void trimWidthHeight(int colx, int rowy, A2Methods_UArray2 arr, 
        void *elem, void *cl);

typedef struct {
        A2Methods_UArray2 *destinationImg;
        A2Methods_T methods;
        int trimWidth;
        int trimHeight;
} destinationAndMethods;

int main(int argc, char *argv[]){
        (void)argc;
        
        FILE *file;
        file = fopen(argv[1], "rb");

        A2Methods_T methods = uarray2_methods_plain;
        Pnm_ppm ppm = Pnm_ppmread(file, methods);

        ppm->width = 1;
        ppm->height = 1;

        Pnm_ppmwrite(stdout, ppm); 

        return 0;


        A2Methods_mapfun *map = methods->map_default; 
        assert(map != NULL);

        A2Methods_UArray2 pixels = ppm->pixels;

        int width;
        int trimWidth = 0;
        if(ppm->width % 2 == 1){
                width = ppm->width - 1;
                trimWidth = 1;
        } else{
                width = ppm->width;
        }
        
        int height;        
        int trimHeight = 0;
        if(ppm->height % 2 == 1){
                height = ppm->height - 1;
                trimHeight = 1;
        } else{
                height = ppm->height;
        }

        A2Methods_UArray2 trimmedPixels = methods->new(width, height, methods->size(pixels));

        destinationAndMethods dAndM = {&trimmedPixels, methods, trimWidth, trimHeight};
        assert(&dAndM != NULL);

        map(pixels, trimWidthHeight, &dAndM);

        Pnm_ppm trimmedImg;
        NEW(trimmedImg);
        trimmedImg->width = methods->width(trimmedPixels);
        trimmedImg->height = methods->height(trimmedPixels);
        trimmedImg->denominator = ppm->denominator;
        trimmedImg->pixels = trimmedPixels;
        trimmedImg->methods = methods;
        assert(&trimmedImg != NULL);

        Pnm_ppmwrite(stdout, trimmedImg);

        Pnm_ppmfree(&ppm);
        Pnm_ppmfree(&trimmedImg);
}         

void trimWidthHeight(int colx, int rowy, A2Methods_UArray2 arr, 
        void *elem, void *cl){
        assert(arr != NULL);


        destinationAndMethods *dAndM = (destinationAndMethods *)cl;

        A2Methods_UArray2 *destinationImg = dAndM->destinationImg;

        // printf("line")
        assert(destinationImg !=NULL);

        A2Methods_T methods = dAndM->methods;
        // assert(methods != NULL);

        if((colx == methods->width(arr) - 1) && dAndM->trimWidth){
                return;
        }
        if((colx == methods->height(arr) - 1) && dAndM->trimHeight){
                return;
        }


        Pnm_rgb destVal = methods->at(*destinationImg, colx, rowy);
        Pnm_rgb ogVal = elem;
        destVal->red = ogVal->red;
        destVal->green = ogVal->green;
        destVal->blue = ogVal->blue;
}
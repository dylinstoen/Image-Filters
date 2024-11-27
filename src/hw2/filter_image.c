#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "image.h"
#define TWOPI 6.2831853
// Name: Dylin Stoen
// Id: 1976635
// Email: dylin20@uw.edu
// Partner: Gareth Coad
void l1_normalize(image im)
{
    float w = im.w;
    float h = im.h;
    float c = im.c;
    for (int i = 0; i < im.c; ++i){
        for (int k = 0; k < im.h; ++k){
            for (int j = 0; j < im.w; ++j){
                set_pixel(im, j, k, i, 1);
                float pixel = get_pixel(im, j, k, i);
                float pixel_norm = pixel * (1/(w * h * c));
                set_pixel(im, j, k, i, pixel_norm);
            }
        }
    } 
}

image make_box_filter(int w)
{
    image tmp = make_image(w, w, 1);
    l1_normalize(tmp);
    return tmp;
}

image convolve_image(image im, image filter, int preserve)
{
    assert(filter.c == im.c || filter.c == 1);
    
    int i, j, k;
    float pixel_f, pixel_im;
    int col, row, c;
    int bound_up;
    float Q;
    image result;
    bound_up = floor(filter.w/2);
    if ((preserve == 1) && ((filter.c == 1) && (im.c > 1))) {
        result = make_image(im.w, im.h, im.c);
        for (i = 0; i < im.c; ++i){
            for (j = 0; j < im.h; ++j){
                for (k = 0; k < im.w; ++k){
                    Q = 0;
                    for (row = 0; row < filter.h; ++row){
                        for (col = 0; col < filter.w; ++col){
                            for (c = 0; c < filter.c; ++c){
                                pixel_f = get_pixel(filter, col, row, 0);
                                pixel_im = get_pixel(im, k + col - bound_up, j + row - bound_up, i);
                                Q = pixel_f * pixel_im + Q;
                            }
                        }
                    }
                    set_pixel(result, k, j, i, Q);
                }
            }
        }
    }
    if ((preserve != 1) && ((filter.c == 1) && (im.c > 1))){
        result = make_image(im.w, im.h, filter.c);
        for (j = 0; j < im.h; ++j){
            for (k = 0; k < im.w; ++k){
                Q = 0;
                for (i = 0; i < im.c; ++i){
                    for (row = 0; row < filter.h; ++row){
                        for (col = 0; col < filter.w; ++col){
                            pixel_f = get_pixel(filter, col, row, 0);
                            pixel_im = get_pixel(im, k + col - bound_up, j + row - bound_up, i + 0);
                            Q = pixel_f * pixel_im + Q;
                        }
                    }
                }
                set_pixel(result, k, j, 0, Q);
            }
        }
    }
    return result;
}

image make_highpass_filter()
{
    image tmp = make_box_filter(3);
    tmp.data[0] = 0.0f;
    tmp.data[1] = -1.0f;
    tmp.data[2] = 0.0f;
    tmp.data[3] = -1.0f;
    tmp.data[4] = 4.0f;
    tmp.data[5] = -1.0f;
    tmp.data[6] = 0.0f;
    tmp.data[7] = -1.0f;
    tmp.data[8] = 0.0f;
    return tmp;
}

image make_sharpen_filter()
{
    image tmp = make_box_filter(3);
    tmp.data[0] = 0.0f;
    tmp.data[1] = -1.0f;
    tmp.data[2] = 0.0f;
    tmp.data[3] = -1.0f;
    tmp.data[4] = 5.0f;
    tmp.data[5] = -1.0f;
    tmp.data[6] = 0.0f;
    tmp.data[7] = -1.0f;
    tmp.data[8] = 0.0f;
    return tmp;
}

image make_emboss_filter()
{
    image tmp = make_box_filter(3);
    tmp.data[0] = -2.0f;
    tmp.data[1] = -1.0f;
    tmp.data[2] = 0.0f;
    tmp.data[3] = -1.0f;
    tmp.data[4] = 1.0f;
    tmp.data[5] = 1.0f;
    tmp.data[6] = 0.0f;
    tmp.data[7] = 1.0f;
    tmp.data[8] = 2.0f;;
    return tmp;
}

// Question 2.2.1: Which of these filters should we use preserve when we run our convolution and which ones should we not? Why?
// Answer:
// High pass and Sharper you want to perserve the images channels because these algorithms are smoothing the middle pixel with the 
// surrounding pixel using a weighted adverage and so keeping the channels is important in order for the smoothing to be scaled properly
// If you combine all of the pixels from the channels then the middle pixel has a chance of significantly scaling higher than the other pixels
// Emboss is an edge detection so its purpose would be to increase the scale so the edges that are going at a 45 degree angle in the image are more visiable.
// with no color values. However the Emboss filter is used for detecting edges and so combining all of the RGB may outcome in steeper
// vallys.
// Question 2.2.2: Do we have to do any post-processing for the above filters? Which ones and why?
// Answer: Yes with Emboss and Highpass because they both sum to 0. Sharper doesn't need too because it sums to 1.

image make_gaussian_filter(float sigma)
{
    int size = (int)(roundf(sigma)) * 6;
    if(size % 2 == 0) {
        size++;
    }
    image guassian = make_box_filter(size);

    
    int kDistW = floorf((((float)guassian.w)/2.0f));
    int kDistH = floorf((((float)guassian.h)/2.0f));

    for(int y = 0; y < guassian.h; y++) {
        for(int x = 0; x < guassian.w; x++) {
            int _x = abs(x - kDistW);
            int _y = abs(y - kDistH);
            int offset = guassian.w * y + x;
            float powG = ((_x * _x) + (_y * _y)) / (2.0f* (sigma * sigma));
            float baseG = 1.0f/(2.0f * 3.14159f * (sigma * sigma));
            float e = powf((1.0f/2.71828f),powG);
            guassian.data[offset] = baseG * e;
        }
    }

    float norm = 0.0f;
    int totalSize = guassian.w * guassian.h * guassian.c;
    for(int i = 0; i < totalSize; i++) {
        norm += guassian.data[i];
    }
    for(int j = 0; j < totalSize; j++) {
        guassian.data[j] = guassian.data[j]/norm;
    }

    return guassian;
}

image add_image(image a, image b)
{
    int aSize = a.c * a.h * a.w;
    int bSize = b.c * b.h * b.w;
    if(aSize != bSize) {
        return a;
    }
    image newImg = make_image(a.w, a.h, a.c);
    for(int i = 0; i < aSize; i++) {
        newImg.data[i] = a.data[i] + b.data[i];
    }
    return newImg;
}

image sub_image(image a, image b)
{
   int aSize = a.c * a.h * a.w;
    int bSize = b.c * b.h * b.w;
    if(aSize != bSize) {
        return a;
    }
    image newImg = make_image(a.w, a.h, a.c);
    for(int i = 0; i < aSize; i++) {
        newImg.data[i] = a.data[i] - b.data[i];
    }
    return newImg;
}

image make_gx_filter()
{
    image tmp = make_box_filter(3);
    tmp.data[0] = -1.0f;
    tmp.data[1] = 0.0f;
    tmp.data[2] = 1.0f;
    tmp.data[3] = -2.0f;
    tmp.data[4] = 0.0f;
    tmp.data[5] = 2.0f;
    tmp.data[6] = -1.0f;
    tmp.data[7] = 0.0f;
    tmp.data[8] = 1.0f;
    return tmp;
}

image make_gy_filter()
{
    image tmp = make_box_filter(3);
    tmp.data[0] = -1.0f;
    tmp.data[1] = -2.0f;
    tmp.data[2] = -1.0f;
    tmp.data[3] = 0.0f;
    tmp.data[4] = 0.0f;
    tmp.data[5] = 0.0f;
    tmp.data[6] = 1.0f;
    tmp.data[7] = 2.0f;
    tmp.data[8] = 1.0f;
    return tmp;
}

void feature_normalize(image im)
{
    int i;
    float min = im.data[0];
    float max = im.data[0];
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if(im.data[i] > max) max = im.data[i];
        if(im.data[i] < min) min = im.data[i];
    }
    for(i = 0; i < im.w*im.h*im.c; ++i){
        if((max - min) == 0.0f) {
            im.data[i] = 0.0f;
        } else {
            im.data[i] = (im.data[i] - min)/(max-min);
        }
        
    }
}

image *sobel_image(image im)
{
    image * imPtr = calloc(2, sizeof(image));
    image gx = make_gx_filter();
    image gy = make_gy_filter();
    
    imPtr[0] = convolve_image(im, gx, 0);
    imPtr[1] = convolve_image(im, gy , 0);

    int size = imPtr[0].c * imPtr[0].h * imPtr[0].w;
    for(int i = 0; i < size; i++) {
        float _gx = imPtr[0].data[i];
        float _gy = imPtr[1].data[i];
        imPtr[0].data[i] = sqrtf((_gx * _gx) + (_gy * _gy));
        imPtr[1].data[i] = atan2f(_gy, _gx);
    }
    return imPtr;
}

image colorize_sobel(image im)
{
    image guassian = make_gaussian_filter(2.0f);
    image * imPtr = sobel_image(im);
    image newImg = convolve_image(imPtr[0], guassian, 1);
    image mag = newImg;
    image angle = imPtr[1];
    feature_normalize(mag);
    feature_normalize(angle);
    
    int size = im.h * im.w;
    for(int _c = 0; _c < im.c; _c++) {
        for(int i = 0; i < size; i++) {
            if(_c == 0) {
                
                im.data[i] = angle.data[i];
            }
            else if(_c == 1) {
                im.data[i] = mag.data[i];
                
            }
            else {
                im.data[i] = mag.data[i];
            }
            
        }
    }
    return im;
}

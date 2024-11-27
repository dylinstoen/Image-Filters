#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "image.h"

float get_pixel(image im, int x, int y, int c)
{
    int _x = x;
    int _y = y;
    int _c = c;

    int W = im.w;
    int H = im.h;
    int C = im.c;

    if(im.data == NULL) {
        return 0.0f;
    }
    if(_x > W|| _x < 0) {
        if(_x < 0) {
            _x = 0;
        } else {
            _x = W - 1;
        }
        
    }
    if(_y > H || _y < 0) {
        if(_y < 0) {
            _y = 0;
        } else {
            _y = H - 1;
        }
    }
    if(_c > C || _c < 0) {
        if(_c < 0) {
            _c = 0;
        } else {
            _c = C - 1;
        }
    }

    float * pixel = im.data;
    int offset =  _c * H * W + _y * W + _x;
    return pixel[offset];
    
}

void set_pixel(image im, int x, int y, int c, float v)
{
       int _x = x;
    int _y = y;
    int _c = c;

    int W = im.w;
    int H = im.h;
    int C = im.c;

    if(im.data == NULL) {
        return;
    }
    if(_x > W|| _x < 0) {
        if(_x < 0) {
            _x = 0;
        } else {
            _x = W - 1;
        }
        
    }
    if(_y > H || _y < 0) {
        if(_y < 0) {
            _y = 0;
        } else {
            _y = H - 1;
        }
    }
    if(_c > C || _c < 0) {
        if(_c < 0) {
            _c = 0;
        } else {
            _c = C - 1;
        }
    }

    float * pixel = im.data;
    int offset =  _c * H * W + _y * W + _x;
    pixel[offset] = v;
}

image copy_image(image im)
{
    image copy = make_image(im.w, im.h, im.c);
    if(im.data != NULL) {
        float * _data = im.data;
        int size = im.w * im.h * im.c;
        for(int i = 0; i < size; i++) {
            copy.data[i] = _data[i]; 
        }
    }

    
    return copy;
}

image rgb_to_grayscale(image im)
{
    assert(im.c == 3);
    image gray = make_image(im.w, im.h, 1);
        if(im.data != NULL) {
        float * _data = im.data;
        int size = im.w * im.h;
        int zNear= im.w * im.h;
        int zFar = im.w * im.h * 2;
        for(int i = 0; i < size; i++) {
            float r = _data[i];
            float g =  _data[(i + zNear)];
            float b =  _data[(i + zFar)];
            float value = (r * 0.299f + g * 0.587f + b * 0.114f);
            gray.data[i] = value;
        }
    }
    return gray;
}

void shift_image(image im, int c, float v)
{
    // TODO Fill this in
        if(im.data != NULL) {
        float * _data = im.data;
        int cScale = c;
        if(v > 1.0f) {
            v = 1.0f;
        }
        if(v < 0.0f) {
            v = 0.0f;
        }
        if(cScale < 0 || cScale > im.c) {
            if(cScale < 0) {
                cScale = 1;
            } else {
                cScale = im.c - 1;
            }
        }
        int size = im.w * im.h;
        int z = im.w * im.h * cScale;
        for(int i = 0; i < size; i++) {
            _data[(i + z)] += v;
        }
    }
}

void clamp_image(image im)
{
    // TODO Fill this in
    int size = im.h * im.w * im.c;
    for(int i = 0; i < size; i++) {
        if(im.data[i] > 1.0f) {
            im.data[i] = 1.0f;
        }
        if(im.data[i] < 0.0f) {
            im.data[i] = 0.0f;
        }

    }
}


// These might be handy
float three_way_max(float a, float b, float c)
{
    return (a > b) ? ( (a > c) ? a : c) : ( (b > c) ? b : c) ;
}

float three_way_min(float a, float b, float c)
{
    return (a < b) ? ( (a < c) ? a : c) : ( (b < c) ? b : c) ;
}

void rgb_to_hsv(image im)
{
    assert(im.c == 3);
    if(im.data == NULL) {
        return;
    }
    float * _data = im.data;
    int n = im.w * im.h;
    int zNear = n;
    int zFar = n * 2;
    for(int i = 0; i < n; i++) {

    //Value
        float r = _data[i];
        float g =  _data[(i + zNear)];
        float b =  _data[(i + zFar)];
        float V = three_way_max(r, g, b);
        im.data[i + zFar] = V;
    //Saturation
        float m = three_way_min(r,g,b);
        float C = V - m;
        float S = 0.0f;
        if(V > 0.0f) {
            S = (C / V);
        }
        _data[(i + zNear)] = S;

        //Hue
        float H = 0.0f;

         if(C > 0.0f) {
             if(V == r) {
                 H = ((g - b)/C)/6.0f;
             } if(V == g) {
                H = (((b - r)/C) + 2.0f)/6.0f;
             }
             if (V == b) {
                 H = (((r - g)/C) + 4.0f)/6.0f;
             }
             if(H < 0.0f) {
                 H += 1;
             }
         }

         
         _data[i] = H;

    }


}

void hsv_to_rgb(image im)
{
    assert(im.c == 3);
    if(im.data == NULL) {
        return;
    }
    float * _data = im.data;
    int n = im.w * im.h;
    int zNear = n;
    int zFar = n * 2;

    for(int i = 0; i < n; i++) {

        float H = _data[i];
        float S =  _data[(i + zNear)];
        float V =  _data[(i + zFar)];
        
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;

        H *= 6.0f;

        float C = V * S;
        float X = C * (1 - abs(fmod((H / 60.0f), 2.0f) - 1.0f));
        float m = V - C;

        if(H >= 0 && H < 1.0f) {
            r = C;
            g = X;
            b = 0.0f;
        }
        if(H >= 1.0f && H < 2.0f) {
            r = X;
            g = C;
            b = 0.0f;
        }
        if(H >= 2.0f && H < 3.0f) {
            r = 0.0f;
            g = C;
            b = X;
        }
        if(H >= 3.0f && H < 4.0f) {
            r = 0.0f;
            g = X;
            b = C;
        }
        if(H >= 4.0f && H < 5.0f) {
            r = X;
            g = 0.0f;
            b = C;
        }
        if(H >= 5.0f && H < 6.0f) {
            r = C;
            g = 0.0f;
            b = X;
        }
        float R = (r + m);
        float G = (g + m);
        float B = (b + m);

        _data[i] = R;
        _data[(i + zNear)] = G;
        _data[(i + zFar)] = B;
    }

}

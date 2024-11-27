#include <math.h>
#include "image.h"

float nn_interpolate(image im, float x, float y, int c)
{
    if(x < 0.0f) {
        x = 0.0f;
    }
    if(y < 0.0f) {
        y = 0.0f;
    }
    int xPos =  roundf(x);
    int yPos = roundf(y);
    int offset =  c * im.h * im.w + yPos * im.w + xPos;
    
    return im.data[offset];
}

image nn_resize(image im, int w, int h)
{
    //Make new image
    image newImg = make_image(w, h, im.c);
    //Find a and b system of equations using new image start pos: (-0.5f, -0.5f) and end pos (newImg.w - 0.5f, newImage.h - 0.5f)
    // subtract X and Y values from a * X + b = Y, aX(1) - aX(2) = Y(1) - Y(2)

    // Linear equation for width
    float deltaX = (-0.5f - (((float) newImg.w) - 0.5f));
    float deltaY = (-0.5f - (((float) im.w) - 0.5f));
    // aX = Y, a = Y/X for width
    float ax = (deltaY/deltaX);
    //Plug a back in to get b by using a * -0.5f + b = -0.5f
    float bx = ((-0.5f) - (ax * (-0.5f)));

    // For height
    float _deltaX = (-0.5f - (((float) newImg.h) - 0.5f));
    float _deltaY = (-0.5f - (((float) im.h) - 0.5f));
    // aX = Y, a = Y/X for width
    float ay = (_deltaY/_deltaX);
    //Plug a back in to get b by using a * -0.5f + b = -0.5f
    float by = ((-0.5f) - (ay * (-0.5f)));


    //Iterate over new image points to map old points to the new ones
    //Once the new points are found nearest neighbor just rounds
    int offset = 0;
    
    for(int _c = 0; _c < newImg.c; _c++) {
        for(int _y = 0; _y < newImg.h; _y++) {
            for(int _x = 0; _x < newImg.w; _x++) {
                //Loop through new cords, map to old
                float xCord = (ax * ((float) _x) + bx);
                float yCord = (ay * ((float) _y) + by);
                newImg.data[offset] = nn_interpolate(im, xCord, yCord, _c);
                offset++;
            }
        }    
    }

    return newImg;

}

float bilinear_interpolate(image im, float x, float y, int c)
{
    //Clamp top left
    if(x <= 0.0f && y <= 0.0f) {
        int topLeftOffset = c * im.h * im.w;
        return im.data[topLeftOffset];
    }
    //Clamp top right
    if(x >= (im.w - 0.5f) && y <= 0.0f) {
        int topRightOffSet = c * im.h * im.w + (im.w - 1);
        return im.data[topRightOffSet];
    }
    //Clamp bot left
    if(x <= 0.0f && y >= (im.h - 0.5f)) {
        int botLeftOffset = c * im.h * im.w + (im.h - 1) * im.w;
        return im.data[botLeftOffset];
    }
    //Clamp bot right
    if(x > (im.w - 0.5f) && y >= (im.h - 0.5f)) {
        int clampBotRight =  (c * im.h * im.w + im.w * im.h) - 1;
        return im.data[clampBotRight];
    }
    //Clamp Top
    if(y <= 0.0f) {
        y = 0.5f; 
    }
    //Clamp bot
    if(y >= (im.h - 0.5f)) {
        y = (im.h - 0.5f);
    }
    //Clamp left
    if(x <= 0.0f) {
        x = 0.5f;
    }
    //Clamp right
    if(x >= (im.w - 0.5f)) {
        x = (im.w - 0.5f);
    }

    
    
    int leftX =  floorf(x);
    int ceilY = ceilf(y);

    int rightX = ceilf(x);
    int floorY =  floorf(y);

    //q1 and q2
    float weightTop = 1.0f - (ceilY - y);
    float weightBot = 1.0f - (y - floorY);

    //q1 leftx and ceil y, leftx and floor y
    int offsetTopLeft =  c * im.h * im.w + ceilY * im.w + leftX;
    int offsetBotLeft =  c * im.h * im.w + floorY * im.w + leftX;
    float q1 = (weightTop * (im.data[offsetTopLeft])) + (weightBot * (im.data[offsetBotLeft]));

    //q2 rightx and ceil y, rightx and floor y
    int offsetTopRight =  c * im.h * im.w + ceilY * im.w + rightX;
    int offsetBotRight =  c * im.h * im.w + floorY * im.w + rightX;
    float q2 = (weightTop * (im.data[offsetTopRight])) + (weightBot * (im.data[offsetBotRight]));

    // q1 and its x axis weight compared to q2
    float weightRight = 1.0f - (rightX - x);
    float weightLeft = 1.0f - (x - leftX);
    float value = (q1 * weightLeft) + (q2 * weightRight);

    return value;
}

image bilinear_resize(image im, int w, int h)
{
    //Make new image
    image newImg = make_image(w, h, im.c);
    //Find a and b system of equations using new image start pos: (-0.5f, -0.5f) and end pos (newImg.w - 0.5f, newImage.h - 0.5f)
    // subtract X and Y values from a * X + b = Y, aX(1) - aX(2) = Y(1) - Y(2)

    // Linear equation for width
    float deltaX = (-0.5f - (((float) newImg.w) - 0.5f));
    float deltaY = (-0.5f - (((float) im.w) - 0.5f));
    // aX = Y, a = Y/X for width
    float ax = (deltaY/deltaX);
    //Plug a back in to get b by using a * -0.5f + b = -0.5f
    float bx = ((-0.5f) - (ax * (-0.5f)));

    // For height
    float _deltaX = (-0.5f - (((float) newImg.h) - 0.5f));
    float _deltaY = (-0.5f - (((float) im.h) - 0.5f));
    // aX = Y, a = Y/X for width
    float ay = (_deltaY/_deltaX);
    //Plug a back in to get b by using a * -0.5f + b = -0.5f
    float by = ((-0.5f) - (ay * (-0.5f)));


    //Iterate over new image points to map old points to the new ones
    //Once the new points are found nearest neighbor just rounds
    int offset = 0;
    
    for(int _c = 0; _c < newImg.c; _c++) {
        for(int _y = 0; _y < newImg.h; _y++) {
            for(int _x = 0; _x < newImg.w; _x++) {
                //Loop through new cords, map to old
                float xCord = (ax * ((float) _x) + bx);
                float yCord = (ay * ((float) _y) + by);
                newImg.data[offset] = bilinear_interpolate(im, xCord, yCord, _c);
                offset++;
            }
        }    
    }

    return newImg;
}


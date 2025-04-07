#include <SDL_rect.h>
#include <math.h>
#include <stdbool.h>

// define M_PI if not already defined (useful for systems that don't provide it by default)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../include/rectangle_obb_corners.h"

// function to calculate the corners of a rotated rectangle (Oriented Bounding Box)
// based on a given bounding box and rotation angle (in degrees)
void corners_rotate(const SDL_Rect bounding_box, const float angle_degrees, SDL_FPoint corners[4]){
    // calculate half the width and height of the rectangle
    float half_w = bounding_box.w / 2.0f;
    float half_h = bounding_box.h / 2.0f;

    // calculate the center of the rectangle
    float cx = bounding_box.x + half_w;
    float cy = bounding_box.y + half_h;

    // offsets for each corner (top-left, top-right, bottom-right, bottom-left)
    float dx[] = {-half_w, half_w, half_w, -half_w};
    float dy[] = {-half_h, -half_h, half_h, half_h};

    // convert angle from degrees to radians
    float angle_radians = (angle_degrees * M_PI) / 180.0f;

    // calculate and store the rotated corners
    for(int i = 0; i < 4; i++){
        // rotate each corner by applying the 2D-rotation matrix
        corners[i].x = cx + (dx[i] * cosf(angle_radians) - dy[i] * sinf(angle_radians));
        corners[i].y = cy + (dx[i] * sinf(angle_radians) + dy[i] * cosf(angle_radians));
    }
}

// check if the corner is within the bounds of the other rectangle
bool corners_overlap(const SDL_FPoint corner, const SDL_Rect other_object){
    return ((corner.x >= other_object.x) && (corner.x <= other_object.x + other_object.w)
        && (corner.y >= other_object.y) && (corner.y <= other_object.y + other_object.h));
}
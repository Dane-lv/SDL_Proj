// #pragma once

// ensures the file is included only once in a single compilation to prevent multiple definitions
#ifndef RECTANGLE_OBB_CORNERS_H
#define RECTANGLE_OBB_CORNERS_H

// including SDL's rectangle definitions for handling SDL_Rect
#include <SDL_rect.h>
// including standard boolean type definition (true/false)
#include <stdbool.h>

// function declaration for calculating the rotated corners of a bounding box
// takes a bounding box (SDL_Rect), an angle (in degrees), and an array to store the rotated corners
void corners_rotate(const SDL_Rect bounding_box, const float angle_degrees, SDL_FPoint corners[4]);

// function declaration for checking if a corner of a rotated rectangle overlaps another rectangle
// takes a corner (SDL_FPoint), and another rectangle
bool corners_overlap(const SDL_FPoint corner, const SDL_Rect other_object);

#endif
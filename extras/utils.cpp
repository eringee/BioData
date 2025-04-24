// utils.cpp
#include "utils.h"

inline float clamp(float value, float min, float max) {
    return (value < min) ? min : (value > max) ? max : value;
}

inline float map(float value, float min1, float max1, float min2, float max2) {
    return (value - min1) / (max1 - min1) * (max2 - min2) + min2;
}

inline float min(float a, float b) {
    return (a < b) ? a : b; 
}

inline float max(float a, float b){
    return (a > b) ? a : b;
}
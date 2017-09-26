#pragma once
#include <vector>

extern void tessellate(std::vector<std::vector<float>> *contours_in, std::vector<float> *tris_out, 
                       std::vector<std::vector<float>> *edges, bool getBounds = false, float z = -0.9f);


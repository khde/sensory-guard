#ifndef DETECTION_H
#define DETECTION_H

#include <string>

struct BoundingBox {
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
};

struct DetectionResult {
    BoundingBox box;
    float score = 0.0f;
    std::string entity;
};

#endif
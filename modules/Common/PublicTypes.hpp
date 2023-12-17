#ifndef PUBLIC_TYPES_HPP_
#define PUBLIC_TYPES_HPP_

#include <iostream>

#include "Image.hpp"

// Define types that should be used across multiple modules or programs

typedef Image *ImagePtr;

enum class DistanceMetric
{
    MANHATTAN,
    EUCLIDEAN
};

class Neighbor
{
public:
    ImagePtr image;
    double distance;
    Neighbor() {}
    Neighbor(ImagePtr image, double distance) : image(image), distance(distance) {}
    ~Neighbor() {}
};

class CompareNeighbor
{
public:
    bool operator()(const Neighbor &a, const Neighbor &b) const
    {
        // Compare based on the double value (distance).
        return a.distance < b.distance;
    }
};

#endif
#ifndef IMAGE_HPP_
#define IMAGE_HPP_

#include <iostream>
#include <vector>

// Defines a data point of a MNIST dataset. The id is stored to later indentify neighbors and images found in the range search
// Pixels is stored as double due to clustering which creates data points with non-existing coordinates from the dataset
class Image
{
public:
    int id;
    std::vector<double> pixels;

    Image(int id, const std::vector<double> &pixels) : id(id), pixels(pixels) {}

    Image() {}

    ~Image() {}
};

#endif
#include <iostream>
#include <vector>
#include <cmath>

#include "ImageDistance.hpp"

// Initialize static variables
ImageDistance ImageDistance::instance;
DistanceMetric ImageDistance::metric;
bool ImageDistance::isMetricSet = false;

ImageDistance::ImageDistance() {}

ImageDistance::~ImageDistance() {}

// Configures the metric. Call this method only once before getInstance
void ImageDistance::setMetric(DistanceMetric inputMetric)
{
    if (isMetricSet)
    {
        std::cerr << "ImageDistance: Metric can only be set once" << std::endl;
        exit(EXIT_FAILURE);
    }
    metric = inputMetric;
    isMetricSet = true;
}

// Get access to distance helper in order to call calculate. The first time it is called, the instance gets created
ImageDistance *ImageDistance::getInstance()
{
    if (!isMetricSet)
    {
        std::cerr << "Metric must be set before calling getInstance" << std::endl;
        exit(EXIT_FAILURE);
    }
    return &instance;
}

// choose between euclidean and manhattan distances depending on the configuration of the metric
double ImageDistance::calculate(const ImagePtr &first, const ImagePtr &second)
{
    if (metric == DistanceMetric::EUCLIDEAN)
    {
        return EuclideanImageDistance(first, second);
    }
    else if (metric == DistanceMetric::MANHATTAN)
    {
        return ManhattanImageDistance(first, second);
    }

    std::cerr << "ImageDistance: unexpected error. Metric is invalid" << std::endl;
    exit(EXIT_FAILURE);
}

double ImageDistance::EuclideanImageDistance(const ImagePtr &first, const ImagePtr &second)
{
    double difference, result = 0.0;
    size_t limit = first->pixels.size();
    for (size_t i = 0; i < limit; i++)
    {
        difference = first->pixels[i] - second->pixels[i];
        result += difference * difference; // squared differences
    }
    return sqrt(result);
}

double ImageDistance::ManhattanImageDistance(const ImagePtr &first, const ImagePtr &second)
{
    double result = 0;
    size_t limit = first->pixels.size();
    for (size_t i = 0; i < limit; i++)
        result += std::abs(first->pixels[i] - second->pixels[i]);
    return result;
}

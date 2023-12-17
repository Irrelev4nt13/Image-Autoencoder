#ifndef GNNS_HPP_
#define GNNS_HPP_

#include <vector>
#include "Image.hpp"
#include "ImageDistance.hpp"
#include "GraphAlgorithm.hpp"
/**
 * @brief The class of a GNNS consists of the following
 *
 * @param graphNN the number of nearest neighbors for lsh
 * @param expansions the number of expansions to find Y_t
 * @param restarts the number of restart which starts from a random point
 * @param numNn the number of nearest neighbors needed
 *
 * @method Approximate_kNN returns a vector with numNn aproxximate nearest neighbors
 */
class GNNS : public GraphAlgorithm
{
private:
    int graphNN;
    int expansions;
    int restarts;
    int numNn;
    ImageDistance *distance;
    std::vector<std::vector<ImagePtr>> PointsWithNeighbors;

public:
    GNNS(const std::vector<ImagePtr> &images, int graphNN, int expansions, int restarts, int numNn);
    ~GNNS();
    std::vector<Neighbor> Approximate_kNN(ImagePtr query);
};

#endif
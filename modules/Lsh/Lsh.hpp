#ifndef LSH_HPP_
#define LSH_HPP_

#include <vector>

#include "Image.hpp"
#include "HashTable.hpp"
#include "ImageDistance.hpp"
/**
 * @brief The class of a lsh consists of the following
 *
 * @param numHashFuncs the dimension k = d' of hypercube
 * @param numHtables the maximum number of points the algorithm will check
 * @param probes the number of probes the algorithm will check (we check the initial bucket + probes which in reality is probes + 1)
 * @param numNn the number of nearest neighbors needed
 * @param w the window
 * @param numBuckets the number of buckets which will be used
 * @param hashtables this algorithm requires many hashtables, so we have a vector with objects HashTable which are essentially our own implementation to match our needs
 * @param distance the generic distance
 *
 * @method Approximate_kNN returns a vector with numNn aproxximate nearest neighbors
 * @method Approximate_Range_Search returns a vector with points inside the given radius
 */
class Lsh
{
private:
    int numHashFuncs;                  // –k number of hash functions
    int numHtables;                    // -L number of hash tables
    int numNn;                         // -Ν number of Nearest Neighbors
    int w;                             // w
    int numBuckets;                    // number of buckets
    std::vector<HashTable> hashtables; // hash tables
    ImageDistance *distance;

public:
    Lsh(const std::vector<ImagePtr> &images, int numHashFuncs, int numHtables, int numNn, int w, int numBuckets);
    ~Lsh();
    std::vector<Neighbor> Approximate_kNN(ImagePtr query);
    std::vector<ImagePtr> Approximate_Range_Search(ImagePtr query, const double radius);
};

#endif
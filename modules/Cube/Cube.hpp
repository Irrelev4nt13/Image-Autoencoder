#ifndef CUBE_HPP_
#define CUBE_HPP_

#include <vector>
#include <unordered_map>

#include "PublicTypes.hpp"
#include "Image.hpp"
#include "HashFunction.hpp"
#include "ImageDistance.hpp"

/**
 * @brief The class of a cube consists of the following
 *
 * @param dimension the dimension k = d' of hypercube
 * @param maxCanditates the maximum number of points the algorithm will check
 * @param probes the number of probes the algorithm will check (we check the initial bucket + probes which in reality is probes + 1)
 * @param numNn the number of nearest neighbors needed
 * @param w the window
 * @param numBuckets the number of buckets which will be used which is essentially 2^k since {0,1}^d'
 * @param buckets the buckets in a vector form
 * @param map the map to match f_i(h_i()) values
 * @param hash_functions the h_i functions that are used
 * @param distance the generic distance
 *
 * @method hash utilizies the h_i functions to insert an image
 * @method insert inserts an image into the buckets according to the hash
 * @method Approximate_kNN returns a vector with numNn aproxximate nearest neighbors
 * @method Approximate_Range_Search returns a vector with points inside the given radius
 */
class Cube
{
private:
    int dimension;     // –k number of dimensions
    int maxCanditates; // -M maximum candidates
    int probes;        // -p probes
    int numNn;         // -Ν number of nearest Neighbors
    int w;
    int numBuckets;
    std::vector<std::vector<ImagePtr>> buckets;
    std::unordered_map<int, int> *map;
    std::vector<HashFunction> hash_functions;
    int hash(ImagePtr image);
    ImageDistance *distance;

public:
    Cube(const std::vector<ImagePtr> images, int w, int dimension, int maxCanditates, int probes, int numNn, int numBuckets);
    ~Cube();
    void insert(ImagePtr image);
    std::vector<Neighbor> Approximate_kNN(ImagePtr query);
    std::vector<ImagePtr> Approximate_Range_Search(ImagePtr query, const double radius);
};

#endif

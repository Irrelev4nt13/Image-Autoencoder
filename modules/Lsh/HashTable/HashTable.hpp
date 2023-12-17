#ifndef HASHTABLE_HPP_
#define HASHTABLE_HPP_

#include <vector>
#include <unordered_map>

#include "Image.hpp"
#include "HashFunction.hpp"
#include "PublicTypes.hpp"

/**
 * @brief The class of a amplified lsh function (g_i) consists of the following
 *
 * @param r the random vector
 * @param hash_functions all the hash_functions h_i that are used
 *
 * @method hash utilizies the h_i functions and a combination of the r vector to insert an image
 */
class AmpLsh
{
private:
    std::vector<int> r;
    std::vector<HashFunction> hash_functions;

public:
    AmpLsh(int w, int numHashFuncs, int dimension);
    ~AmpLsh();

    int hash(ImagePtr image);
};

/**
 * @brief The class of a HashTable consists of the following
 *
 * @param numBuckets the number of buckets which will be used which is essentially 2^k since {0,1}^d'
 * @param buckets the buckets in a 2d vector form, the first vector is for the buckets and the second for the elements of each bucket
 * @param hashmap the amplified hash function for the hashtable
 *
 * @method insert inserts an image into the buckets according to the hash
 * @method get_bucket returns the bucket for the given image
 */
template <typename T>
using Bucket = std::vector<T>;

class HashTable
{
private:
    int numBuckets;
    std::vector<Bucket<ImagePtr>> buckets;
    AmpLsh hashmap;

public:
    HashTable(int numBuckets, const AmpLsh &hashmap);
    ~HashTable();

    void insert(ImagePtr image);

    std::vector<ImagePtr> get_bucket(ImagePtr image);
};

#endif
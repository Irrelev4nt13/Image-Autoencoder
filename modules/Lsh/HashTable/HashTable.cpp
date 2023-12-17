#include <vector>
#include <unordered_map>
#include <string>
#include <climits>
#include <cstdint>

#include "Utils.hpp"
#include "HashTable.hpp"

// That's the number from the slides
static const int64_t M = ((int64_t)(1) << 32) - 5;

// The constructor of the amplified function g_i
AmpLsh::AmpLsh(int w, int numHashFuncs, int dimension)
{
    // We are using num hash functions h_i
    for (int i = 0; i < numHashFuncs; i++)
    {
        // We are gradually creating the r vector for the amplified
        r.push_back(IntDistribution(0, INT32_MAX));

        // But we are also creating different v vector for each hash_function
        std::vector<double> v;
        for (int j = 0; j < dimension; j++)
            v.push_back(NormalDistribution(0.0, 1.0));
        // The RealDistribution is the t (shift)
        hash_functions.push_back(HashFunction(w, RealDistribution(0, w), v));
    }
}
AmpLsh::~AmpLsh() {}

// Utilizes the respective hash_functions with r to get the sum of their multiplications. Then we take the modulo of it with M.
int AmpLsh::hash(ImagePtr image)
{
    uint64_t hashval = 0;
    for (int i = 0, num_hashes = hash_functions.size(); i < num_hashes; i++)
        hashval += r[i] * hash_functions[i].hash(image);
    return Modulo(hashval, M);
}

// We are making a HashTable object with numBuckets empty buckets
HashTable::HashTable(int numBuckets, const AmpLsh &hash) : numBuckets(numBuckets), hashmap(hash)
{
    buckets.resize(numBuckets);
    for (int i = 0; i < numBuckets; i++)
    {
        std::vector<ImagePtr> new_bucket;
        buckets[i] = new_bucket;
    }
}

HashTable::~HashTable() {}

// To insert the image we are using the hash with mod table_size as were showed in slides
void HashTable::insert(ImagePtr image)
{
    buckets.at(Modulo(hashmap.hash(image), numBuckets)).push_back(image);
}

// Returns the bucket of the given image with the formula used to insert the image
std::vector<ImagePtr> HashTable::get_bucket(ImagePtr image) { return buckets.at(Modulo(hashmap.hash(image), numBuckets)); }

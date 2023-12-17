#include <vector>
#include <queue>
#include <bitset>

#include "Utils.hpp"
#include "Cube.hpp"
#include "PublicTypes.hpp"
#include "HashFunction.hpp"
#include "ImageDistance.hpp"

// Constructor for cube object, uses initialization list
Cube::Cube(const std::vector<ImagePtr> images, int w, int dimension, int maxCanditates, int probes, int numNn, int numBuckets)
    : dimension(dimension), maxCanditates(maxCanditates), probes(probes), numNn(numNn), w(w), numBuckets(numBuckets)
{
    // Initialize the general distance
    this->distance = ImageDistance::getInstance();

    // We make num of dimension hash_functions as were showed in slides
    for (int i = 0; i < dimension; i++)
    {
        std::vector<double> v;
        // Create the v vector
        for (std::size_t j = 0; j < images[0]->pixels.size(); j++)
            v.push_back(NormalDistribution(0.0, 1.0));

        // The RealDistribution is the t (shift)
        hash_functions.push_back(HashFunction(w, RealDistribution(0, w), v));
    }

    // Initially we have numBuckets empty buckets
    for (int i = 0; i < numBuckets; i++)
        buckets.push_back(std::vector<ImagePtr>());

    // We have a pointer to an unordered_map for f_i(h_i()) values
    map = new std::unordered_map<int, int>[dimension];

    // We begin filling the buckets with images
    for (std::size_t i = 0; i < images.size(); i++)
        insert(images[i]);
}

// Utilizes the respective hash_functions to make a string consisting from 0 and 1. Then we convert this binary number into decimal
// and the index to the bucket the current image will be inserted
int Cube::hash(ImagePtr image)
{
    std::string res = "";
    for (int i = 0; i < dimension; i++)
    {
        int hash = hash_functions[i].hash(image);
        if (map[i].find(hash) == map[i].end())
            map[i][hash] = IntDistribution(0, 1);

        res += std::to_string(map[i][hash]);
    }
    return std::bitset<32>(res).to_ulong();
}

// Free allocated memory for map
Cube::~Cube() { delete[] map; }

// Insert the current image to the bucket showed from hash
void Cube::insert(ImagePtr image) { buckets[hash(image)].push_back(image); }

// Returns the k approximate nearest neighbors
std::vector<Neighbor> Cube::Approximate_kNN(ImagePtr query)
{
    // We are using a priority queue to store the objects efficiently with a custom compare class
    std::priority_queue<Neighbor, std::vector<Neighbor>, CompareNeighbor> nearestNeighbors;

    // We get the bucket that the query would be inserted to in order to search there
    int query_bucket = hash(query);
    int candidates = 0;
    std::vector<ImagePtr> bucket;
    int hamDistance = 0;

    // This loop will run until one of the conditions are satisfied, either the number of probes that was searched is reached or the number of candidates is reached
    for (int i = 0; i < probes + 1 && candidates < maxCanditates; hamDistance++)
    {
        // This is the query bucket we are searching in
        if (i == 0)
        {
            bucket = buckets[query_bucket];

            // Iterate over all images for the current bucket
            for (ImagePtr input : bucket)
            {
                // We calculate the distance from this image to the query
                double dist = distance->calculate(input, query);
                // Push it to the priority queue
                nearestNeighbors.push(Neighbor(input, dist));
                // In order to save time later we only store numNn of approximate nearest neighbors
                if ((int)nearestNeighbors.size() > numNn)
                    nearestNeighbors.pop();
                // If the number of candidates is reached stop the loop
                if (++candidates == maxCanditates)
                    break;
            }
            // We are letting the loop know that we are moving to the next bucket
            i++;
        }
        else
        {
            // For every bucket
            for (int j = 0; j < numBuckets && candidates < maxCanditates && i < probes + 1; j++)
            {
                // We examine its hamming distance from the query
                if (HammingDistance(query_bucket, j) == hamDistance)
                {
                    bucket = buckets[j];
                    // Iterate over all images for the current bucket
                    for (ImagePtr input : bucket)
                    {
                        // We calculate the distance from this image to the query
                        double dist = distance->calculate(input, query);
                        // Push it to the priority queue
                        nearestNeighbors.push(Neighbor(input, dist));
                        // In order to save time later we only store numNn of approximate nearest neighbors
                        if ((int)nearestNeighbors.size() > numNn)
                            nearestNeighbors.pop();
                        // If the number of candidates is reached stop the loop
                        if (++candidates == maxCanditates)
                            break;
                    }
                    // We also let know the loop that we searched one more bucket
                    ++i;
                }
            }
        }
    }

    // Lastly we want to make a vector from those neighbors
    std::vector<Neighbor> KnearestNeighbors;
    while (!nearestNeighbors.empty())
    {
        // We insert it in the beginning of the vector because the elements in priority queue are stored in descending order based on their distances
        KnearestNeighbors.insert(KnearestNeighbors.begin(), nearestNeighbors.top());
        nearestNeighbors.pop();
    }
    return KnearestNeighbors;
}

// Returns a vector with images inside the given radius
std::vector<ImagePtr> Cube::Approximate_Range_Search(ImagePtr query, const double radius)
{
    std::vector<ImagePtr> RangeSearch;

    int query_bucket = hash(query);
    int candidates = 0;
    std::vector<ImagePtr> bucket;
    int hamDistance = 0;
    // This loop will run until one of the conditions are satisfied, either the number of probes that was searched is reached or the number of candidates is reached
    for (int i = 0; i < probes + 1 && candidates < maxCanditates; hamDistance++)
    {
        // This is the query bucket we are searching in
        if (i == 0)
        {
            bucket = buckets[query_bucket];
            // Iterate over all images for the current bucket
            for (ImagePtr input : bucket)
            {
                // We calculate the distance from this image to the query
                double dist = distance->calculate(input, query);
                // If its distance is less or equal to the given radius
                if (dist <= radius)
                    // We push it to the vector
                    RangeSearch.push_back(input);
                // If the number of candidates is reached stop the loop
                if (++candidates == maxCanditates)
                    break;
            }
            // We are letting the loop know that we are moving to the next bucket
            i++;
        }
        else
        {
            // For every bucket
            for (int j = 0; j < numBuckets && candidates < maxCanditates && i < probes + 1; j++)
            {
                // We examine its hamming distance from the query
                if (HammingDistance(query_bucket, j) == hamDistance)
                {
                    bucket = buckets[j];
                    // Iterate over all images for the current bucket
                    for (ImagePtr input : bucket)
                    {
                        // We calculate the distance from this image to the query
                        double dist = distance->calculate(input, query);
                        // If its distance is less or equal to the given radius
                        if (dist <= radius)
                            // We push it to the vector
                            RangeSearch.push_back(input);
                        // If the number of candidates is reached stop the loop
                        if (++candidates == maxCanditates)
                            break;
                    }
                    // We also let know the loop that we searched one more bucket
                    ++i;
                }
            }
        }
    }
    return RangeSearch;
}
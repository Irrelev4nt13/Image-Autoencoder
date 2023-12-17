#include <iostream>
#include <vector>
#include <queue>
#include <set>
#include <unordered_set>

#include "Image.hpp"
#include "Utils.hpp"
#include "HashTable.hpp"
#include "Lsh.hpp"
#include "PublicTypes.hpp"
#include "ImageDistance.hpp"

// Constructor for lsh object, uses initialization list
Lsh::Lsh(const std::vector<ImagePtr> &images, int numHashFuncs, int numHtables, int numNn, int w, int numBuckets)
    : numHashFuncs(numHashFuncs), numHtables(numHtables), numNn(numNn), w(w), numBuckets(numBuckets)
{
  // Initialize the general distance
  this->distance = ImageDistance::getInstance();

  int dimension = images.at(0)->pixels.size();
  // We need num hash tables
  for (int i = 0; i < numHtables; i++)
  {
    // we make one HashTable object and push it to the vector
    hashtables.push_back(HashTable(numBuckets, AmpLsh(w, numHashFuncs, dimension)));

    // Then we initialize it with every image
    for (std::size_t j = 0; j < images.size(); j++)
      hashtables[i].insert(images[j]);
  }
}

Lsh::~Lsh() {}

// Returns the k approximate nearest neighbors
std::vector<Neighbor> Lsh::Approximate_kNN(ImagePtr query)
{
  // We are using a set to store the objects efficiently with a custom compare class
  std::set<Neighbor, CompareNeighbor> nearestNeighbors;

  // We are searching in every hash table
  for (int i = 0; i < numHtables; i++)
  {
    // We are getting the current bucket for the query
    const std::vector<ImagePtr> bucket = hashtables[i].get_bucket(query);

    // Iterate over all images of the bucket
    for (ImagePtr input : bucket)
    {
      // We calculate the distance from this image to the query
      double dist = distance->calculate(input, query);
      // Push it to the set which will automatically check for duplicates
      nearestNeighbors.insert(Neighbor(input, dist));

      // In order to save space we only store numNn of approximate nearest neighbors
      if ((int)nearestNeighbors.size() > numNn)
        nearestNeighbors.erase(std::prev(nearestNeighbors.end()));
    }
  }
  // Lastly we want to make a vector from those neighbors
  std::vector<Neighbor> KnearestNeighbors(nearestNeighbors.begin(), nearestNeighbors.end());
  return KnearestNeighbors;
}

// Returns a vector with images inside the given radius
std::vector<ImagePtr> Lsh::Approximate_Range_Search(ImagePtr query, const double radius)
{
  // We are using a set to store the objects efficiently without duplicates
  std::set<ImagePtr> rangesearch;
  // We are searching in every hash table
  for (int i = 0; i < numHtables; i++)
  {
    // We are getting the current bucket for the query
    const std::vector<ImagePtr> bucket = hashtables[i].get_bucket(query);

    // Iterate over all images of the bucket
    for (ImagePtr input : bucket)
    {
      // We calculate the distance from this image to the query
      double dist = distance->calculate(input, query);
      // If its distance is less or equal to the given radius
      if (dist <= radius)
        // We push it to the vector
        rangesearch.insert(input);
    }
  }
  std::vector<ImagePtr> RangeSearch(rangesearch.begin(), rangesearch.end());
  return RangeSearch;
}
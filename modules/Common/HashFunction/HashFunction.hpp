#ifndef HASH_FUNCTION_HPP_
#define HASH_FUNCTION_HPP_

#include <iostream>
#include <vector>
#include "PublicTypes.hpp"

// Class to store parameters of a hash function. Use hash method to hash with the given parameters in the constructor
class HashFunction
{
private:
    int w;                 // window
    double t;              // uniform distribution 0 to window
    std::vector<double> v; // normal distribution 0.0 to 1.0

public:
    HashFunction(int w, double t, const std::vector<double> &v);
    ~HashFunction();

    uint64_t hash(ImagePtr image) const;
};

#endif

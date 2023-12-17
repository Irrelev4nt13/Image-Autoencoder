#ifndef SEARCH_ALGORITHM_HPP_
#define SEARCH_ALGORITHM_HPP_

#include <vector>
#include "PublicTypes.hpp"

// Search Algorithm interface
class GraphAlgorithm
{
public:
    virtual ~GraphAlgorithm() = default;
    virtual std::vector<Neighbor> Approximate_kNN(ImagePtr query) = 0;
};

#endif

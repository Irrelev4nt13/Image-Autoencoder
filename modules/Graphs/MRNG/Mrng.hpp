#include <iostream>
#include <vector>

#include "PublicTypes.hpp"
#include "ImageDistance.hpp"
#include "GraphAlgorithm.hpp"
#include "Lsh.hpp"

class Mrng : public GraphAlgorithm
{
private:
    int numNn;
    int candidates;
    ImageDistance *distHelper;
    ImagePtr navNode;
    std::vector<std::vector<ImagePtr>> graph;

public:
    Mrng(const std::vector<ImagePtr> &images, int numNn, int l);
    ~Mrng();
    std::vector<Neighbor> Approximate_kNN(ImagePtr query);
};

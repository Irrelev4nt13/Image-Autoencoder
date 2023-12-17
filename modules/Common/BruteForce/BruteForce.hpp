#ifndef BRUTEFORCE_HPP_
#define BRUTEFORCE_HPP_

#include <vector>

#include "Image.hpp"
#include "PublicTypes.hpp"

std::vector<Neighbor> BruteForce(const std::vector<ImagePtr> &images_input, const ImagePtr query, const int k);

#endif
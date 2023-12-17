
#include <iostream>
#include <vector>

#include "HashFunction.hpp"
#include "Utils.hpp"
#include "PublicTypes.hpp"

HashFunction::HashFunction(int w, double t, const std::vector<double> &v) : w(w), t(t), v(v) {}

HashFunction::~HashFunction() {}

uint64_t HashFunction::hash(ImagePtr image) const { return floor((DotProduct(v, image->pixels) + t) / (double)w); }

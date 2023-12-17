#ifndef UTILS_HPP_
#define UTILS_HPP_

#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <cstdint>

#include "PublicTypes.hpp"

void readFilenameIfEmpty(std::string &filename, std::string fileType);

#ifdef DEBUG
std::string getFullPath(std::string relativePath);
#endif

void startClock();

std::chrono::nanoseconds stopClock();

double DotProduct(const std::vector<double> &first, const std::vector<double> &second);

template <typename T, typename U>
uint8_t Modulo(T a, U b) { return static_cast<uint8_t>(a % b); }

int binarySearch(const std::vector<double> &probs, double x);

int HammingDistance(const int first, const int second);

double RealDistribution(int from, int to);

int IntDistribution(int from, int to);

double NormalDistribution(double from, double to);

#endif
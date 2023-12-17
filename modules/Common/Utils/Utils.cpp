#include <iostream>
#include <vector>
#include <random>
#include <cstdint>
#include <cmath>

#include <chrono>

#include "Utils.hpp"

#ifdef DEBUG
#include <unistd.h>
#include <limits.h>
#endif

/**
 * @brief Waits for user input for a filename in case the file is already empty.
 *
 * @param filename name of file that will be read
 * @param fileType category of file to help user input the correct file
 */
void readFilenameIfEmpty(std::string &filename, std::string fileType)
{
    if (filename.empty())
    {
        std::cout << "Enter " << fileType << " file: ";
        std::getline(std::cin, filename);
    }
}

/**
 * @brief Finds the full path in the filesystem from a given relative path of an existing file
 * @param relativePath relative path to the file
 */
#ifdef DEBUG
std::string getFullPath(std::string relativePath)
{
    std::string cwd;
    char buffer[PATH_MAX];
    if (getcwd(buffer, sizeof(buffer)) != nullptr)
    {
        cwd = buffer;
    }

    // cwd gets the full path of the file from which the debugger was started
    // run debugger from Makefile for 0 parent folders
    int numParents = 0;
    std::string fullPath = cwd;
    for (int i = 0; i < numParents; i++)
    {
        size_t pos = fullPath.rfind('/');
        if (pos != std::string::npos)
        {
            fullPath = fullPath.substr(0, pos);
        }
    }
    return fullPath + "/" + relativePath;
}
#endif

static std::chrono::high_resolution_clock::time_point startTime;

// Call before stop clock to start clock timer. Call more than once only if the clock is already stopped with stop clock
void startClock() { startTime = std::chrono::high_resolution_clock::now(); }

// Call after start clock and get the duration
std::chrono::nanoseconds stopClock()
{
    std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();

    std::chrono::nanoseconds duration = std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime);

    return duration;
}

double DotProduct(const std::vector<double> &first, const std::vector<double> &second)
{
    double sum = 0.0;
    for (size_t i = 0; i < first.size(); ++i)
        sum += first[i] * second[i];
    return sum;
}

/**
 * @brief Binary search in probalities to find the index corresponding to x
 *
 * @param partial partial sums used in KmeansPlusPlus
 * @param x must be uniform real number from 0 to last of partial
 * @return index to partial sums based on x
 */
int binarySearch(const std::vector<double> &partial, double x)
{
    int l = 0, r = partial.size() - 1;
    while (l <= r)
    {
        int mid = l + (r - l) / 2;
        if (partial[mid] < x)
        {
            l = mid + 1;
        }
        else
        {
            r = mid - 1;
        }
    }
    return l;
}

int HammingDistance(const int first, const int second)
{
    int xorResult = first ^ second;
    int distance = 0;

    while (xorResult > 0)
    {
        // Increment the distance for each set by 1 bit.
        distance += xorResult & 1;

        // Right-shift the XOR result to check the next bit.
        xorResult >>= 1;
    }

    return distance;
}

static std::random_device generator;

double RealDistribution(int from, int to)
{
    std::uniform_real_distribution<double> rd(from, to);
    return rd(generator);
}

int IntDistribution(int from, int to)
{
    std::uniform_int_distribution<int> id(from, to);
    return id(generator);
}

double NormalDistribution(double from, double to)
{
    std::normal_distribution<double> nd(from, to);
    return nd(generator);
}
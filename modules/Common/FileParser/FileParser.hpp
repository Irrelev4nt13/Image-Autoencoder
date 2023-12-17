#ifndef FILEPARSER_HPP_
#define FILEPARSER_HPP_

#include <vector>

#include "Image.hpp"
#include "PublicTypes.hpp"

/**
 * @brief Stores the metadata of a MNIST dataset
 *
 */
class Metadata
{
public:
    uint32_t magicNumber;
    uint32_t numOfImages;
    uint32_t numOfRows;
    uint32_t numOfColumns;
};

/**
 * @brief Parses a file of a MNIST dataset and stores its metadata and data points
 * @param inputFile filename of dataset
 */
class FileParser
{
private:
    Metadata metadata;
    std::vector<ImagePtr> images;

public:
    FileParser(std::string inputFile, int size = -1);
    ~FileParser();
    inline const Metadata &GetMetadata() const { return metadata; }
    inline const std::vector<ImagePtr> &GetImages() const { return images; }
};

#endif
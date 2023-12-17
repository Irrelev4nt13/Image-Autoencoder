#include <vector>
#include <fstream>
#include <arpa/inet.h>

#include "Image.hpp"
#include "FileParser.hpp"

#ifdef DEBUG
#include <unistd.h>
#include <limits.h>
#include "Utils.hpp"
#endif

FileParser::FileParser(std::string inputFile, int size)
{

#ifdef DEBUG
    inputFile = getFullPath(inputFile);
#endif

    std::ifstream file(inputFile, std::ios::binary);

    if (!file.is_open())
    {
        std::cerr << "Failed to open the file." << std::endl;
        file.close();
        exit(EXIT_FAILURE);
    }

    if (!file.read((char *)&metadata, sizeof(Metadata)))
    {
        std::cerr << "Failed to read the header." << std::endl;
        file.close();
        exit(EXIT_FAILURE);
    }

    metadata.magicNumber = ntohl(metadata.magicNumber);
    if (size == -1)
        metadata.numOfImages = ntohl(metadata.numOfImages);
    else
        metadata.numOfImages = size;
    metadata.numOfRows = ntohl(metadata.numOfRows);
    metadata.numOfColumns = ntohl(metadata.numOfColumns);

    uint32_t image_size = metadata.numOfRows * metadata.numOfColumns;

    images.resize(metadata.numOfImages);
    uint8_t *buffer = new uint8_t[image_size];

    for (std::size_t i = 0; i < images.size(); i++)
    {
        images[i] = new Image;
        images[i]->pixels.resize(image_size);
        images[i]->id = i;

        if (!file.read((char *)buffer, image_size))
        {
            std::cerr << "Failed to read image data." << std::endl;
            file.close();
            exit(EXIT_FAILURE);
        }

        for (std::size_t j = 0; j < image_size; ++j)
        {
            images[i]->pixels[j] = static_cast<double>(buffer[j]);
        }
    }

    delete[] buffer;

    file.close();
}

FileParser::~FileParser()
{
    for (const ImagePtr image_ptr : images)
        delete image_ptr;
}

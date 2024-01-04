#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

#include "BruteForce.hpp"
#include "CubeCmdArgs.hpp"
#include "Cube.hpp"
#include "FileParser.hpp"
#include "Utils.hpp"
#include "ImageDistance.hpp"

int main(int argc, char const *argv[])
{
    // Analyze arguments from command line and store them in a simple object
    CubeCmdArgs args(argc, argv);

    readFilenameIfEmpty(args.inputFile, "input");

    // Parse file and get the images
    FileParser inputParser(args.inputFile, args.in_size);
    const std::vector<ImagePtr> input_images = inputParser.GetImages();

    readFilenameIfEmpty(args.queryFile, "query");

    readFilenameIfEmpty(args.outputFile, "output");
    std::ofstream output_file;

    // window
    int w = 2240;       // default window
    int approxAvg = 60; // approximate average pixel distance

    if (args.hasLatentDim)
    {
        w = sqrt(approxAvg * approxAvg * inputParser.GetMetadata().numOfRows); // for latent space
    }

    int numBuckets = std::pow(2, args.dimension); // {0,1}^d'=> 2^k

    // Configure the metric used for the lsh program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);

    // Initialize cube structure
    Cube cube(input_images, w, args.dimension, args.maxCanditates, args.probes, args.numNn, numBuckets);

    auto tTotalApproximate = std::chrono::nanoseconds(0);
    auto tTotalTrue = std::chrono::nanoseconds(0);
    double AAF = 0;
    int found = 0;

    // Keep reading new query and output files until the user types "exit"
    while (true)
    {
        // Get query images
        FileParser queryParser(args.queryFile, args.q_size);
        std::vector<ImagePtr> query_images = queryParser.GetImages();

        output_file.open(args.outputFile);

        // For each query data point calculate its approximate k nearesest neighbors with hypercube algorithm and compare it to brute force
        // Also, make a range search with hypercube
        for (int q = 0; q < args.q_size; q++)
        {
            ImagePtr query = query_images[q];

            startClock();
            std::vector<Neighbor> approx_vector = cube.Approximate_kNN(query);
            auto elapsed_cube = stopClock();
            tTotalApproximate += elapsed_cube;

            startClock();
            std::vector<Neighbor> brute_vector = BruteForce(input_images, query, args.numNn);
            auto elapsed_brute = stopClock();
            tTotalTrue += elapsed_brute;

            output_file << "Query: " << query->id << std::endl;

            int limit = approx_vector.size();
            for (int i = 0; i < limit; i++)
            {
                ImagePtr image = approx_vector[i].image;
                double approxDist = approx_vector[i].distance;

                output_file << "Nearest neighbor-" << i + 1 << ": " << image->id << std::endl
                            << "distanceLSH: " << approxDist << "\n";

                double trueDist = brute_vector[i].distance;
                output_file << "distanceTrue: " << trueDist << "\n";

                AAF += approxDist / trueDist;
            }

            found += limit;

            output_file << "tHypercube: " << elapsed_cube.count() * 1e-9 << std::endl;
            output_file << "tTrue: " << elapsed_brute.count() * 1e-9 << std::endl;

            output_file << std::endl;
        }

        output_file << "tAverageApproximate: " << tTotalApproximate.count() * 1e-9 / 100 << std::endl; // Average Approximate time
        output_file << "tAverageTrue: " << tTotalTrue.count() * 1e-9 / 100 << std::endl;               // Average True time
        output_file << "AAF: " << AAF / found << std::endl;                                            // Average Approximation Factor

        // Read new query and output files.
        args.queryFile.clear();
        std::cout << "Enter new query file, type exit to stop: ";
        std::getline(std::cin, args.queryFile);

        if (args.queryFile == "exit")
            break;

        args.outputFile.clear();
        readFilenameIfEmpty(args.outputFile, "output");

        output_file.close();
    }

    return EXIT_SUCCESS;
}
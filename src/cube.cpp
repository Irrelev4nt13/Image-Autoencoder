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

    int approxAvgDist = 50; // approximate average pixel distance in latent space

    // window
    int w = sqrt(approxAvgDist * approxAvgDist * inputParser.GetMetadata().numOfRows * inputParser.GetMetadata().numOfColumns);

    FileParser iniDatasetParser = FileParser(args.initDataset, args.in_size);
    std::vector<ImagePtr> initImages = iniDatasetParser.GetImages();

    FileParser initQuerysetParser = FileParser(args.initQueryset, args.q_size);
    std::vector<ImagePtr> initQueryImages = initQuerysetParser.GetImages();

    int numBuckets = std::pow(2, args.dimension); // {0,1}^d'=> 2^k

    // Configure the metric used for the lsh program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);

    // Initialize cube structure
    Cube cube_new(input_images, w, args.dimension, args.maxCanditates, args.probes, args.numNn, numBuckets);

    auto tTotalApproximate = std::chrono::nanoseconds(0);
    auto tTotalTrue = std::chrono::nanoseconds(0);
    double sum_all_AAF = 0;
    int sum_all_neighbors = 0;

    ImageDistance *distHelper = ImageDistance::getInstance();

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
            std::vector<Neighbor> approx_new = cube_new.Approximate_kNN(query);
            auto elapsed_cube = stopClock();
            tTotalApproximate += elapsed_cube;

            ImagePtr initSpaceQuery = initQueryImages[query->id];

            startClock();
            std::vector<Neighbor> brute_vector = BruteForce(initImages, initSpaceQuery, args.numNn);
            auto elapsed_brute = stopClock();
            tTotalTrue += elapsed_brute;

            output_file
                << "Query: " << query->id << std::endl;

            int limit = approx_new.size();
            for (int i = 0; i < limit; i++)
            {
                Neighbor neighbor = approx_new[i];

                int neighborId = neighbor.image->id;

                ImagePtr initSpaceNeighbor = initImages[neighborId];

                double dist_new = distHelper->calculate(initSpaceNeighbor, initSpaceQuery);
                double dist_true = brute_vector[i].distance;

                output_file << "Nearest neighbor-" << i + 1 << ": " << neighborId << std::endl
                            << "distanceLSH: " << dist_new << "\n";

                output_file << "distanceTrue: " << dist_true << "\n";

                double localAAF = dist_new / dist_true;
                sum_all_AAF += localAAF;
            }

            sum_all_neighbors += approx_new.size();

            output_file << "tHypercube: " << elapsed_cube.count() * 1e-9 << std::endl;
            output_file << "tTrue: " << elapsed_brute.count() * 1e-9 << std::endl;

            output_file << std::endl;
        }

        output_file << "tAverageApproximate: " << tTotalApproximate.count() * 1e-9 / 100 << std::endl; // Average Approximate time
        output_file << "tAverageTrue: " << tTotalTrue.count() * 1e-9 / 100 << std::endl;               // Average True time
        output_file << "AAF: " << sum_all_AAF / sum_all_neighbors << std::endl;                        // Average Approximation Factor

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
#include <iostream>
#include <vector>
#include <fstream>

#include "Image.hpp"
#include "Utils.hpp"
#include "FileParser.hpp"
#include "BruteForce.hpp"
#include "ImageDistance.hpp"
#include "BruteCmdArgs.hpp"

int main(int argc, char const *argv[])
{
    // Analyze arguments from command line and store them in a simple object
    BruteCmdArgs args(argc, argv);

    bool original = false;
    for (int i = 0; i < argc; i++)
        if (!strcmp(argv[i], "-init"))
            original = true;

    readFilenameIfEmpty(args.inputFile, "input");
    readFilenameIfEmpty(args.queryFile, "query");

    // Parse files and get images
    FileParser inputParser(args.inputFile, args.in_size);
    const std::vector<ImagePtr> latentSpaceImages = inputParser.GetImages();

    FileParser iniDatasetParser = FileParser(args.initDataset, args.in_size);
    const std::vector<ImagePtr> initSpaceImages = iniDatasetParser.GetImages();

    FileParser initQuerysetParser = FileParser(args.initQueryset, args.q_size);
    const std::vector<ImagePtr> initSpaceQueries = initQuerysetParser.GetImages();

    // Configure the metric used for the whole program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);

    ImageDistance *distService = ImageDistance::getInstance();
    auto tTotalApproximate = std::chrono::nanoseconds(0);
    auto tTotalTrue = std::chrono::nanoseconds(0);
    double sum_all_AAF = 0;
    int sum_all_neighbors = 0;
    double MAF = -1;

    // Get query images
    FileParser queryParser(args.queryFile, args.q_size);
    std::vector<ImagePtr> latentSpaceQueries = queryParser.GetImages();

    // For each query data point calculate its approximate k nearesest neighbors
    for (int q = 0; q < args.q_size; q++)
    {
        ImagePtr latentQuery = latentSpaceQueries[q];

        // latent space search (approximate result)
        startClock();
        std::vector<Neighbor> approxNeighbors = BruteForce(latentSpaceImages, latentQuery, args.numNn);
        auto elapsed_lsh = stopClock();
        tTotalApproximate += elapsed_lsh;

        // init space search (exact result)
        ImagePtr projectedQuery = initSpaceQueries[latentQuery->id]; // projected to init space
        startClock();
        std::vector<Neighbor> realNeighbors = BruteForce(initSpaceImages, projectedQuery, args.numNn);
        auto elapsed_brute = stopClock();
        tTotalTrue += elapsed_brute;

        for (int i = 0; i < (int)approxNeighbors.size(); i++)
        {
            Neighbor neighbor = approxNeighbors[i];
            int neighborId = neighbor.image->id;
            ImagePtr projectedNeighbor = initSpaceImages[neighborId]; // project to init space

            double dist_new = distService->calculate(projectedNeighbor, projectedQuery);
            double dist_true = realNeighbors[i].distance;

            double localAAF = dist_new / dist_true;
            sum_all_AAF += localAAF;
            if (localAAF > MAF || MAF == -1)
                MAF = localAAF;
        }

        sum_all_neighbors += approxNeighbors.size();
    }

    std::cout << "tAverageApproximate: " << tTotalApproximate.count() * 1e-9 / args.q_size << std::endl; // Average Approximate time
    std::cout << "tAverageTrue: " << tTotalTrue.count() * 1e-9 / args.q_size << std::endl;               // Average True time
    std::cout << "AAF: " << sum_all_AAF / sum_all_neighbors << std::endl;                                // Average Approximation Factor
    std::cout << "MAF: " << MAF << std::endl;                                                            // Maximum Approximation Factor

    return EXIT_SUCCESS;
}
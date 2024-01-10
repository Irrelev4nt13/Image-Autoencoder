#include <iostream>
#include <vector>
#include <fstream>

#include "Image.hpp"
#include "Utils.hpp"
#include "Lsh.hpp"
#include "LshCmdArgs.hpp"
#include "FileParser.hpp"
#include "BruteForce.hpp"
#include "ImageDistance.hpp"

int main(int argc, char const *argv[])
{
    // Analyze arguments from command line and store them in a simple object
    LshCmdArgs args(argc, argv);

    readFilenameIfEmpty(args.inputFile, "input");

    // Parse file and get the images
    FileParser inputParser(args.inputFile, args.in_size);
    const std::vector<ImagePtr> latentSpaceImages = inputParser.GetImages();

    readFilenameIfEmpty(args.queryFile, "query");

    readFilenameIfEmpty(args.outputFile, "output");
    std::ofstream output_file;

    FileParser iniDatasetParser = FileParser(args.initDataset, args.in_size);
    std::vector<ImagePtr> initSpaceImages = iniDatasetParser.GetImages();

    FileParser initQuerysetParser = FileParser(args.initQueryset, args.q_size);
    std::vector<ImagePtr> initQueries = initQuerysetParser.GetImages();

    // Configure the metric used for the lsh program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);

    int approxAvgDist = 50; // approximate average pixel distance in latent space

    // window
    int w = sqrt(approxAvgDist * approxAvgDist * inputParser.GetMetadata().numOfRows * inputParser.GetMetadata().numOfColumns);
    // buckets
    int numBuckets = inputParser.GetMetadata().numOfImages / 8; // n / 8

    // Initialize hash tables
    Lsh lsh_latent(latentSpaceImages, args.numHashFuncs, args.numHtables, args.numNn, w, numBuckets);

    ImageDistance *distHelper = ImageDistance::getInstance();
    auto tTotalApproximate = std::chrono::nanoseconds(0);
    auto tTotalTrue = std::chrono::nanoseconds(0);
    double sum_all_AAF = 0;
    double MAF = -1;
    int sum_all_neighbors = 0;

    // Keep reading new query and output files until the user types "exit"
    while (true)
    {
        // Get query images
        FileParser queryParser(args.queryFile, args.q_size);
        std::vector<ImagePtr> query_images = queryParser.GetImages();

        output_file.open(args.outputFile);

        // For each query data point calculate its approximate k nearesest neighbors with lsh algorithm and compare it to brute force
        // Also, make a range search with lsh
        for (int q = 0; q < args.q_size; q++)
        {
            ImagePtr query = query_images[q];

            // latent space search with lsh (approximate neighbors)
            startClock();
            std::vector<Neighbor> approxNeighbors = lsh_latent.Approximate_kNN(query);
            auto elapsed_lsh = stopClock();
            tTotalApproximate += elapsed_lsh;

            ImagePtr projectedQuery = initQueries[query->id];

            // init space search with brute force (exact neighbors)
            startClock();
            std::vector<Neighbor> exactNeighbors = BruteForce(initSpaceImages, projectedQuery, args.numNn);
            auto elapsed_brute = stopClock();
            tTotalTrue += elapsed_brute;

            output_file << "Query: " << query->id << std::endl;

            for (int i = 0; i < (int)approxNeighbors.size(); i++)
            {
                Neighbor neighbor = approxNeighbors[i];

                int neighborId = neighbor.image->id;

                ImagePtr projectedNeighbor = initSpaceImages[neighborId];

                double dist_approx = distHelper->calculate(projectedNeighbor, projectedQuery);
                double dist_true = exactNeighbors[i].distance;

                output_file << "Nearest neighbor-" << i + 1 << ": " << neighborId << std::endl
                            << "distanceLSH: " << dist_approx << "\n";

                output_file << "distanceTrue: " << dist_true << "\n";

                double localAAF = dist_approx / dist_true;
                sum_all_AAF += localAAF;

                if (localAAF > MAF || MAF == -1)
                    MAF = localAAF;
            }

            sum_all_neighbors += approxNeighbors.size();

            output_file
                << "tLSH: " << elapsed_lsh.count() * 1e-9 << std::endl;
            output_file << "tTrue: " << elapsed_brute.count() * 1e-9 << std::endl
                        << std::endl;
        }

        output_file << "tAverageApproximate: " << tTotalApproximate.count() * 1e-9 / args.q_size << std::endl; // Average Approximate time
        output_file << "tAverageTrue: " << tTotalTrue.count() * 1e-9 / args.q_size << std::endl;               // Average True time
        output_file << "AAF: " << sum_all_AAF / sum_all_neighbors << std::endl;                                // Average Approximation Factor
        output_file << "MAF: " << MAF << std::endl;                                                            // Maximum Approximation Factor

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
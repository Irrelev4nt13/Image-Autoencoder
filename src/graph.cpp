#include <iostream>
#include <cstring>
#include <vector>
#include <fstream>
#include <chrono>

#include "Image.hpp"
#include "Utils.hpp"
#include "Lsh.hpp"
#include "GraphsCmdArgs.hpp"
#include "FileParser.hpp"
#include "BruteForce.hpp"
#include "ImageDistance.hpp"
#include "GraphAlgorithm.hpp"
#include "Gnns.hpp"
#include "Mrng.hpp"

int main(int argc, char const *argv[])
{
    // Analyze arguments from command line and store them in a simple object
    GraphsCmdArgs args(argc, argv);

    readFilenameIfEmpty(args.inputFile, "input");

    // Parse file and get the images
    FileParser inputParser(args.inputFile, args.in_size);
    const std::vector<ImagePtr> input_images = inputParser.GetImages();

    readFilenameIfEmpty(args.queryFile, "query");

    readFilenameIfEmpty(args.outputFile, "output");
    std::ofstream output_file;

    // Configure the metric used for the lsh program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);
    ImageDistance *distHelper = ImageDistance::getInstance();

    // init file parsers
    FileParser iniDatasetParser = FileParser(args.initDataset, args.in_size);
    std::vector<ImagePtr> initImages = iniDatasetParser.GetImages();

    FileParser initQuerysetParser = FileParser(args.initQueryset, args.q_size);
    std::vector<ImagePtr> initQueryImages = initQuerysetParser.GetImages();

    // Initialize Graphs
    GraphAlgorithm *algorithm_new = nullptr;
    std::string graph_algorithm_name = "";

    if (args.numNn < 1)
    {
        std::cerr << "Error, the number of nearest neighbors has to be positive" << std::endl;
        return EXIT_FAILURE;
    }
    if (args.m == 1)
    {
        // GNNS initialization
        graph_algorithm_name = "GNNS";
        algorithm_new = new GNNS(input_images, args.graphNN, args.expansions, args.restarts, args.numNn);
    }
    else if (args.m == 2)
    {
        // MRNG initialization
        if (args.l < args.numNn)
        {
            std::cerr << "Error, the number of candidates must be greater or equal to the number of nearest neighbors" << std::endl;
            return EXIT_FAILURE;
        }
        graph_algorithm_name = "MRNG";
        algorithm_new = new Mrng(input_images, args.numNn, args.l);
    }
    else
    {
        std::cerr << "Error, unknown type of graph" << std::endl;
        return EXIT_FAILURE;
    }

    auto tTotalApproximate = std::chrono::nanoseconds(0);
    auto tTotalTrue = std::chrono::nanoseconds(0);
    double sum_all_AAF = 0;
    int sum_all_neighbors = 0;
    double MAF = -1;

    // Keep reading new query and output files until the user types "exit"
    while (true)
    {
        // Get query images
        FileParser queryParser(args.queryFile, args.q_size);
        std::vector<ImagePtr> query_images = queryParser.GetImages();

        output_file.open(args.outputFile);

        output_file << graph_algorithm_name << " Results" << std::endl;

        // For each query data point calculate its approximate k nearesest neighbors with the preferable graph algorithm and compare it to brute force
        for (int q = 0; q < args.q_size; q++)
        {
            ImagePtr query = query_images[q];

            ImagePtr initSpaceQuery = initQueryImages[query->id];

            startClock();
            std::vector<Neighbor> approx_new = algorithm_new->Approximate_kNN(query);
            auto elapsed_graph = stopClock();
            tTotalApproximate += elapsed_graph;

            startClock();
            std::vector<Neighbor> brute_vector = BruteForce(initImages, initSpaceQuery, args.numNn);
            auto elapsed_brute = stopClock();
            tTotalTrue += elapsed_brute;

            output_file << "Query: " << query->id << std::endl;

            int limit = approx_new.size();
            for (int i = 0; i < limit; i++)
            {
                Neighbor neighbor = approx_new[i];

                int neighborId = neighbor.image->id;

                ImagePtr initSpaceNeighbor = initImages[neighborId];

                double dist_new = distHelper->calculate(initSpaceNeighbor, initSpaceQuery);
                double dist_true = brute_vector[i].distance;

                output_file << "Nearest neighbor-" << i + 1 << ": " << neighborId << std::endl
                            << "distance" << graph_algorithm_name << "Approximate: " << dist_new << "\n";

                double trueDist = brute_vector[i].distance;
                output_file << "distanceTrue: " << trueDist << "\n";

                double localAAF = dist_new / dist_true;
                sum_all_AAF += localAAF;
                if (localAAF > MAF || MAF == -1)
                    MAF = localAAF;
            }
            sum_all_neighbors += approx_new.size();

            output_file << "t" << graph_algorithm_name << ": " << elapsed_graph.count() * 1e-9 << std::endl;
            output_file << "tTrue: " << elapsed_brute.count() * 1e-9 << std::endl;

            output_file << std::endl;
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

    delete algorithm_new;

    return EXIT_SUCCESS;
}
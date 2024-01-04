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

    // Initialize Graphs
    GraphAlgorithm *algorithm = nullptr;
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
        algorithm = new GNNS(input_images, args.graphNN, args.expansions, args.restarts, args.numNn);
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
        algorithm = new Mrng(input_images, args.numNn, args.l);
    }
    else
    {
        std::cerr << "Error, unknown type of graph" << std::endl;
        return EXIT_FAILURE;
    }

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

        output_file << graph_algorithm_name << " Results" << std::endl;

        // For each query data point calculate its approximate k nearesest neighbors with the preferable graph algorithm and compare it to brute force
        for (int q = 0; q < args.q_size; q++)
        {
            ImagePtr query = query_images[q];

            startClock();
            std::vector<Neighbor> approx_vector = algorithm->Approximate_kNN(query);
            auto elapsed_graph = stopClock();
            tTotalApproximate += elapsed_graph;

            startClock();
            std::vector<Neighbor> brute_vector = BruteForce(input_images, query, args.numNn);
            auto elapsed_brute = stopClock();
            tTotalTrue += elapsed_brute;

            output_file << "Query: " << query->id << std::endl;

            int limit = approx_vector.size();
            for (int i = 0; i < limit; i++)
            {
                ImagePtr image = approx_vector[i].image;
                double aproxDist = approx_vector[i].distance;

                output_file << "Nearest neighbor-" << i + 1 << ": " << image->id << std::endl
                            << "distance" << graph_algorithm_name << "Approximate: " << aproxDist << "\n";

                double trueDist = brute_vector[i].distance;
                output_file << "distanceTrue: " << trueDist << "\n";

                AAF += aproxDist / trueDist;
            }
            found += limit;
            output_file << "t" << graph_algorithm_name << ": " << elapsed_graph.count() * 1e-9 << std::endl;
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

    delete algorithm;

    return EXIT_SUCCESS;
}
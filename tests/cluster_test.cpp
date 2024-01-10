#include <iostream>
#include <fstream>
#include <iomanip>

#include "PublicTypes.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include "FileParser.hpp"
#include "ClusterCmdArgs.hpp"
#include "Cluster.hpp"
#include "ClusterAlgorithms.hpp"
#include "Lsh.hpp"
#include "Cube.hpp"

int main(int argc, char const *argv[])
{
    // Analyze arguments from command line and store them in a simple object
    ClusterCmdArgs args(argc, argv);

    readFilenameIfEmpty(args.inputFile, "input");

    // Parse file and get the images
    FileParser inputParser(args.inputFile, args.in_size);
    const std::vector<ImagePtr> input_images = inputParser.GetImages();

    // Configure the metric used for the entire program
    ImageDistance::setMetric(DistanceMetric::EUCLIDEAN);

    FileParser iniDatasetParser = FileParser(args.initDataset, args.in_size);
    std::vector<ImagePtr> initImages = iniDatasetParser.GetImages();

    // Initialize timer
    std::chrono::nanoseconds elapsed_cluster;

    // Object with all clustering algorithms
    ClusterAlgorithms *alg = new ClusterAlgorithms();

    std::vector<Cluster> clusters;
    startClock();
    clusters = alg->LloydsAssignment(input_images, args.number_of_clusters);
    elapsed_cluster = stopClock();

    bool original = false;
    for (int i = 0; i < argc; i++)
        if (!strcmp(argv[i], "-init"))
            original = true;

    double objective_function = 0.0;
    for (auto &cluster : clusters)
    {
        if (original)
            cluster.ConvertToInitSpace(input_images, initImages);
        objective_function += cluster.SumSquaredError();
    }
    std::tuple<std::vector<double>, double> silhouettes = alg->Silhouettes(clusters);

    std::cout << "\nClustering Time: " << elapsed_cluster.count() * 1e-9 << std::endl;

    double sum_silhouette = 0;

    for (auto silhouette : std::get<0>(silhouettes))
    {
        sum_silhouette += silhouette;
    }

    std::cout << "Average Silhouette: " << (double)sum_silhouette / (double)args.number_of_clusters << std::endl;

    std::cout << "Objective Function: " << objective_function << std::endl;

    delete alg;

    return EXIT_SUCCESS;
}
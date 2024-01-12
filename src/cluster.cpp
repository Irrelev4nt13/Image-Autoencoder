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
    readFilenameIfEmpty(args.outputFile, "output");

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

    // Execute algorithm based on the method
    if (args.method == "Classic")
    {
        startClock();
        clusters = alg->LloydsAssignment(input_images, args.number_of_clusters);
        elapsed_cluster = stopClock();
    }
    else if (args.method == "LSH")
    {
        int w = 10;
        int numBuckets = inputParser.GetMetadata().numOfImages / 8;
        Lsh lsh(input_images, args.number_of_vector_hash_functions, args.number_of_vector_hash_tables, -1, w, numBuckets);

        startClock();
        clusters = alg->ReverseRangeSearchLSH(input_images, lsh, args.number_of_clusters);
        elapsed_cluster = stopClock();
    }
    else if (args.method == "Hypercube")
    {
        int w = 10;
        int numBuckets = std::pow(2, args.number_of_hypercube_dimensions);
        Cube cube(input_images, w, args.number_of_hypercube_dimensions, args.max_number_M_hypercube, args.number_of_probes, -1, numBuckets);

        startClock();
        clusters = alg->ReverseRangeSearchHyperCube(input_images, cube, args.number_of_clusters);
        elapsed_cluster = stopClock();
    }
    else
    {
        std::cout << "Error, unknown method" << std::endl;
        exit(EXIT_FAILURE);
    }

    // convert clusters to initial space and get evaluate the objective function
    double objective_function = 0.0;
    for (auto &cluster : clusters)
    {
        cluster.ConvertToInitSpace(input_images, initImages);
        objective_function += cluster.SumSquaredError();
    }

    std::tuple<std::vector<double>, double> silhouettes = alg->Silhouettes(clusters);

    std::ofstream output_file;
    output_file.open(args.outputFile);

    // Only for formatting purposes
    int cluster_id_formatter = (int)std::to_string(args.number_of_clusters).length();
    int size_formatter = (int)std::to_string(input_images.size()).length();

    // We iterate over all clusters to print the necessary informations
    for (auto cluster : clusters)
    {
        output_file << "CLUSTER-" << std::setw(cluster_id_formatter)
                    << cluster.GetClusterId() + 1 << " {size: "
                    << std::setw(size_formatter)
                    << cluster.GetMemberOfCluster().size() << ", centroid:";
        for (auto pixel : cluster.GetCentroid()->pixels)
            output_file << " " << pixel;
        output_file << "}" << std::endl;
    }

    output_file << "\nclustering_time: " << elapsed_cluster.count() * 1e-9 << std::endl;
    output_file << "\nSilhouette: [";
    double sum_silhouette = 0;

    for (auto silhouette : std::get<0>(silhouettes))
    {
        output_file << silhouette << ", ";
        sum_silhouette += silhouette;
    }

    output_file << std::get<1>(silhouettes) << "]" << std::endl
                << std::endl;

    output_file << "Objective Function evaluation in initial space: " << objective_function << std::endl;

    if (args.complete)
        for (auto cluster : clusters)
        {
            output_file << "CLUSTER-" << std::setw(cluster_id_formatter)
                        << cluster.GetClusterId() + 1 << " {centroid:";
            for (auto pixel : cluster.GetCentroid()->pixels)
                output_file << " " << pixel;
            for (auto image : cluster.GetMemberOfCluster())
                output_file << ", " << image->id;
            output_file << "}" << std::endl;
        }

    output_file.close();

    delete alg;

    return EXIT_SUCCESS;
}
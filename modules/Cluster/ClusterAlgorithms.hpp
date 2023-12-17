#ifndef CLUSTERALGORITHMS_HPP_
#define CLUSTERALGORITHMS_HPP_

#include <vector>
#include <unordered_set>

#include "PublicTypes.hpp"
#include "Image.hpp"
#include "Cluster.hpp"
#include "Lsh.hpp"
#include "Cube.hpp"
/**
 * @brief The class of the clustering algorithms consists of the following
 *
 * @param distanceHelper the generic distance
 *
 * @method MinDistanceToCentroids calculates and returns the minDistance, the cluster with minDistance and the next closest cluster for the given image
 * @method MinDistanceCentroids calculates and returns the minDistance between all centroids
 * @method KMeansPlusPlus returns a vector with Cluster objects with initialized centroids
 * @method MacQueen the MacQueen algorithm to update the centroids
 * @method LloydsAssignment the implementation of Lloyds algorithm
 * @method ReverseAssignment a helper function to assign the objects from RangeSearch
 * @method LloydsForRest a helper function to assign the images that were not assigned during reverse assignment
 * @method ReverseRangeSearchLSH the clustering algorithm for LSH
 * @method ReverseRangeSearchHyperCube the clustering algorithm for Hypercube
 * @method Silhouettes returns a tuple with first element a vector of double values with the silhouette of each cluster and the second element is the stotal
 */
class ClusterAlgorithms
{
private:
    ImageDistance *distanceHelper;

public:
    ClusterAlgorithms();
    ~ClusterAlgorithms();

    std::tuple<double, int, int> MinDistanceToCentroids(const ImagePtr image, std::vector<Cluster> clusters);
    double MinDistanceCentroids(std::vector<Cluster> clusters);
    std::vector<Cluster> KMeansPlusPlus(std::vector<ImagePtr> input_images, int number_of_clusters);
    void MacQueen(std::vector<Cluster> &clusters, int prev_clust, int new_clust, ImagePtr image);
    std::vector<Cluster> LloydsAssignment(std::vector<ImagePtr> input_images, int number_of_clusters);
    void ReverseAssignment(std::vector<Cluster> &clusters,
                           std::vector<ImagePtr> RangeSearch,
                           std::unordered_map<int, int> &assigned_images,
                           int &assignment_occurred,
                           int &assignment_occurred_inner, int candidate_cluster);
    void LloydsForRest(std::vector<ImagePtr> input_images,
                       std::vector<Cluster> &clusters,
                       std::unordered_map<int, int> &assigned_images,
                       int &assignment_occurred);
    std::vector<Cluster> ReverseRangeSearchLSH(std::vector<ImagePtr> input_images, Lsh lsh, int number_of_clusters);
    std::vector<Cluster> ReverseRangeSearchHyperCube(std::vector<ImagePtr> input_images, Cube &cube, int number_of_clusters);
    std::tuple<std::vector<double>, double> Silhouettes(std::vector<Cluster> clusters);
};

#endif
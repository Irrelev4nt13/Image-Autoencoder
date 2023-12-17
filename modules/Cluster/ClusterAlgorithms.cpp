#include <vector>
#include <unordered_set>
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <algorithm>

#include "PublicTypes.hpp"
#include "Utils.hpp"
#include "Image.hpp"
#include "Cluster.hpp"
#include "ClusterAlgorithms.hpp"
#include "Lsh.hpp"
#include "Cube.hpp"
#include "ImageDistance.hpp"

// Initializes the generic distance
ClusterAlgorithms::ClusterAlgorithms() { distanceHelper = ImageDistance::getInstance(); }

ClusterAlgorithms::~ClusterAlgorithms() {}

// Calculates and returns the distance from the closest cluster, the id of the closest cluster and the next closest cluster id
std::tuple<double, int, int> ClusterAlgorithms::MinDistanceToCentroids(const ImagePtr image, std::vector<Cluster> clusters)
{

    double minDistance = -1;
    double secondMinDistance = -1;
    int cluster_id = -1;
    int next_cluster_id = -1;
    bool minDistanceInitialized = false;

    // We iterate over all clusters
    for (std::size_t i = 0; i < clusters.size(); i++)
    {
        // Calculate the distance
        double distance = distanceHelper->calculate(clusters[i].GetCentroid(), image);

        // If the minDistance is unitialized we initialize it
        if (!minDistanceInitialized)
        {
            minDistance = distance;
            cluster_id = i;
            minDistanceInitialized = true;
        }
        // If the new distance is smaller update the minDistance
        else if (distance < minDistance)
        {
            secondMinDistance = minDistance;
            next_cluster_id = cluster_id;
            minDistance = distance;
            cluster_id = i;
        }
        // If the new distance is not smaller from the minDistance check if it is from the secondMinDistance in order to save the next closest cluster too
        else if (distance < secondMinDistance || secondMinDistance == -1)
        {
            secondMinDistance = distance;
            next_cluster_id = i;
        }
    }
    return std::tuple<double, int, int>{minDistance, cluster_id, next_cluster_id};
}

// Returns the minimum distance between centroids
double ClusterAlgorithms::MinDistanceCentroids(std::vector<Cluster> clusters)
{
    double minDistance = -1;
    bool minDistanceInitialized = false;
    // For every cluster we need to compute the distance of its centroid with the other centroids
    // But we need to make sure we don't take into calculation the same cluster.
    for (auto &cluster1 : clusters)
    {
        for (auto &cluster2 : clusters)
        {
            // Avoid comparing a cluster with itself
            if (&cluster1 != &cluster2)
            {
                // Calculate the distance
                double dist = distanceHelper->calculate(cluster1.GetCentroid(), cluster2.GetCentroid());
                if (!minDistanceInitialized)
                {
                    minDistance = dist;
                    minDistanceInitialized = true;
                }
                else if (dist < minDistance)
                    minDistance = dist;
            }
        }
    }

    return minDistance;
}

std::vector<Cluster> ClusterAlgorithms::KMeansPlusPlus(std::vector<ImagePtr> input_images, int number_of_clusters)
{
    std::vector<Cluster> clusters;     // all clusters
    std::unordered_set<int> centroids; // set to remember which centroids have already been chosen

    // choose a random existing data point as an initial centroid in the first cluster
    int first_index = IntDistribution(0, input_images.size() - 1);
    clusters.push_back(Cluster(input_images[first_index], 0));

    // update set
    centroids.insert(input_images[first_index]->id);

    // For each cluster after the first one
    // Calculate the min distance from the images to centroids
    for (int i = 1; i < number_of_clusters; i++)
    {
        std::vector<double> minDistances;

        double maxDistance = 0.0;

        for (std::size_t j = 0; j < input_images.size(); j++)
        {
            // If current image id of dataset matches an id from previously selected centroids, continue
            if (centroids.find(input_images[j]->id) != centroids.end())
            {
                continue;
            }

            // calculate the min distances
            std::tuple<double, int, int> distance_and_id = MinDistanceToCentroids(input_images[j], clusters);

            double distance = std::get<0>(distance_and_id);
            minDistances.push_back(distance);

            if (distance > maxDistance)
                maxDistance = distance;
        }

        // size: n - t where t increases by 1 after each cluster has been initialized
        int partialSum_size = input_images.size() - centroids.size();

        // partial sums
        std::vector<double> partialSum(partialSum_size);

        // calculate partial sums
        double sum = 0;
        for (int r = 0; r < partialSum_size; r++)
        {
            minDistances[r] /= maxDistance;
            sum += pow(minDistances[r], 2);
            partialSum[r] = sum;
        }

        // random x with uniform real distriubution in the range of the probabilities values
        double x = RealDistribution(0, partialSum[partialSum_size - 1]);

        // apply binary search for x in partialSum vector and find the corresponding index
        int idx = binarySearch(partialSum, x);

        // update set and clusters
        centroids.insert(idx);
        clusters.push_back(Cluster(input_images[idx], i));
    }
    return clusters;
}

// The MacQueen updates the centroids on two clusters.
void ClusterAlgorithms::MacQueen(std::vector<Cluster> &clusters, int prev_clust, int new_clust, ImagePtr image)
{
    // If the image was previously in a different cluster it removes the image from that cluster and also update its centroid
    if (prev_clust != -1)
    {
        clusters[prev_clust].RemoveMember(image);
        clusters[prev_clust].UpdateCentroid(image);
    }
    // Also, it will update the centroid of the new cluster
    clusters[new_clust].UpdateCentroid(image);
}

std::vector<Cluster> ClusterAlgorithms::LloydsAssignment(std::vector<ImagePtr> input_images, int number_of_clusters)
{
    // Get the initializations of centroids bases on KMeans++
    std::vector<Cluster> clusters = KMeansPlusPlus(input_images, number_of_clusters);

    // A map to match each image in the cluster it is assigned to
    std::unordered_map<int, int> assigned_images;

    // Until there are no new assignments
    while (true)
    {
        int assignment_occurred = 0;
        // Iterate over every image from input
        for (auto image : input_images)
        {
            // Calculate its minDistance from centroids and the cluster it belongs to
            std::tuple<double, int, int> distance_and_id = MinDistanceToCentroids(image, clusters);

            // If this is the first assignment just add it to the cluster and call MacQueen with prev_clust = -1 to let it know that its the first assignment
            if (assigned_images.find(image->id) == assigned_images.end())
            {
                assignment_occurred++;
                assigned_images[image->id] = std::get<1>(distance_and_id);
                clusters[std::get<1>(distance_and_id)].AddToCluster(image);
                MacQueen(clusters, -1, std::get<1>(distance_and_id), image);
            }
            else // If the image have been assigned at least once
            {
                // Re-assign only if the new cluster is different
                // Only this time call MacQueen with a real prev_clust to make the necessary updates
                if (assigned_images[image->id] != std::get<1>(distance_and_id))
                {
                    assignment_occurred++;
                    int prev_clust = assigned_images[image->id];
                    assigned_images[image->id] = std::get<1>(distance_and_id);
                    clusters[std::get<1>(distance_and_id)].AddToCluster(image);
                    MacQueen(clusters, prev_clust, std::get<1>(distance_and_id), image);
                }
            }
        }
        // That's the condition to break the while loop as we said before
        if (assignment_occurred == 0)
            break;
    }
    return clusters;
}

// That's the helper method for ReverseRangeSearch for LSH and Hypercube
// It's purpose is to assign each object from the range search result
void ClusterAlgorithms::ReverseAssignment(std::vector<Cluster> &clusters,
                                          std::vector<ImagePtr> RangeSearch,
                                          std::unordered_map<int, int> &assigned_images,
                                          int &assignment_occurred,
                                          int &assignment_occurred_inner, int candidate_cluster)
{
    // Iterate over every image from Range Search
    for (auto image : RangeSearch)
    {
        // If this is the first assignment just add it to the cluster and call MacQueen with prev_clust = -1 to let it know that its the first assignment
        if (assigned_images.find(image->id) == assigned_images.end())
        {
            assignment_occurred++;
            assignment_occurred_inner++;
            assigned_images[image->id] = candidate_cluster;
            clusters[candidate_cluster].AddToCluster(image);
            MacQueen(clusters, -1, candidate_cluster, image);
        }
        else
        {
            // If it's not the first assignment calculate the distance between the new and the previous cluster
            double candidate_cluster_distance = distanceHelper->calculate(clusters[candidate_cluster].GetCentroid(), image);
            double cur_cluster_distance = distanceHelper->calculate(clusters[assigned_images[image->id]].GetCentroid(), image);

            // Make a new assignment only if the new cluster candidate is closer than the previous one
            if (candidate_cluster_distance < cur_cluster_distance)
            {
                assignment_occurred++;
                assignment_occurred_inner++;
                int prev_clust = assigned_images[image->id];
                assigned_images[image->id] = candidate_cluster;
                clusters[candidate_cluster].AddToCluster(image);
                MacQueen(clusters, prev_clust, candidate_cluster, image);
            }
        }
    }
}

// That's the helper method for ReverseRangeSearch for LSH and Hypercube
// It's purpose is to assign all the leftover images after the ReverseAssignment using Lloyd's Algorithm
void ClusterAlgorithms::LloydsForRest(std::vector<ImagePtr> input_images,
                                      std::vector<Cluster> &clusters,
                                      std::unordered_map<int, int> &assigned_images,
                                      int &assignment_occurred)
{
    // Iterate over every image from input
    for (auto image : input_images)
    {
        // If the image is not assigned then add it to the closest cluster and call MacQueen with prev_clust = -1 to let it know that its the first assignment
        if (assigned_images.find(image->id) == assigned_images.end())
        {
            assignment_occurred++;
            std::tuple<double, int, int> distance_id_nextid = MinDistanceToCentroids(image, clusters);
            clusters[std::get<1>(distance_id_nextid)].AddToCluster(image);
            assigned_images[image->id] = std::get<1>(distance_id_nextid);
            MacQueen(clusters, -1, std::get<1>(distance_id_nextid), image);
        }
    }
}

// The method which does all the essential handling of submethods for the implementation of Reverse Range Search with LSH
std::vector<Cluster> ClusterAlgorithms::ReverseRangeSearchLSH(std::vector<ImagePtr> input_images, Lsh lsh, int number_of_clusters)
{
    // Get the initializations of centroids bases on KMeans++
    std::vector<Cluster> clusters = KMeansPlusPlus(input_images, number_of_clusters);

    // A map to match each image in the cluster it is assigned to
    std::unordered_map<int, int> assigned_images;

    // Until there are no new assignments
    while (true)
    {
        int assignment_occurred = 0;

        // We initialize the radius based on the minDistance between all centroids devided by 2
        double radius = MinDistanceCentroids(clusters) / 2.0;

        // Until there are no new assignments from range search
        while (true)
        {
            int assignment_occurred_inner = 0;

            // For every cluster we make a range search for its centroid
            for (int i = 0; i < number_of_clusters; i++)
            {
                // Do the range search for its centroid with LSH
                std::vector<ImagePtr> RangeSearch = lsh.Approximate_Range_Search(clusters[i].GetCentroid(), radius);

                // Call the reverse assignment helper method to do the assigns and the necessary updates
                ReverseAssignment(clusters, RangeSearch, assigned_images, assignment_occurred, assignment_occurred_inner, i);
            }

            // That's the condition to break the inner while loop as we said before
            if (assignment_occurred_inner == 0)
                break;

            // If there is at least one assignment multiply the radius by 2 and start all over again
            radius *= 2;
        }

        // Call the helper method to assign the rest images based on Lloyd's
        LloydsForRest(input_images, clusters, assigned_images, assignment_occurred);

        // That's the condition to break the while loop as we said before
        if (assignment_occurred == 0)
            break;
    }
    return clusters;
}

std::vector<Cluster> ClusterAlgorithms::ReverseRangeSearchHyperCube(std::vector<ImagePtr> input_images, Cube &cube, int number_of_clusters)
{
    // Get the initializations of centroids bases on KMeans++
    std::vector<Cluster> clusters = KMeansPlusPlus(input_images, number_of_clusters);

    // A map to match each image in the cluster it is assigned to
    std::unordered_map<int, int> assigned_images;

    // Until there are no new assignments
    while (true)
    {
        int assignment_occurred = 0;

        // We initialize the radius based on the minDistance between all centroids devided by 2
        double radius = MinDistanceCentroids(clusters) / 2.0;

        // Until there are no new assignments from range search
        while (true)
        {
            int assignment_occurred_inner = 0;

            // For every cluster we make a range search for its centroid
            for (int i = 0; i < number_of_clusters; i++)
            {
                // Do the range search for its centroid with Hypercube
                std::vector<ImagePtr> RangeSearch = cube.Approximate_Range_Search(clusters[i].GetCentroid(), radius);

                // Call the reverse assignment helper method to do the assigns and the necessary updates
                ReverseAssignment(clusters, RangeSearch, assigned_images, assignment_occurred, assignment_occurred_inner, i);
            }

            // That's the condition to break the inner while loop as we said before
            if (assignment_occurred_inner == 0)
                break;

            // If there is at least one assignment multiply the radius by 2 and start all over again
            radius *= 2;
        }

        // Call the helper method to assign the rest images based on Lloyd's
        LloydsForRest(input_images, clusters, assigned_images, assignment_occurred);

        // That's the condition to break the while loop as we said before
        if (assignment_occurred == 0)
            break;
    }
    return clusters;
}

// Calculates and returns the sihlouetters for all clusters and the stotal
std::tuple<std::vector<double>, double> ClusterAlgorithms::Silhouettes(std::vector<Cluster> clusters)
{
    // We initialize an empty vector which will be used to store the silhouette for each cluster
    std::vector<double> silhouettes;
    int num_of_clusters = clusters.size();
    double stotal = 0.0;
    int total_members = 0;

    // Iterate over every cluster
    for (int i = 0; i < num_of_clusters; i++)
    {

        double si = 0.0;

        // Iterate over every member from the current cluster's member list
        for (auto member : clusters[i].GetMemberOfCluster())
        {
            // We apply the formula from the slides
            double ai = clusters[i].AverageDistance(member);
            double bi = clusters[std::get<2>(MinDistanceToCentroids(member, clusters))].AverageDistance(member);
            si += (bi - ai) / std::max(ai, bi);
        }
        stotal += si;
        total_members += clusters[i].GetMemberOfCluster().size();
        silhouettes.push_back(si / clusters[i].GetMemberOfCluster().size());
    }
    return std::tuple<std::vector<double>, double>{silhouettes, stotal / total_members};
}

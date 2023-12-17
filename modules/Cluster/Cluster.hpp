#ifndef CLUSTER_HPP_
#define CLUSTER_HPP_

#include <vector>

#include "PublicTypes.hpp"
#include "Image.hpp"
#include "ImageDistance.hpp"
/**
 * @brief The class of a cluster consists of the following
 *
 * @param centroid the centroid of the cluster
 * @param member_of_cluster the images that are part of this cluster
 * @param id the id of the cluster (it's number in the creation an so on)
 * @param prev_len the number of the members of the cluster before an addition/removal
 * @param cur_len the number of the members of the after an addition/removal
 * @param distance the generic distance
 *
 * @method GetCentroid it returns the pointer to the centroid
 * @method GetMemberOfCluster it returns the vector with the member of the cluster
 * @method AddToCluster it returns the pointer to the centroid
 * @method GetClusterId it returns the id of the cluster
 * @method RemoveMember it removes the current member from the members vector if it exists in the cluster
 * @method UpdateCentroid update the centroid as the mean of all members
 * @method AverageDistance it returns the average distance of the given image from the members of the cluster
 */
class Cluster
{
private:
    ImagePtr centroid;
    std::vector<ImagePtr> member_of_cluster;
    int id;
    int prev_len;
    int cur_len;
    ImageDistance *distance;

public:
    Cluster(ImagePtr image, const int id);
    ~Cluster();
    ImagePtr GetCentroid();
    std::vector<ImagePtr> GetMemberOfCluster();
    void AddToCluster(ImagePtr image);
    int GetClusterId();
    void RemoveMember(ImagePtr member);
    void UpdateCentroid(ImagePtr image);
    double AverageDistance(ImagePtr image);
};

#endif
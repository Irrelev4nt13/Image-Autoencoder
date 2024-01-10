#include <vector>
#include <algorithm>

#include "PublicTypes.hpp"
#include "Cluster.hpp"
#include "Image.hpp"
#include "Utils.hpp"
#include "ClusterAlgorithms.hpp"
#include "BruteForce.hpp"

// Simple constructor for Cluster object. We make a new pointer for the centoid to avoid conflicts with the real one
Cluster::Cluster(ImagePtr image, const int id) : centroid(new Image(*image)), id(id), prev_len(0), cur_len(0), distance(ImageDistance::getInstance()) {}

Cluster::~Cluster() {}

ImagePtr Cluster::GetCentroid() { return centroid; }

void Cluster::ConvertToInitSpace(const std::vector<ImagePtr> &latentSpaceImages, const std::vector<ImagePtr> &initSpaceImages)
{
    // Convert all members of cluster using the id
    for (auto &image : this->member_of_cluster)
    {
        image = initSpaceImages[image->id];
    }

    // Convert centroid to initial space
    // First find the closest point to the centroid from the latent dimension dataset
    ImagePtr closestPointToCentroid = BruteForce(latentSpaceImages, this->centroid, 1)[0].image;

    // Convert centroid to initial space using the id
    this->centroid = initSpaceImages[closestPointToCentroid->id];
}

std::vector<ImagePtr> Cluster::GetMemberOfCluster() { return member_of_cluster; }

// We add the image to the member vector
void Cluster::AddToCluster(ImagePtr image)
{
    member_of_cluster.push_back(image);
    prev_len = cur_len;
    ++cur_len;
}

int Cluster::GetClusterId() { return id; }

// We remove the image from the member vector if it exists there
void Cluster::RemoveMember(ImagePtr member)
{
    auto it = std::find(member_of_cluster.begin(), member_of_cluster.end(), member);
    if (it != member_of_cluster.end())
    {
        member_of_cluster.erase(it);
        prev_len = cur_len;
        --cur_len;
    }
}

// In this method we utilize the use of the prev_len and cur_len. To avoid calculate the mean from the beginning we can calculate it
// After an insertion or removal based on the formula below:
// https://math.stackexchange.com/questions/106700/incremental-averaging#:~:text=The%20idea%20is%20to%20represent,divided%20by%20the%20total%20count.

void Cluster::UpdateCentroid(ImagePtr image)
{
    if (cur_len > prev_len) // Point was added
        for (int i = 0; i < (int)centroid->pixels.size(); i++)
            centroid->pixels[i] = centroid->pixels[i] * prev_len / cur_len + image->pixels[i] / cur_len;
    else // Point was removed
    {
        // After a removal we need to check that the cur_size is not zero to avoid division with 0
        if (cur_len == 0)
            for (auto &pixel : centroid->pixels)
                pixel = 0;
        else
            for (int i = 0; i < (int)centroid->pixels.size(); i++)
                centroid->pixels[i] = centroid->pixels[i] * prev_len / cur_len - image->pixels[i] / cur_len;
    }
}

// We want to calculate the average distance for the given image for all the members of the cluster
// We don't take into consideration the distance of the image to itself if it exists
// In case the image is member we first check that the size of the cluster is greater than 1 to avoid divide with 0
double Cluster::AverageDistance(ImagePtr image)
{
    bool is_member = false;
    double totalDistance = 0.0;
    for (auto member : this->GetMemberOfCluster())
        if (image->id == member->id)
            is_member = true;
        else
            totalDistance += distance->calculate(member, image);
    if (!is_member)
        return totalDistance / (int)this->GetMemberOfCluster().size();
    if ((int)this->GetMemberOfCluster().size() > 1)
        return totalDistance / ((int)this->GetMemberOfCluster().size() - 1);
    return totalDistance / (int)this->GetMemberOfCluster().size();
}

double Cluster::SumSquaredError()
{
    double sse = 0.0;

    for (auto &member : this->member_of_cluster)
    {
        // Calculate the distance between each member and the centroid
        double dist = this->distance->calculate(this->centroid, member);

        // Square the distance and add it to the total SSE of cluster
        sse += dist * dist;
    }

    return sse;
}

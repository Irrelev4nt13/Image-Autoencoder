#include <iostream>
#include <vector>
#include <algorithm>
#include <set>

#include "Mrng.hpp"
#include "Utils.hpp"
#include "BruteForce.hpp"

class ThreadData
{
public:
    int id;
    std::vector<std::vector<ImagePtr>> &graph;
    const std::vector<ImagePtr> &images;
    int startIdx;
    int endIdx;
    ImageDistance *distHelper;
    std::vector<double> &sum;

    ThreadData(int id, std::vector<std::vector<ImagePtr>> &graph, const std::vector<ImagePtr> &images, int startIdx, int endIdx,
               ImageDistance *distHelper, std::vector<double> &sum)
        : id(id), graph(graph), images(images), startIdx(startIdx), endIdx(endIdx), distHelper(distHelper), sum(sum) {}
};

void *ThreadFunction(void *threadData)
{
    ThreadData *data = static_cast<ThreadData *>(threadData);

    int dim = data->images[0]->pixels.size();
    data->sum.resize(dim);

    for (int i = data->startIdx; i <= data->endIdx; i++)
    {
        std::vector<Neighbor> Rp = BruteForce(data->images, data->images[i], data->images.size());
        Rp.erase(Rp.begin());

        // Compute the sum in all dimensions of image
        for (int d = 0; d < dim; d++)
        {
            data->sum[d] += data->images[i]->pixels[d];
        }

        std::vector<ImagePtr> Lp;
        double minDistance = Rp[0].distance;
        for (int j = 0; j < (int)Rp.size(); j++)
        {
            if (Rp[j].distance > minDistance)
            {
                break; // No more points with the minimum distance to add
            }
            Lp.push_back(Rp[j].image);
        }

        // For each element of Rp check the Mrng condition and add it to Lp
        for (int r = 0; r < (int)Rp.size(); r++)
        {
            if (std::find(Lp.begin(), Lp.end(), Rp[r].image) != Lp.end())
            {
                continue; // Point already in Lp
            }

            // Mrng condition to ensure monotonic path
            bool condition = true;
            for (int t = 0; t < (int)Lp.size(); t++)
            {
                double prDistance = Rp[r].distance; // same as dist(images[i], Rp[r].image)
                double ptDistance = data->distHelper->calculate(data->images[i], Lp[t]);
                double rtDistance = data->distHelper->calculate(Rp[r].image, Lp[t]);

                // Check if pr is the longest edge in the triangle prt
                if (prDistance > rtDistance && prDistance > ptDistance)
                {
                    // pr is the longest edge, so it is not a valid neighbor for Mrng
                    condition = false;
                    break;
                }
            }

            if (condition)
            {
                Lp.push_back(Rp[r].image);
            }
        }

        data->graph[i] = Lp; // Add neighbors of current image to graph
    }

    delete data;
    pthread_exit(nullptr);
}

Mrng::Mrng(const std::vector<ImagePtr> &images, int numNn, int l) : numNn(numNn), candidates(l),
                                                                    distHelper(ImageDistance::getInstance()), navNode(nullptr)
{
    // startClock();

    graph.resize(images.size());

    const int numThreads = 4;
    std::vector<std::vector<double>> sums(numThreads);
    std::vector<pthread_t> threads(numThreads);
    int imagesPerThread = images.size() / numThreads;
    int remainingImages = images.size() % numThreads;

    for (int i = 0; i < numThreads; i++)
    {
        int startIdx = i * imagesPerThread;

        bool addRemaining = startIdx + 2 * imagesPerThread > (int)images.size();

        int endIdx = startIdx + imagesPerThread - 1 + (addRemaining ? remainingImages : 0);

        ThreadData *threadData = new ThreadData(i, graph, images, startIdx, endIdx, distHelper, sums[i]);

        if (pthread_create(&threads[i], NULL, ThreadFunction, threadData))
        {
            std::cerr << "Error creating thread " << i << std::endl;
            return;
        }
    }

    for (int i = 0; i < numThreads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    int dim = images[0]->pixels.size();
    std::vector<double> totalSum;
    totalSum.resize(dim);

    for (int i = 0; i < numThreads; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            totalSum[j] += sums[i][j];
        }
    }

    // calculate mean
    std::vector<double> meanPixels(dim);
    for (int i = 0; i < dim; i++)
    {
        meanPixels[i] = totalSum[i] / (double)images.size();
    }

    Image centroid(0, meanPixels); // use dummy id

    // Find the closest image from dataset to centroid with brute force
    std::vector<Neighbor> closest = BruteForce(images, &centroid, 1);

    navNode = images[closest[0].image->id];

    // auto mrngDuration = stopClock();
    // std::cout << "Mrng index construction finished in: " << mrngDuration.count() * 1e-9 << std::endl;
}
// Mrng::Mrng(const std::vector<ImagePtr> &images, int numNn, int l)
//     : numNn(numNn), candidates(l), distHelper(ImageDistance::getInstance()), navNode(nullptr)
// {
//     startClock();

//     std::vector<double> sum;
//     int dim = images[0]->pixels.size();
//     sum.resize(dim);

//     for (std::size_t i = 0; i < images.size(); i++)
//     {
//         // Get sorted Rp with brute force
//         std::vector<Neighbor> Rp = BruteForce(images, images[i], images.size());

//         Rp.erase(Rp.begin()); // Erase first element that is the same as the query

//         for (int d = 0; d < dim; d++)
//         {
//             sum[d] += images[i]->pixels[d];
//         }

//         // Initialize Lp with points that have the minimum distance to p (images[i])
//         std::vector<ImagePtr> Lp;
//         double minDistance = Rp[0].distance;
//         for (int j = 0; j < (int)Rp.size(); j++)
//         {
//             if (Rp[j].distance > minDistance)
//             {
//                 break; // No more points with the minimum distance to add
//             }
//             Lp.push_back(Rp[j].image);
//         }

//         // For each element of Rp check the Mrng condition and add it to Lp
//         for (int r = 0; r < (int)Rp.size(); r++)
//         {
//             if (std::find(Lp.begin(), Lp.end(), Rp[r].image) != Lp.end())
//             {
//                 continue; // Point already in Lp
//             }

//             // Mrng condition to ensure monotonic path
//             bool condition = true;
//             for (int t = 0; t < (int)Lp.size(); t++)
//             {
//                 double prDistance = Rp[r].distance;                          // same as dist(images[i], Rp[r])
//                 double ptDistance = distHelper->calculate(images[i], Lp[t]); // same as dist(images[i], Lp[t])
//                 double rtDistance = distHelper->calculate(Rp[r].image, Lp[t]);

//                 // prDistance is always greater than ptDistance = minDistance since Rp is sorted
//                 // Need to check between prDistance and rtDistance for triangle prt
//                 if (prDistance > rtDistance && prDistance > ptDistance)
//                 {
//                     // pr is the longest edge, so it is not a valid neighbor for Mrng
//                     condition = false;
//                     break;
//                 }
//             }

//             if (condition)
//             {
//                 Lp.push_back(Rp[r].image);
//             }
//         }

//         for (auto neighbor : Lp)
//             std::cout << neighbor->id << " ";
//         std::cout << std::endl;

//         graph.push_back(Lp); // Add neighbors of current image to graph
//     }

//     std::vector<double> meanPixels(dim);
//     for (int d = 0; d < dim; d++)
//     {
//         meanPixels[d] = sum[d] / (double)images.size();
//     }
//     Image centroid(0, meanPixels);
//     auto result = BruteForce(images, &centroid, 1);

//     navNode = result[0].image;

//     auto mrngDuration = stopClock();
//     std::cout << "Mrng index construction finished in: " << mrngDuration.count() * 1e-9 << std::endl;
// }

Mrng::~Mrng() {}

class NeighborInSet
{
public:
    Neighbor neighbor;
    bool checked;

    NeighborInSet(ImagePtr image, double distance, bool checked) : neighbor(image, distance), checked(checked) {}
};

class CompareNeighborInSet
{
public:
    bool operator()(const NeighborInSet &a, const NeighborInSet &b) const
    {
        // Compare based on the double value (distance) within Neighbor objects.
        return a.neighbor.distance < b.neighbor.distance;
    }
};

std::vector<Neighbor> Mrng::Approximate_kNN(ImagePtr query)
{
    // Initialize R to an empty set
    std::set<NeighborInSet, CompareNeighborInSet> R;

    // Start with the navigating node
    NeighborInSet p = NeighborInSet(navNode, distHelper->calculate(navNode, query), false);
    R.insert(p);

    int i = 1;
    int visitedNodes = 0;

    // Search for the number of candidates
    while (i < candidates && (int)R.size() > visitedNodes)
    {
        // Each time get the first unchecked node of R
        for (auto it = R.begin(); it != R.end(); ++it)
        {
            if (!it->checked)
            {
                // Erase the existing NeighborInSet and insert the updated one
                R.erase(it);

                // Mark the element as checked
                NeighborInSet updatedNeighborInSet = *it;
                updatedNeighborInSet.checked = true;

                R.insert(updatedNeighborInSet);

                p = *it; // update p
                break;
            }
        }

        visitedNodes++;

        // Get neighbors of p based on the graph
        std::vector<ImagePtr> neighborImages = graph[p.neighbor.image->id];
        for (int k = 0; k < (int)neighborImages.size(); k++)
        {
            NeighborInSet element = NeighborInSet(neighborImages[k], distHelper->calculate(neighborImages[k], query), false);
            auto result = R.insert(element);
            if (result.second) // insert succeeded
            {
                i++;
            }
        }
    }
    std::vector<Neighbor> KnearestNeighbors;

    // Iterate R and extract Neighbor objects
    for (const auto &NeighborInSet : R)
    {
        if ((int)KnearestNeighbors.size() >= numNn)
        {
            break;
        }

        KnearestNeighbors.push_back(NeighborInSet.neighbor);
    }

    return KnearestNeighbors;
}
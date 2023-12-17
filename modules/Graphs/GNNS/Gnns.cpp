#include <vector>
#include <set>
#include <pthread.h>

#include "Image.hpp"
#include "Lsh.hpp"
#include "PublicTypes.hpp"
#include "ImageDistance.hpp"
#include "Gnns.hpp"
#include "Utils.hpp"

class threadArgs
{
public:
    int start;
    int end;
    std::vector<std::vector<ImagePtr>> *storage;
    std::vector<ImagePtr> images;
    Lsh *lsh;
    threadArgs(int start, int end, const std::vector<ImagePtr> &images, Lsh *lsh, std::vector<std::vector<ImagePtr>> *storage) : start(start), end(end), storage(storage), images(images), lsh(lsh) {}
};

static void *parallel_initialization(void *arg)
{
    threadArgs *args = (threadArgs *)arg;
    for (int i = args->start; i < args->end; i++)
        for (auto neighbor : args->lsh->Approximate_kNN(args->images[i]))
            (*args->storage)[i].push_back(neighbor.image);

    delete args;
    return nullptr;
}

GNNS::GNNS(const std::vector<ImagePtr> &images, int graphNN, int expansions, int restarts, int numNn)
    : graphNN(graphNN), expansions(expansions), restarts(restarts), numNn(numNn)
{
    // Initialize the general distance
    this->distance = ImageDistance::getInstance();

    // Initialize lsh which will be used to initialize the graph
    Lsh lsh(images, 4, 5, graphNN + 1, 2240, (int)images.size() / 8);

    // startClock();

    // In order to avoid multiple reallocs we will resize the vector
    PointsWithNeighbors.resize((int)images.size());

    // We need to initialize our graph, for every image in the input file we will get its neighbors
    // and store them in a 2d vector the first dimension will represent the Image in the input file
    // and the second its neighbors
    // for (int i = 0; i < (int)images.size(); i++)
    //     for (auto neighbor : lsh.Approximate_kNN(images[i]))
    //         PointsWithNeighbors[i].push_back(neighbor.image);
    const int threadNum = 4;
    std::vector<pthread_t> threads(threadNum);
    for (int i = 0; i < threadNum; i++)
        pthread_create(&threads[i], nullptr, parallel_initialization,
                       new threadArgs(i * ((int)images.size() / threadNum),
                                      (i == threadNum - 1) ? (int)images.size() : (i + 1) * ((int)images.size() / threadNum),
                                      images,
                                      &lsh,
                                      &PointsWithNeighbors));

    for (int i = 0; i < threadNum; i++)
        pthread_join(threads[i], nullptr);

    // auto gnnsDuration = stopClock();
    // std::cout << "GNNS initialized in: " << gnnsDuration.count() * 1e-9 << " seconds" << std::endl;
}

// GNNS::GNNS(const std::vector<ImagePtr> &images, int graphNN, int expansions, int restarts, int numNn)
//     : graphNN(graphNN), expansions(expansions), restarts(restarts), numNn(numNn)
// {
//     // Initialize the general distance
//     this->distance = ImageDistance::getInstance();
//     // Initialize lsh which will be used to initialize the graph
//     Lsh lsh(images, 4, 5, graphNN + 1, 2240, (int)images.size() / 8);
//     // In order to avoid multiple reallocs we will resize the vector
//     PointsWithNeighbors.resize((int)images.size());
//     // We need to initialize our graph, for every image in the input file we will get its neighbors
//     // and store them in a 2d vector the first dimension will represent the Image in the input file
//     // and the second its neighbors
//     for (int i = 0; i < (int)images.size(); i++)
//         for (auto neighbor : lsh.Approximate_kNN(images[i]))
//             PointsWithNeighbors[i].push_back(neighbor.image);
// }

GNNS::~GNNS() {}

std::vector<Neighbor> GNNS::Approximate_kNN(ImagePtr query)
{
    // We are using a set to store the objects efficiently with a custom compare class
    std::set<Neighbor, CompareNeighbor> nearestNeighbors;
    // std::cout << "Query: " << query->id << std::endl;
    // We will do the same update process for all restarts
    for (int r = 0; r < restarts; r++)
    {
        // find Y_0 uniformly over D
        int Y_prev = IntDistribution(0, (int)PointsWithNeighbors.size() - 1);
        int t;
        // We will find new points t-1 times because the first one was done outside the loop
        for (t = 1; t <= 30; t++)
        {
            double min = -1;
            int index = -1;
            // We first check that the LSH returned expansions number of neighbors otherwise we are doing
            // the same process for size() numbers of neighbors
            int limit = (expansions > (int)PointsWithNeighbors[Y_prev].size() ? (int)PointsWithNeighbors[Y_prev].size() : expansions + 1);
            // We skip the first neighbor because it is itself with distance 0 and we go until + 1
            // to take all the expanded neighbors
            for (int i = 1; i < limit; i++)
            {
                // Calculate the distance of the neighbor with the query
                double dist = distance->calculate(PointsWithNeighbors[Y_prev][i], query);
                // Update set with S U N(Y_t-1,E,G)
                nearestNeighbors.insert(Neighbor(PointsWithNeighbors[Y_prev][i], dist));
                // Find Y_t = argmin_Y_in_N(Y_t-1,E,G) δ(Y,query)
                if (min == -1 || dist < min)
                {
                    min = dist;
                    index = PointsWithNeighbors[Y_prev][i]->id;
                }
            }
            if (index == -1)
                break;
            if (nearestNeighbors.size() > 1)
            {
                Neighbor smallest = *nearestNeighbors.begin();
                if (min > smallest.distance)
                    break;
            }

            Y_prev = index;
        }
        // std::cout << "For restart: " << r << std::endl;
        // std::cout << "Stopped at greedy step: " << t << std::endl;
    }
    // The set is already sorted so we can skip the sorting step
    // Lastly we want to make a vector from those neighbors
    std::vector<Neighbor> KnearestNeighbors(nearestNeighbors.begin(), std::next(nearestNeighbors.begin(), std::min(numNn, static_cast<int>(nearestNeighbors.size()))));
    return KnearestNeighbors;
}
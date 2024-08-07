#ifndef LSHCMDARGS_HPP_
#define LSHCMDARGS_HPP_

#include <string>
#include <cstring>

// A simple class which stores the command line argument for lsh algorithms
// The arguments are initialized to their default values except from input,
// query and output file which are necessary in order for the program to execute
class LshCmdArgs
{
public:
    std::string inputFile;    // -d <input file>
    std::string queryFile;    // -q <query file>
    std::string outputFile;   // -o <output file>
    int numHashFuncs;         // –k number of hash functions
    int numHtables;           // -L number of hash tables
    int numNn;                // -Ν number of Nearest Neighbors
    double radius;            // -R radius
    int in_size;              // input size
    int q_size;               // query size
    std::string initDataset;  // -dinit <dataset of init space>
    std::string initQueryset; // -qinit <queryset of init space>

    LshCmdArgs(const int argc, const char *argv[]) : inputFile(""),
                                                     queryFile(""),
                                                     outputFile(""),
                                                     numHashFuncs(4),
                                                     numHtables(5),
                                                     numNn(1),
                                                     radius(10000),
                                                     in_size(-1),
                                                     q_size(-1),
                                                     initDataset(""),
                                                     initQueryset("")
    {
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i], "-d"))
                inputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-q"))
                queryFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-k"))
                numHashFuncs = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-L"))
                numHtables = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-o"))
                outputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-N"))
                numNn = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-R"))
                radius = atof(argv[i + 1]);
            else if (!strcmp(argv[i], "-sd"))
                in_size = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-sq"))
                q_size = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-dinit"))
                initDataset = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-qinit"))
                initQueryset = std::string(argv[i + 1]);
        }
    }
};

#endif
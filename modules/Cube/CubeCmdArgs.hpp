#ifndef CUBECMDARGS_HPP_
#define CUBECMDARGS_HPP_

#include <string>
#include <cstring>

// A simple class which stores the command line argument for hypercube algorithms
// The arguments are initialized to their default values except from input,
// query and output file which are necessary in order for the program to execute
class CubeCmdArgs
{
public:
    std::string inputFile;  // -d <input file>
    std::string queryFile;  // -q <query file>
    std::string outputFile; // -o <output file>
    int dimension;          // –k <int>
    int maxCanditates;      // -M <int>
    int probes;             // -probes <int>
    int numNn;              // -Ν <number of nearest>
    double radius;          // -R radius

    CubeCmdArgs(const int argc, const char *argv[]) : inputFile(""),
                                                      queryFile(""),
                                                      outputFile(""),
                                                      dimension(14),
                                                      maxCanditates(10),
                                                      probes(2),
                                                      numNn(1),
                                                      radius(10000)
    {
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i], "-d"))
                inputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-q"))
                queryFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-k"))
                dimension = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-M"))
                maxCanditates = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-o"))
                outputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-probes"))
                probes = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-N"))
                numNn = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-R"))
                radius = atof(argv[i + 1]);
        }
    }
};

#endif
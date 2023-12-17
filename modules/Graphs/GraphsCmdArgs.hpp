#ifndef GRAPHSCMDARGS_HPP_
#define GRAPHSCMDARGS_HPP_

#include <string>
#include <cstring>

// A simple class which stores the command line argument for graphs algorithms
// The arguments are initialized to their default values except from input,
// query and output file which are necessary in order for the program to execute
// The m and l are initialized to negative values in order for us to know whether
// or not the user "gave" them.
class GraphsCmdArgs
{
public:
    std::string inputFile;  // -d <input file>
    std::string queryFile;  // -q <query file>
    std::string outputFile; // -o <output file>
    int m;                  // -m <1 for GNNS, 2 for MRNG>
    int l;                  // -l <int, only for Search-on-Graph> number of candidates

    int graphNN;    // -k number of Nearest Neighbors in the GRAPH
    int expansions; // -E number of extensions
    int restarts;   // -R number of restarts
    int numNn;      // -Ν number of Nearest Neighbors

    GraphsCmdArgs(const int argc, const char *argv[]) : inputFile(""),
                                                        queryFile(""),
                                                        outputFile(""),
                                                        m(-1),
                                                        l(-1),
                                                        graphNN(50),
                                                        expansions(30),
                                                        restarts(1)
    {
        for (int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i], "-d"))
                inputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-q"))
                queryFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-k"))
                graphNN = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-E"))
                expansions = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-R"))
                restarts = atof(argv[i + 1]);
            else if (!strcmp(argv[i], "-N"))
                numNn = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-l"))
                l = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-m"))
                m = atoi(argv[i + 1]);
            else if (!strcmp(argv[i], "-o"))
                outputFile = std::string(argv[i + 1]);
        }
    }
};

#endif
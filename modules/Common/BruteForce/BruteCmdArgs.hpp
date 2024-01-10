#ifndef BRUTECMDARGS_HPP_
#define BRUTECMDARGS_HPP_

#include <string>
#include <cstring>

// A simple class which stores the command line argument for brute force algorithm
// The arguments are initialized to their default values except from input,
// query and output file which are necessary in order for the program to execute
class BruteCmdArgs
{
public:
    std::string inputFile;    // -d <input file>
    std::string queryFile;    // -q <query file>
    std::string outputFile;   // -o <output file>
    int numNn;                // -Ν <number of nearest>
    int in_size;              // input size
    int q_size;               // query size
    std::string initDataset;  // -dinit <dataset of init space>
    std::string initQueryset; // -qinit <queryset of init space>

    BruteCmdArgs(const int argc, const char *argv[]) : inputFile(""),
                                                       queryFile(""),
                                                       outputFile(""),
                                                       numNn(1),
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
            else if (!strcmp(argv[i], "-o"))
                outputFile = std::string(argv[i + 1]);
            else if (!strcmp(argv[i], "-N"))
                numNn = atoi(argv[i + 1]);
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
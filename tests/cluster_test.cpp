#include <iostream>
#include <cstring>

int main(int argc, char const *argv[])
{
    bool original = false;
    for (int i = 0; i < argc; i++)
        if (!strcmp(argv[i], "-init"))
            original = true;

    if (original)
    {
        ;
        return 0;
    }
    std::cout << "hello from cluster test" << std::endl;
    return 0;
}
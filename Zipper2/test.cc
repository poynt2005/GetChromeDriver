#include "Zipper.h"
#include <string.h>
#include <string>
#include <vector>
#include <iostream>

int main(int argc, char *argv[])
{
    std::vector<std::string> arguments;
    for (int i = 0; i < argc; ++i)
    {
        arguments.emplace_back(argv[i]);
    }

    Zipper zipper;

    if (arguments[1] == "unpack")
    {
        zipper.UnPackArchive(arguments[2], arguments[3]);
    }
    else if (arguments[1] == "pack")
    {

        std::cout << "pack"
                  << "\n";
        zipper.PackArchive(arguments[2], arguments[3]);
    }

    std::cout << zipper.GetErrorCode() << "\n";
    return 0;
}
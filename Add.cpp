#undef UNICODE

#include <iostream>
#include <fstream>
#include <windows.h>

int main(int argc, char **argv)
{
    if (argc != 2 || strcmp(argv[1], "-A")) {
        std::cerr << "\"MiniGit add\" support only \"-A\" option for now." << std::endl;
        return EXIT_FAILURE;
    }


    

    std::cout<<"ss" << std::endl;
    return 0;
}


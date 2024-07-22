#include <iostream>
#include <ios>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <new>
#include <string>
#include <vector>
#undef UNICODE
#include <windows.h>
#include "Crypt.hpp"
#include "Index.hpp"
#include "FileTree.hpp"
#include "CommitObj.hpp"

int main(int argc, char** argv)
{
    std::string sha1_str = argv[1];
    std::string sha1_file_name_pre = sha1_str.substr(0, 2);          // upper 2 digit
    std::string sha1_file_name = sha1_str.substr(2);                 // rest

    // Get current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);

    // obj path
    std::string current_dir_s(current_dir);
    std::string obj_path = current_dir_s + "\\" + sha1_file_name_pre + "\\" + sha1_file_name;

    std::ifstream ifs;
    ifs.open(obj_path, std::ios::in | std::ios::binary);


    ifs.close();
    return EXIT_SUCCESS;
}

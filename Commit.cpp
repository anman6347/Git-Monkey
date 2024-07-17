#undef UNICODE

#include <iostream>
#include <ios>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <new>
#include <string>
#include <vector>
#include <windows.h>

struct Node {
public:
    Node *parent = nullptr;
    Node *left_child = nullptr;
    std::string dir_path;
    std::vector<std::string> childfile_path_list;
};

class FileTree {
public:
    Node *root;
public:
    FileTree(const std::string& dir_name);
    void scan(Node *nd);
};

FileTree::FileTree(const std::string& dir_name)
{
    root = new Node;
    root->dir_path = dir_name;
}

void FileTree::scan(Node *nd)
{
    HANDLE fHandle;
    WIN32_FIND_DATA win32fd;
    
}


// std::vector<std::string> scan_directory(const std::string& dir_name) {

//     HANDLE fHandle;
//     WIN32_FIND_DATA win32fd;
//     std::vector<std::string> file_names;

//     std::string search_name = dir_name + "\\*";

//     fHandle = FindFirstFile(search_name.c_str(), &win32fd);

//     if (fHandle == INVALID_HANDLE_VALUE) {
//         // "'GetLastError() == 3' is 'file not found'"
//         return file_names;
//     }

//     do {
//         if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {  // directory

//             // ignore current directory
//             if (win32fd.cFileName[0] == '.') continue;
//             // ignore .git directory
//             if (strcmp(win32fd.cFileName, ".git") == 0) continue;

//             std::string fullpath = dir_name + "\\" + win32fd.cFileName;


//             //再帰的にファイルを検索
//             std::vector<std::string> files = scan_directory(fullpath);

//             file_names.insert(file_names.end(), files.begin(), files.end());
//         } else {  // file
//             std::string fullpath = dir_name + "\\" + win32fd.cFileName;
//             file_names.emplace_back(fullpath);
//         }
//     } while (FindNextFile(fHandle, &win32fd));

//     FindClose(fHandle);

//     return file_names;
// }


int main(int argc, char **argv)
{
    //std::cout << argv[2] << std::endl;
    if (argc != 3 || strcmp(argv[1], "-m")) {
        std::cerr << "\"MiniGit commit\" support only \"-m\" option for now." << std::endl;
        return EXIT_FAILURE;
    }

    std::string commit_message = argv[2];
    std::cout << "cmsg: "<< commit_message << std:: endl;

    // Get current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);
    std::string current_dir_str(current_dir);


    //
    // we'll develop a function in the near future to determine if modified files have been properly staged
    //


    // create tree objects
    FileTree filetree(current_dir_str);
    std::cout << filetree.root->dir_path << std::endl;

    return EXIT_SUCCESS;
}

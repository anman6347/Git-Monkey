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
#include "Crypt.hpp"

struct Node {
public:
    Node *parent = nullptr;
    Node *left_child = nullptr;
    Node *right_sibling = nullptr;
    std::string dir_path;
    std::vector<std::string> childfile_path_list;
};

class FileTree {
public:
    Node *root;
public:
    FileTree(const std::string& _dir_path);
    //~FileTree();
private:
    void scan(Node *nd);
};

FileTree::FileTree(const std::string& _dir_path)
{
    root = new Node;
    root->dir_path = _dir_path;
    scan(root);
}

void FileTree::scan(Node *nd)
{
    HANDLE fHandle;
    WIN32_FIND_DATA win32fd;
    std::string search_name = nd->dir_path + "\\*";
    fHandle = FindFirstFile(search_name.c_str(), &win32fd);
    if (fHandle == INVALID_HANDLE_VALUE) {
        // "'GetLastError() == 3' is 'file not found'"
        return;
    }

    bool first_dir_flag = true;
    Node *left_sibling = nullptr;
    do {
        if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {  // directory => create dir tree
            // ignore . and .. directory
            if (win32fd.cFileName[0] == '.') continue;
            // ignore .git directory
            if (strcmp(win32fd.cFileName, ".git") == 0) continue;

            if (first_dir_flag) {
                nd->left_child = new Node;
                nd->left_child->dir_path = nd->dir_path + "\\" + win32fd.cFileName;
                nd->left_child->parent = nd;
                left_sibling = nd->left_child;
                first_dir_flag = false;
                scan(nd->left_child);
            } else {
                left_sibling->right_sibling = new Node;
                left_sibling->right_sibling->dir_path = nd->dir_path + "\\" + win32fd.cFileName;
                left_sibling->right_sibling->parent = nd;
                left_sibling = left_sibling->right_sibling;
                scan(left_sibling);
            }
        } else {
            nd->childfile_path_list.push_back(nd->dir_path + "\\" + win32fd.cFileName);
        }

    } while(FindNextFile(fHandle, &win32fd));

    FindClose(fHandle);
    return;
}

void debug_ftree (Node *cur, int space_count){
    if (cur == nullptr) return;

    for (Node *cur2 = cur; cur2 != nullptr; cur2 = cur2->right_sibling) {
        for (int k = 0; k < space_count * 6; k++) std::cout << " ";
        std::cout << "[dir]: " << cur2->dir_path << std::endl;
        for (std::string t: cur2->childfile_path_list) {
            for (int k = 0; k < space_count * 6; k++) std::cout << " ";
            std::cout << "  |---[file]: " << t << std::endl;
            std::cout << calc_blob_sha1_str(t) << std::endl;
        }
        debug_ftree(cur2->left_child, space_count + 1);
    }
    return;
}

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

    Node *root = filetree.root;
    debug_ftree(root, 0);
    return EXIT_SUCCESS;
}

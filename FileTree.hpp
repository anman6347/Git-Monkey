#ifndef __FILETREE_HPP
#define __FILETREE_HPP
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
#include "Index.hpp"

struct Node {
public:
    Node *parent = nullptr;
    Node *left_child = nullptr;
    Node *right_sibling = nullptr;
    std::string dir_path;
    std::string tree_sha1_str;
    uint8_t tree_sha1[20];
    std::vector<std::string> childfile_path_list;
};

class FileTree {
public:
    Node *root;
public:
    FileTree(const std::string& _dir_path);
    //~FileTree();
    Node* get_node_ptr_by_path(std::string &dirpath);
private:
    void scan(Node *nd);    // called in constructor
};


#endif

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
#include "FileTree.hpp"
#include "Compress.hpp"
#include "CommitObj.hpp"


// you must calculate sha1 of child treeObj before calling this func
int create_tree_obj(FileTree *ft, CUR_IDX *cur_idx, std::string &dirpath)
{
    // get node corresponding to the dir
    Node *nd = ft->get_node_ptr_by_path(dirpath);
    if (nd == nullptr) {
        std::cerr << "couldn't find Node : " << dirpath << std::endl;
        return EXIT_FAILURE;
    }
    std::vector<uint8_t> tree_content;

    /**
     * Known bug:
     * We should sord entries in the tree by name.
     * The exception are trees, where you have to pretend that there is a trailing slash.
     */


    // files
    int parent_dir_path_strsize = nd->dir_path.size();
    for (std::string file_path : nd->childfile_path_list) {
        int IdxEnts_index = cur_idx->get_index_of_IdxEnts(file_path);
        if (IdxEnts_index == -1) {
            std::cerr << "IdxEnts_index not found." << std::endl;
            return EXIT_FAILURE;
        }
        uint32_t file_mode = ntohl(cur_idx->IdxEnts[IdxEnts_index].mode);
        // only regular files are supported
        tree_content.push_back('1');
        tree_content.push_back('0');
        tree_content.push_back('0');
        // file permission
        for (int i = 0; i < 3; i++) {
            uint8_t file_permission_1digit = 0;
            for (int j = 0; j < 3; j++) {
                if (file_mode & (1 << (8 - 3 * i - j))) file_permission_1digit +=  1 << (2 - j);
            }
            tree_content.push_back('0' + file_permission_1digit);
        }
        tree_content.push_back(' ');
        // filename
        std::string file_name = file_path.substr(parent_dir_path_strsize + 1, file_path.size() - parent_dir_path_strsize - 1);    // ignore '\'
        for (int i = 0; i < file_name.size(); i++) tree_content.push_back(file_name[i]);
        tree_content.push_back('\0');
        // sha1
        /**
         *  We'll implement a function in the near future to determine if modified files have been properly staged
         *  std::string sha1 = calc_blob_sha1_str(file_path);
        */ 

        for (int i = 19; i >= 0; i--) {
            tree_content.push_back(cur_idx->IdxEnts[IdxEnts_index].sha1[i]);
        }
    }

    // dirs
    for (Node *cur = nd->left_child; cur != nullptr; cur = cur->right_sibling) {
        tree_content.push_back('4');
        tree_content.push_back('0');
        tree_content.push_back('0');
        tree_content.push_back('0');
        tree_content.push_back('0');
        tree_content.push_back(' ');
        // dirname
        std::string dir_name = cur->dir_path.substr(parent_dir_path_strsize + 1, cur->dir_path.size() - parent_dir_path_strsize - 1);    // ignore '\'
        for (int i = 0; i < dir_name.size(); i++) tree_content.push_back(dir_name[i]);
        tree_content.push_back('\0');
        for (int i = 19; i >= 0; i--) tree_content.push_back(cur->tree_sha1[i]);
    }

    // prefix data
    std::string tree_size_str = std::to_string(tree_content.size());
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>('\0'));
    tree_content.insert(tree_content.begin(), tree_size_str.begin(), tree_size_str.end());
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>(' '));
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>('e'));
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>('e'));
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>('r'));
    tree_content.insert(tree_content.begin(), static_cast<uint8_t>('t'));

    nd->tree_sha1_str = calc_sha1_str_from_u8data(tree_content);
    calc_sha1_from_u8data(nd->tree_sha1, tree_content);

    // create tree obj file
    std::string sha1_file_name_pre = nd->tree_sha1_str.substr(0, 2);          // upper two digits
    std::string sha1_file_name = nd->tree_sha1_str.substr(2);                 // rest

    // create dir
    const char *dir_name_buf;
    std::string target_path = ft->root->dir_path + "\\.git\\objects\\" + sha1_file_name_pre;
    dir_name_buf = target_path.c_str();
    if (!CreateDirectory(dir_name_buf, NULL)) {
        switch (GetLastError())
        {
        case ERROR_ALREADY_EXISTS:
            break;
        case ERROR_PATH_NOT_FOUND:
            std::cerr << "err: " << dir_name_buf << " path not found." << std::endl;
            return EXIT_FAILURE;
        default:
            std::cerr << "err: " << dir_name_buf << "unknown error." << std::endl;
            return EXIT_FAILURE;
        }
    }

    // create file
    std::string out_file_name = ".\\.git\\objects\\" + sha1_file_name_pre + "\\" + sha1_file_name;
    const char *Out = out_file_name.c_str();
    do_compress2(tree_content, Out);   // zlib compress

    return EXIT_SUCCESS;
}


void _create_all_tree_obj(FileTree *ft, CUR_IDX *cur_idx, Node *nd)
{
    if (nd == nullptr) return;

    for (Node *cur = nd; cur != nullptr; cur = cur->right_sibling) {
        _create_all_tree_obj(ft, cur_idx, cur->left_child);
        create_tree_obj(ft, cur_idx, cur->dir_path);
    }
    return;
}

void create_all_tree_obj(FileTree *ft, CUR_IDX *cur_idx)
{
    _create_all_tree_obj(ft, cur_idx, ft->root);
    return;
}


int main(int argc, char **argv)
{
    if (argc != 3 || strcmp(argv[1], "-m")) {
        std::cerr << "\"MiniGit commit\" support only \"-m\" option for now." << std::endl;
        return EXIT_FAILURE;
    }


    // set author, e_mail info
    char env_path[255];
    GetModuleFileName(NULL, env_path, 255);
    int index = 254;
    while (env_path[index] != '\\' || index < 0) index--;
    env_path[index] = '\0';
    sprintf(env_path, "%s%s", env_path, "\\.env");
    std::ifstream ifs;
    ifs.open(env_path, std::ios::in);
    if (ifs.fail()) {
        std::cerr << "couldn't open .env file." << std::endl;
        return EXIT_FAILURE;
    }

    std::string author;
    std::string e_mail;

    std::getline(ifs, author);
    std::getline(ifs, e_mail);
    ifs.close();

    // commit message
    std::string commit_message = argv[2];

    // Get current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);
    std::string current_dir_str(current_dir);


    /* create tree objects */

    // create file tree
    FileTree filetree(current_dir_str);
    // read index
    CUR_IDX cur_idx;
    // create tree objects
    create_all_tree_obj(&filetree, &cur_idx);


    /* create commit object */


    CommitObj commitobj(filetree, author, e_mail, commit_message);
    commitobj.create_obj_file();

    return EXIT_SUCCESS;
}

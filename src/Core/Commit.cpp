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
#include "Tree.hpp"

// you must calculate sha1 of child treeObj before calling this func
int create_tree_obj(FileTree *ft, CUR_IDX *cur_idx, std::string &dirpath)
{
    Tree tree(dirpath, ft, cur_idx);
    if (tree.create_tree_bin_data()) {
        std::cerr << "failed to create tree data of " << dirpath << std::endl;
        return EXIT_FAILURE;
    }
    if (tree.cleate_obj()) {
        std::cerr << "failed to create tree object of " << dirpath << std::endl;
        return EXIT_FAILURE;
    }

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

    /* create other files */
    std::ofstream writing_file;
    // \\.git\\logs\\HEAD
    std::string logsHEAD_path = filetree.root->dir_path + "\\.git\\logs\\HEAD";
    writing_file.open(logsHEAD_path, std::ios::app);
    if (ifs.fail()) {
        std::cerr << "couldn't open .git/logs/HEAD" << std::endl;
        return EXIT_FAILURE;
    }
    writing_file << commitobj.parent_sha1_str << " " << commitobj.commit_obj_sha1_str << " " << commitobj.author << " " << commitobj.e_mail << " "
    << std::to_string(commitobj.current_time) << " " << commitobj.timezone << " ";
    if (commitobj.parent_sha1_str == "0000000000000000000000000000000000000000") {
        writing_file << "commit (initial):";
    } else {
        writing_file << "commit:";
    }
    writing_file << " " << commitobj.commit_message << std::endl;
    writing_file.close();

    // \\.git\\refs\\heads\\master
    std::string logs_refs_heads_master_path = filetree.root->dir_path + "\\.git\\logs\\refs\\heads\\master";
    writing_file.open(logs_refs_heads_master_path, std::ios::app);
    if (ifs.fail()) {
        std::cerr << "couldn't open .git/logs/refs/heads/master" << std::endl;
        return EXIT_FAILURE;
    }
    writing_file << commitobj.parent_sha1_str << " " << commitobj.commit_obj_sha1_str << " " << commitobj.author << " " << commitobj.e_mail << " "
    << std::to_string(commitobj.current_time) << " " << commitobj.timezone << " ";
    if (commitobj.parent_sha1_str == "0000000000000000000000000000000000000000") {
        writing_file << "commit (initial):";
    } else {
        writing_file << "commit:";
    }
    writing_file << " " << commitobj.commit_message << std::endl;
    writing_file.close();

    // \\.git\\refs\\heads\\master
    std::string refs_heads_master_path = filetree.root->dir_path + "\\.git\\refs\\heads\\master";
    writing_file.open(refs_heads_master_path, std::ios::out);
    if (ifs.fail()) {
        std::cerr << "couldn't open .git/refs/heads/master" << std::endl;
        return EXIT_FAILURE;
    }
    writing_file << commitobj.commit_obj_sha1_str << std::endl;
    writing_file.close();

    // \\.git\\COMMIT_EDITMSG
    std::string commit_editmsg_path = filetree.root->dir_path + "\\.git\\COMMIT_EDITMSG";
    writing_file.open(commit_editmsg_path, std::ios::out);
    if (ifs.fail()) {
        std::cerr << "couldn't open .git/COMMIT_EDITMSG" << std::endl;
        return EXIT_FAILURE;
    }
    writing_file << commitobj.commit_message << std::endl;
    writing_file.close();
    return EXIT_SUCCESS;
}

#ifndef __TREE_HPP
#define __TREE_HPP

#include "GitFile.hpp"
#include "FileTree.hpp"
#include "Index.hpp"

struct TREE_HEADER {
    std::string sig = "tree";
    std::string content_size;
};

struct TREE_ENT {
    std::vector<uint8_t> file_mode;     /** 6 bit if regular file, 5 bit if directory*/
    std::string file_name;
    std::string sha1_str;
    std::vector<uint8_t> sha1_bin;
};

/**
 * @brief Tree object data (header info + tree entries)
 */
struct TREE_OBJ {
    TREE_HEADER th;
    std::vector<TREE_ENT> tree_entry;
};


/**
 * <Known bug>
 * We should sort entries in the tree by name.
 * The exception are trees, where you have to pretend that there is a trailing slash.
 */
class Tree : public GitFile {
public:
    FileTree *ft;
    CUR_IDX *idx;
    TREE_OBJ tree_data;
public:
    Tree(std::string _target_file_path, FileTree *_ft, CUR_IDX *_idx) : GitFile(_target_file_path), ft(_ft), idx(_idx) {};
    /**
     * @brief Create a tree object data and set it in @link GitFile::bin_data @endlink.
     * @return 0 if success
     * @details For now, error handling is not implemented in this function.
     */
    int create_tree_bin_data();
    // int get_tree_bin_data();
};

#endif

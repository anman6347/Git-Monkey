#include "Tree.hpp"

int Tree::create_tree_bin_data()
{
    // get node corresponding to the dir
    Node *nd = ft->get_node_ptr_by_path(target_file_path);
    if (nd == nullptr) {
        std::cerr << "couldn't find Node : " << target_file_path << std::endl;
        return EXIT_FAILURE;
    }

    // files
    int parent_dir_path_strsize = nd->dir_path.size();
    for (std::string file_path : nd->childfile_path_list) {
        // create tree entry
        TREE_ENT tree_ent;

        int IdxEnts_index = idx->get_index_of_IdxEnts(file_path);
        if (IdxEnts_index == -1) {
            std::cerr << file_path << ": " << std::endl;
            std::cerr << "IdxEnts_index not found." << std::endl;
            return EXIT_FAILURE;
        }
        uint32_t file_mode = ntohl(idx->IdxEnts[IdxEnts_index].mode);

        // only regular files are supported
        std::string regular_file_mode = "100";
        tree_ent.file_mode.insert(tree_ent.file_mode.end(), regular_file_mode.begin(), regular_file_mode.end());
        // file permission
        for (int i = 0; i < 3; i++) {
            uint8_t file_permission_1digit = 0;
            for (int j = 0; j < 3; j++) {
                if (file_mode & (1 << (8 - 3 * i - j))) file_permission_1digit +=  1 << (2 - j);
            }
            tree_ent.file_mode.push_back('0' + file_permission_1digit);
        }

        // filename
        tree_ent.file_name = file_path.substr(parent_dir_path_strsize + 1, file_path.size() - parent_dir_path_strsize - 1);    // ignore '\'

        // sha1 (big endian to big endian)
        for (int i = 0; i < 20; i++) {
            tree_ent.sha1_bin.push_back(idx->IdxEnts[IdxEnts_index].sha1[i]);
        }

        tree_data.tree_entry.push_back(tree_ent);
    }

    // dirs
    for (Node *cur = nd->left_child; cur != nullptr; cur = cur->right_sibling) {
        // create tree entry
        TREE_ENT tree_ent;

        std::string dirs_file_mode = "40000";
        tree_ent.file_mode.insert(tree_ent.file_mode.end(), dirs_file_mode.begin(), dirs_file_mode.end());
        
        // dirname
        tree_ent.file_name = cur->dir_path.substr(parent_dir_path_strsize + 1, cur->dir_path.size() - parent_dir_path_strsize - 1);    // ignore '\'

        // sha1 (big endian to big endian)
        for (int i = 0; i < 20; i++) {
            tree_ent.sha1_bin.push_back(cur->tree_sha1[i]);
        }

        tree_data.tree_entry.push_back(tree_ent);
    }

    /**
     *  We'll implement a function in the near future to determine if modified files have been properly staged
     *  std::string sha1 = calc_blob_sha1_str(file_path);
     */ 

    // calculate and set tree content size
    int tree_content_size = 0;
    for (TREE_ENT tree_ent : tree_data.tree_entry) {
        tree_content_size += tree_ent.file_mode.size();
        tree_content_size += 1;     // white space
        tree_content_size += tree_ent.file_name.size();
        tree_content_size += 1;     // null str
        tree_content_size += 20;    // sha1
    }
    tree_data.th.content_size = std::to_string(tree_content_size);

    // create tree binary data

    bin_data.insert(bin_data.end(), tree_data.th.sig.begin(), tree_data.th.sig.end());
    bin_data.push_back(' ');
    for(int i = 0; i < tree_data.th.content_size.size(); i++) {
        bin_data.push_back(static_cast<uint8_t>(tree_data.th.content_size[i]));
    }
    bin_data.push_back('\0');
    for (TREE_ENT tree_ent : tree_data.tree_entry) {
        bin_data.insert(bin_data.end(), tree_ent.file_mode.begin(), tree_ent.file_mode.end());
        bin_data.push_back(' ');
        bin_data.insert(bin_data.end(), tree_ent.file_name.begin(), tree_ent.file_name.end());
        bin_data.push_back('\0');
        for (int i = 19; i >= 0; i--) {
            bin_data.push_back(tree_ent.sha1_bin[i]);
        }
    }

    // set sha1 of this tree object
    nd->tree_sha1_str = calc_sha1_str_from_u8data(bin_data);
    calc_sha1_from_u8data(nd->tree_sha1, bin_data);

    return EXIT_SUCCESS;
}
#include "FileTree.hpp"

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
        // std::cout << nd->dir_path + "\\" + win32fd.cFileName << std::endl;
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


static Node* _get_node_ptr_by_path(Node *nd, std::string &dirpath)
{
    if (nd == nullptr) return nullptr;

    for (Node *cur = nd; cur != nullptr; cur = cur->right_sibling) {
        if (cur->dir_path == dirpath) {
            return cur;
        }
        Node *res = _get_node_ptr_by_path(cur->left_child, dirpath);
        if (res != nullptr) {
            return res;
        };
    }

    return nullptr;
}

Node* FileTree::get_node_ptr_by_path(std::string &dirpath)
{
    return _get_node_ptr_by_path(root, dirpath);
}

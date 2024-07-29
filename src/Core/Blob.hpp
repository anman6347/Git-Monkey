#ifndef __BLOB_HPP
#define __BLOB_HPP

#include "GitFile.hpp"


struct BLOB_HEADER {
    std::string sig = "blob";
    std::string content_size;
};

/**
 * @brief Blob object data (header info + content)
 */
struct BLOB_OBJ {
    BLOB_HEADER bh;
    std::vector<uint8_t> content;
};

class Blob : public GitFile {
public:
    BLOB_OBJ blob_data;
public:
    Blob(std::string _target_file_path) : GitFile(_target_file_path) {};
    /**
     * @brief Create a blob object data and set it in @link GitFile::bin_data @endlink.
     * @return 0 if success
     * @details For now, error handling is not implemented in this function.
     */
    int create_blob_bin_data();
    // int get_blob_data();
};

#endif

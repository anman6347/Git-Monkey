#ifndef __GITFILE_HPP
#define __GITFILE_HPP
#undef UNICODE

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdint.h>
#include <windows.h>
#include "Crypt.hpp"
#include "Compress.hpp"

class GitFile {
public:
    std::string target_file_path;        // Write Obj: none
    std::vector<uint8_t> bin_data;
    std::string text_data; 
    std::string sha1_str;
public:
    GitFile(std::string _target_file_path) : target_file_path(_target_file_path) {};
    /**
     * @brief create binary file from @link GitFile::bin_data @endlink.
     * @return 0 if success, otherwise 1.
     */
    int cleate_bin_file();
    /**
     * @brief create zlib compressed binary file from @link GitFile::bin_data @endlink in .git/objects.
     * @return 0 if success, otherwise 1.
     */
    int cleate_obj();
    /**
     * @brief write text file on @link GitFile::target_file_path @endlink.
     * @param mode you should use `std::ios::out` or `std::ios::app`
     * @return 0 if success, otherwise 1.
     */
    int write_text_file(std::ios_base::openmode mode);
    /**
     * @brief get binary data of the file specified by @link GitFile::target_file_path @endlink.
     * @return 0 if success, otherwise 1.
     */
    int get_bin_data();
    /**
     * @brief get text data of the file specified by @link GitFile::target_file_path @endlink.
     * @return 0 if success, otherwise 1.
     */
    int get_text_data();
    /**
     * @brief get binary data of the zlib compressed file specified by @link GitFile::target_file_path @endlink.
     * @return 0 if success, otherwise 1.
     */
    int get_obj_data();
    /**
     * @brief set @link  GitFiles::sha1_str @endlink calculated from @link GitFiles::bin_data @endlink
     * @return 0 if success, otherwise 1.
     * @details For now, error handling is not implemented in this function.
     */
    int calc_sha1_str();
};

#endif

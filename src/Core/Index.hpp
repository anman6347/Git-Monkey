#ifndef __INDEX_HPP
#define __INDEX_HPP
#undef UNICODE

#include <stdint.h>
#include <iostream>
#include <iomanip>
#include <ios>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <windows.h>

// Index entry
struct IndexEntry {
    uint32_t ctime_seconds;                             // the last time a file's metadata changed
    uint32_t ctime_nanosecond_fractions;
    uint32_t mtime_seconds;                             //  the last time a file's data changed
    uint32_t mtime_nanosecond_fractions;
    uint32_t dev;                                       // dev num
    uint32_t ino;                                       // inode
    uint32_t mode = 0;                                  // permisson mode (big endian)
    uint32_t uid;
    uint32_t gid;
    uint32_t file_size;                                 // on-disk size
    uint8_t sha1[20];                                   // 160-bit SHA-1 for the represented object
    uint16_t flags;                                     
    char *entry_path_name;                              // Entry path name (variable length) relative to top level directory
    // std::string path_name;                           // Entry path name (for now, this is used when reading index)
    uint8_t pad_size;                                   // 1-8 null bytes as necessary to pad the entry to a multiple of eight bytes
};

// Index
struct INDEX {   
    // All binary numbers are in network byte order
    char sig[4] = {'D', 'I', 'R', 'C'};                 // (stands for "dircache")
    uint32_t version = htonl(2);                        // version 2
    uint32_t entsize;                                   // index entry size
    int ient_off = 12;                                  // entries offset
};

class CUR_IDX {
public:
    std::string current_index_path;
    std::vector<IndexEntry> IdxEnts;
public:
    CUR_IDX();
    std::string convert_sha1_to_str(int index);
    std::string get_full_path_str(int index);
    int get_index_of_IdxEnts(std::string &file_path);
private:
    int get_entries();      // called in constructor
};


#endif

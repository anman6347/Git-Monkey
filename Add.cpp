#undef UNICODE

#include <iostream>
#include <ios>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include "Crypt.hpp"
#include <windows.h>
#include "Compress.hpp"

std::vector<std::string> scan_directory(const std::string& dir_name) {

    HANDLE fHandle;
    WIN32_FIND_DATA win32fd;
    std::vector<std::string> file_names;

    std::string search_name = dir_name + "\\*";

    fHandle = FindFirstFile(search_name.c_str(), &win32fd);

    if (fHandle == INVALID_HANDLE_VALUE) {
        // "'GetLastError() == 3' is 'file not found'"
        return file_names;
    }

    do {
        if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {  //ディレクトリである

            // そのディレクトリそのものを表す場合は処理しない
            if (win32fd.cFileName[0] == '.') continue;
            // .git は無視
            if (strcmp(win32fd.cFileName, ".git") == 0) continue;


            std::string fullpath = dir_name + "\\" + win32fd.cFileName;
            //再帰的にファイルを検索
            std::vector<std::string> files = scan_directory(fullpath);

            file_names.insert(file_names.end(), files.begin(), files.end());
        }
        else {  // ファイルである
            std::string fullpath = dir_name + "\\" + win32fd.cFileName;
            file_names.emplace_back(fullpath);
        }
    } while (FindNextFile(fHandle, &win32fd));

    FindClose(fHandle);

    return file_names;
}

// Index entry
struct IndexEntry {
    uint32_t ctime_seconds;                             // the last time a file's metadata changed
    uint32_t ctime_nanosecond_fractions;
    uint32_t mtime_seconds;                             //  the last time a file's data changed
    uint32_t mtime_nanosecond_fractions;
    uint32_t dev;                                       // dev num
    uint32_t ino;                                       // inode
    uint8_t mode[32];                                   // permisson mode
    uint32_t uid;
    uint32_t gid;
    uint32_t file_size;                                 // on-disk size
    uint8_t sha1[160];                                  // 160-bit SHA-1 for the represented object
    uint16_t flags;                                     
    char *entry_path_name;                              // Entry path name (variable length) relative to top level directory
    uint8_t pad_size;                                   // 1-8 nul bytes as necessary to pad the entry to a multiple of eight bytes
};

// Index
struct INDEX {   
    // All binary numbers are in network byte order
    unsigned char sig[4] = {'D', 'I', 'R', 'C'};        // (stands for "dircache")
    uint32_t version = htonl(2);                        // version 2
    uint32_t entsize;                                   // index entry size
    IndexEntry *ient;                                   // entries
};


int create_index(std::vector<std::string> &child_files)
{
    INDEX Index;
    Index.entsize = htonl(child_files.size());

    for(std::string file_name : child_files) {

    }


    
    return EXIT_SUCCESS;
}



int main(int argc, char **argv)
{
    if (argc != 2 || strcmp(argv[1], "-A")) {
        std::cerr << "\"MiniGit add\" support only \"-A\" option for now." << std::endl;
        return EXIT_FAILURE;
    }

    // Get current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);

    // get child files
    std::string current_dir_s(current_dir);
    std::vector<std::string> child_files = scan_directory(current_dir_s);

    /* create blob objects for all child files*/
    for(std::string file_name : child_files) {
        std::ifstream ifs(file_name, std::ios::in);     // not binary mode to change newline from CRLF to LF
        if (ifs.fail()) {
            std::cerr << "Failed to open file." << std::endl;
            return EXIT_FAILURE;
        }

        std::vector<char> buffer{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
        std::vector<unsigned char> blob_raw_data;
        blob_raw_data.push_back('b');
        blob_raw_data.push_back('l');
        blob_raw_data.push_back('o');
        blob_raw_data.push_back('b');
        blob_raw_data.push_back(' ');
        std::string size_info = std::to_string(buffer.size());
        for(int i = 0; i < size_info.size(); i++) {
            blob_raw_data.push_back((unsigned char)size_info[i]);
        }
        blob_raw_data.push_back('\0');
        blob_raw_data.insert(blob_raw_data.end(), buffer.begin(), buffer.end());

        // calc sha-1
        NTSTATUS status = STATUS_UNSUCCESSFUL;
        std::vector<UCHAR> hash1;
        status = CreateHash(BCRYPT_SHA1_ALGORITHM, blob_raw_data, hash1);
        if (!NT_SUCCESS(status)) {
            std::cout << "Error: " << status << std::endl;
            return EXIT_FAILURE;
        }


        std::ostringstream ss;
        for (size_t i = 0; i < hash1.size(); i++) {
            ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (int)hash1.at(i);      // 1 byte => 2-digit hexadecimal number
        }

        std::string tmp = ss.str();
        std::string sha1_file_name_pre = tmp.substr(0, 2);          // 上位 2 桁
        std::string sha1_file_name = tmp.substr(2);                 // 残り


        // save blob object

        // create dir
        char dir_name_buf[255];
        std::string path_blob = "\\.git\\objects\\" + sha1_file_name_pre;
        const char *path_blob_c = path_blob.c_str();
        sprintf(dir_name_buf, "%s%s", current_dir, path_blob_c);
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

        // std::ofstream writing_file;
        // writing_file.open(out_file_name, std::ios::out | std::ios::binary);
        // for (unsigned char u : blob_raw_data) {
        //     writing_file.write((char *)(&u), sizeof(unsigned char));
        // }
        // writing_file.close();
        //const char *In = file_name.c_str();
        const char *Out = out_file_name.c_str();
        do_compress2(blob_raw_data, Out);   // zlib compress


        // create/update index

    }

    return 0;
}


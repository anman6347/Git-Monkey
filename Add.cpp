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

    // create blob objects
    for(std::string file_name : child_files) {
        std::ifstream ifs(file_name, std::ios::in);     // not binary mode to change newline from CRLF to LF
        if (ifs.fail()) {
            std::cerr << "Failed to open file." << std::endl;
            return -1;
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
            return -1;
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
        //do_compress(In, Out);   // zlib 圧縮
        do_compress2(blob_raw_data, Out);   // zlib 圧縮
    }

    return 0;
}


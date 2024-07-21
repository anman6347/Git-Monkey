#undef UNICODE

#include <iostream>
#include <ios>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <new>
#include <vector>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "Crypt.hpp"
#include <windows.h>
#include "Compress.hpp"
#include "Index.hpp"

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



int create_index(std::string &current_dir, std::vector<std::string> &child_files, std::vector<std::string> &sha1_list)
{
    INDEX Index;
    Index.entsize = htonl(child_files.size());

    // Index entries data
    std::vector<IndexEntry> IdxEnts;

    // create Index entries data
    int i = 0;
    for(std::string file_path : child_files) {
        IndexEntry IdxEnt;

        // not used in windows
        IdxEnt.dev = 0;
        IdxEnt.ino = 0;
        IdxEnt.uid = 0;
        IdxEnt.gid = 0;

        // get file info via stat(2)
        struct stat buf;
        time_t ctime, mtime;
        mode_t mode;

        stat(file_path.c_str(), &buf);
        ctime = buf.st_ctime;
        mtime = buf.st_mtime;
        mode = buf.st_mode;
        if (!S_ISREG(mode)) {
            std::cerr << "MiniGit supports only regular file." << std::endl;
            return EXIT_FAILURE;
        }

        IdxEnt.ctime_seconds = htonl((uint32_t)ctime);
        IdxEnt.ctime_nanosecond_fractions = 0;      // unknown
        IdxEnt.mtime_seconds = htonl((uint32_t)mtime);
        IdxEnt.mtime_nanosecond_fractions = 0;      // unknown

        // regular file
        IdxEnt.mode |= (1 << 15);
        IdxEnt.mode &= ~(1 << 14);
        IdxEnt.mode &= ~(1 << 13);
        IdxEnt.mode &= ~(1 << 12);
        // reserved
        IdxEnt.mode &= ~(1 << 11);
        IdxEnt.mode &= ~(1 << 10);
        IdxEnt.mode &= ~(1 << 9);
        // permission (644 or 755)
        // windows only use 644 ?
        IdxEnt.mode |= (1 << 8);
        IdxEnt.mode |= (1 << 7);
        IdxEnt.mode &= ~(1 << 6);
        IdxEnt.mode |= (1 << 5);
        IdxEnt.mode &= ~(1 << 4);
        IdxEnt.mode &= ~(1 << 3);
        IdxEnt.mode |= (1 << 2);
        IdxEnt.mode &= ~(1 << 1);
        IdxEnt.mode &= ~(1 << 0);

        IdxEnt.mode = htonl(IdxEnt.mode);

        IdxEnt.file_size = htonl(buf.st_size);

        std::string sha1_str = sha1_list[i];
        for(int k = 0; k < 40; k += 2) {
            std::string hex_2digit = sha1_str.substr(k, 2);
            IdxEnt.sha1[19 - (k / 2)] = std::stoul(hex_2digit, nullptr, 16);        // big endian
        }

        int filename_size = file_path.size() - current_dir.size() - 1;      // ignore first '\'
        if(filename_size >= (1 << 12)) {
            std::cerr << "long file name is not supported." << std::endl;
            return EXIT_FAILURE;
        }
        IdxEnt.flags = htons(filename_size);

        // entry path name
        std::string entry_path_name_raw = (file_path.substr(current_dir.size() + 1, filename_size)).c_str();     // ignore first '\'
        for(int k = 0; k < entry_path_name_raw.size(); k++) {
            if (entry_path_name_raw[k] == '\\') {
                entry_path_name_raw[k] = '/';
            }
        }

        char* tmp = new char[filename_size + 1];
        strcpy(tmp, entry_path_name_raw.c_str());
        IdxEnt.entry_path_name = tmp;

        // padding size
        IdxEnt.pad_size = 8 - ((62 + filename_size) % 8);

        // save Index entry data
        IdxEnts.push_back(IdxEnt);

        i++;
    }

    // create Index
    std::string index_file_path = ".\\.git\\index";
    std::ofstream writing_file;
    writing_file.open(index_file_path, std::ios::out | std::ios::binary);

    // write data to index
    writing_file.write(Index.sig, sizeof(unsigned char) * 4);
    writing_file.write((char *)&Index.version, sizeof(uint32_t));
    writing_file.write((char *)&Index.entsize, sizeof(uint32_t));
    for (int i = 0; i < IdxEnts.size(); i++) {
        writing_file.write((char *)&IdxEnts[i].ctime_seconds, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].ctime_nanosecond_fractions, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].mtime_seconds, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].mtime_nanosecond_fractions, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].dev, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].ino, sizeof(uint32_t));
        writing_file.write((char *)(&IdxEnts[i].mode), sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].uid, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].gid, sizeof(uint32_t));
        writing_file.write((char *)&IdxEnts[i].file_size, sizeof(uint32_t));
        for (int j = 19; j >= 0; j--) {
            writing_file.write((char *)(&IdxEnts[i].sha1[j]), sizeof(unsigned char));
        }
        writing_file.write((char *)&IdxEnts[i].flags, sizeof(uint16_t));
        writing_file.write((char *)(IdxEnts[i].entry_path_name), ntohs(IdxEnts[i].flags));
        for (int j = 0; j < IdxEnts[i].pad_size; j++) {
            writing_file.write("\0", sizeof(char));
        }
        delete [] IdxEnts[i].entry_path_name;
    }
    writing_file.close();


    // calc sha1 checksum
    std::ifstream ifs;
    int index_file_size;

    ifs.open(index_file_path, std::ios::in | std::ios::binary);

    ifs.seekg(0, ifs.end);
    index_file_size = static_cast<int>(ifs.tellg());
    ifs.seekg(0, ifs.beg);

    std::vector<unsigned char> buffer(index_file_size, 0);
    ifs.read(reinterpret_cast<char *>(buffer.data()), index_file_size);
    ifs.close();

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    std::vector<UCHAR> hash1;
    status = CreateHash(BCRYPT_SHA1_ALGORITHM, buffer, hash1);
    if (!NT_SUCCESS(status)) {
        std::cout << "Error: " << status << std::endl;
        return EXIT_FAILURE;
    }
    std::ostringstream ss;
    for (size_t i = 0; i < hash1.size(); i++) {
        ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (int)hash1.at(i);      // 1 byte => 2-digit hexadecimal number
    }
    std::string sha1_str = ss.str();

    // write checksum
    writing_file.open(index_file_path, std::ios::app | std::ios::binary);
    for(int k = 0; k < 40; k += 2) {
        std::string hex_2digit = sha1_str.substr(k, 2);
        uint8_t tmp =  static_cast<uint8_t>(std::stoul(hex_2digit, nullptr, 16));
        writing_file.write((char *)(&tmp), sizeof(uint8_t));
    }
    writing_file.flush();
    writing_file.close();
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
    std::vector<std::string> sha1_list;     // sha1

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

        std::string sha1 = ss.str();
        sha1_list.push_back(sha1);
        std::string sha1_file_name_pre = sha1.substr(0, 2);          // 上位 2 桁
        std::string sha1_file_name = sha1.substr(2);                 // 残り

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
        const char *Out = out_file_name.c_str();
        do_compress2(blob_raw_data, Out);   // zlib compress
    }

    // create/update index
    create_index(current_dir_s, child_files, sha1_list);

    return 0;
}


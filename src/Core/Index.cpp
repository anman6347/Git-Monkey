#include "Index.hpp"

CUR_IDX::CUR_IDX()
{
    // set current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);
    current_index_path = current_dir;       // call copy constructor
    current_index_path += "\\.git\\index";

    // read index
    if (get_entries()) {
        std::cerr << "couldn't read the index file" << std::endl;
        exit(EXIT_FAILURE);
    }
}

inline int next_cur(int &cur)
{
    int past = cur;
    cur += 4;
    return past;
}


int CUR_IDX::get_entries()
{
    std::ifstream ifs;
    ifs.open(current_index_path, std::ios::in | std::ios::binary);
    if (ifs.fail()) {
        std::cerr << "Failed to open Index." << std::endl;
        return EXIT_FAILURE;
    }

    std::vector<char> buffer_c{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    ifs.close();
    std::vector<uint8_t> buffer_u8;
    std::vector<uint32_t> buffer_u32;
    buffer_u8.insert(buffer_u8.end(), buffer_c.begin(), buffer_c.end());
    buffer_c.clear();

    uint8_t *base = buffer_u8.data();
    int cur_index = 8;
    int ent_size = ntohl(*reinterpret_cast<uint32_t*>(base + next_cur(cur_index)));

    for (int k = 0; k < ent_size; k++) {
        IndexEntry IdxEnt;
        IdxEnt.ctime_seconds = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.ctime_nanosecond_fractions = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.mtime_seconds = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.mtime_nanosecond_fractions = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.dev = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.ino = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.mode = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.uid = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.gid = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        IdxEnt.file_size = *reinterpret_cast<uint32_t*>(base + next_cur(cur_index));
        for (int i = 19; i >= 0; i--) {
            IdxEnt.sha1[i] = buffer_u8[cur_index];
            cur_index++;
        }
        IdxEnt.flags = *reinterpret_cast<uint16_t*>(base + cur_index);
        cur_index += 2;
        // IdxEnt.entry_path_name = reinterpret_cast<char*>(base + cur_index);
        IdxEnt.entry_path_name = new char[(ntohs(IdxEnt.flags) & 0xFFF) + 1];
        for (int i = 0; i < (ntohs(IdxEnt.flags) & 0xFFF); i++) {
            char *cur = reinterpret_cast<char*>(base) + cur_index;
            IdxEnt.entry_path_name[i] = *(cur);
            cur_index++;
        }
        IdxEnt.entry_path_name[(ntohs(IdxEnt.flags) & 0xFFF)] = '\0';
        IdxEnt.pad_size = 8 - ((62 + (ntohs(IdxEnt.flags) & 0xFFF)) % 8);   // padding size
        cur_index += IdxEnt.pad_size;                                       // add pad size
        IdxEnts.push_back(IdxEnt);
    }

    // for (int k = 0; k < ent_size; k++) {
    //     std::cout << IdxEnts[k].entry_path_name << std::endl;
    // }
    return 0;
}

// big endian
std::string CUR_IDX::convert_sha1_to_str(int index)
{
    IndexEntry IdxEnt = IdxEnts[index];
    std::ostringstream ss;
    for (int i = 19; i >= 0; i--) {
        ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (int)IdxEnt.sha1[i];      // 1 byte => 2-digit hexadecimal number
    }
    return ss.str();
}


std::string CUR_IDX::get_full_path_str(int index)
{
    IndexEntry IdxEnt = IdxEnts[index];
    std::string short_path = "";
    for (int i = 0; i < (ntohs(IdxEnt.flags) & 0xFFF); i++) {
        if (IdxEnt.entry_path_name[i] != '/') {
            short_path += IdxEnt.entry_path_name[i];
        } else {
            short_path += '\\';
        }
    }
    std::string res = current_index_path.substr(0, current_index_path.size() - 10);     // 10 = ".git\index"
    res += short_path;
    return res;
}


int CUR_IDX::get_index_of_IdxEnts(std::string &file_path)
{
    int res = -1;
    for (int i = 0; i < IdxEnts.size(); i++) {
        if (file_path == get_full_path_str(i)) return i;
    }
    return res;
}

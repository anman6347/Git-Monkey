#include "CommitObj.hpp"

int CommitObj::create_obj_file()
{
    // get current time
    get_and_Set_current_time();
    std::string current_time_str = std::to_string(current_time);

    // commit object data
    std::vector<uint8_t> raw_commit_data;

    // create root tree data
    std::vector<uint8_t> root_tree_data;
    root_tree_data.push_back('t');
    root_tree_data.push_back('r');
    root_tree_data.push_back('e');
    root_tree_data.push_back('e');
    root_tree_data.push_back(' ');
    std::string root_tree_sha1 = current_ft.root->tree_sha1_str;
    root_tree_data.insert(root_tree_data.end(), root_tree_sha1.begin(), root_tree_sha1.end());
    root_tree_data.push_back('\n');
    // write
    raw_commit_data.insert(raw_commit_data.end(), root_tree_data.begin(), root_tree_data.end());
    root_tree_data.clear();


    // create parent data
    std::vector<uint8_t> parent_data;
    parent_data.push_back('p');
    parent_data.push_back('a');
    parent_data.push_back('r');
    parent_data.push_back('e');
    parent_data.push_back('n');
    parent_data.push_back('t');
    parent_data.push_back(' ');
    read_parent_sha1();     // get parent sha1
    parent_data.insert(parent_data.end(), parent_sha1_str.begin(), parent_sha1_str.end());
    parent_data.push_back('\n');
    // write
    if (parent_sha1_str != "0000000000000000000000000000000000000000") {
        raw_commit_data.insert(raw_commit_data.end(), parent_data.begin(), parent_data.end());
    }
    parent_data.clear();


    // create author data
    std::vector<uint8_t> author_data;
    author_data.push_back('a');
    author_data.push_back('u');
    author_data.push_back('t');
    author_data.push_back('h');
    author_data.push_back('o');
    author_data.push_back('r');
    author_data.push_back(' ');
    author_data.insert(author_data.end(), author.begin(), author.end());
    author_data.push_back(' ');
    author_data.insert(author_data.end(), e_mail.begin(), e_mail.end());
    author_data.push_back(' ');
    author_data.insert(author_data.end(), current_time_str.begin(), current_time_str.end());
    author_data.push_back(' ');
    author_data.insert(author_data.end(), timezone.begin(), timezone.end());
    author_data.push_back('\n');
    // write
    raw_commit_data.insert(raw_commit_data.end(), author_data.begin(), author_data.end());
    author_data.clear();


    // create commiter data
    std::vector<uint8_t> commiter_data;
    commiter_data.push_back('c');
    commiter_data.push_back('o');
    commiter_data.push_back('m');
    commiter_data.push_back('m');
    commiter_data.push_back('i');
    commiter_data.push_back('t');
    commiter_data.push_back('t');
    commiter_data.push_back('e');
    commiter_data.push_back('r');
    commiter_data.push_back(' ');
    commiter_data.insert(commiter_data.end(), author.begin(), author.end());
    commiter_data.push_back(' ');
    commiter_data.insert(commiter_data.end(), e_mail.begin(), e_mail.end());
    commiter_data.push_back(' ');
    commiter_data.insert(commiter_data.end(), current_time_str.begin(), current_time_str.end());
    commiter_data.push_back(' ');
    commiter_data.insert(commiter_data.end(), timezone.begin(), timezone.end());
    commiter_data.push_back('\n');
    // write
    raw_commit_data.insert(raw_commit_data.end(), commiter_data.begin(), commiter_data.end());
    commiter_data.clear();


    // commit message
    raw_commit_data.push_back('\n');
    raw_commit_data.insert(raw_commit_data.end(), commit_message.begin(), commit_message.end());
    raw_commit_data.push_back('\n');

    // prefix data
    std::string commit_data_size_str = std::to_string(raw_commit_data.size());
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('\0'));
    raw_commit_data.insert(raw_commit_data.begin(), commit_data_size_str.begin(), commit_data_size_str.end());
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>(' '));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('t'));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('i'));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('m'));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('m'));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('o'));
    raw_commit_data.insert(raw_commit_data.begin(), static_cast<uint8_t>('c'));


    // create obj file
    commit_obj_sha1_str = calc_sha1_str_from_u8data(raw_commit_data);
    std::string sha1_file_name_pre = commit_obj_sha1_str.substr(0, 2);          // upper two digits
    std::string sha1_file_name = commit_obj_sha1_str.substr(2); 
    // create dir
    const char *dir_name_buf;
    std::string target_path = current_ft.root->dir_path + "\\.git\\objects\\" + sha1_file_name_pre;
    dir_name_buf = target_path.c_str();
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
    do_compress2(raw_commit_data, Out);   // zlib compress

    return EXIT_SUCCESS;
}


int CommitObj::get_and_Set_current_time()
{
    auto now = std::chrono::system_clock::now();
    current_time = std::chrono::system_clock::to_time_t(now);
    return EXIT_SUCCESS;
}



static int create_dir(std::string &dir_path)
{
    const char *dir_path_c = dir_path.c_str();
    if (!CreateDirectory(dir_path_c, NULL)) {
        switch (GetLastError())
        {
        case ERROR_ALREADY_EXISTS:
            return ERROR_ALIAS_EXISTS;
        case ERROR_PATH_NOT_FOUND:
            std::cerr << "err: " << dir_path_c << " path not found." << std::endl;
            return EXIT_FAILURE;
        default:
            std::cerr << "err: " << dir_path_c << "unknown error." << std::endl;
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

int CommitObj::read_parent_sha1()
{

    std::string logsHEAD_file_path = current_ft.root->dir_path + "\\.git\\logs\\HEAD";
    std::ifstream ifs;
    ifs.open(logsHEAD_file_path, std::ios::in);

    if (ifs.fail()) {       // Initial commit
        std::string logs_path = current_ft.root->dir_path + "\\.git\\logs";
        std::string logs_refs_path = current_ft.root->dir_path + "\\.git\\logs\\refs";
        std::string logs_refs_heads_path = current_ft.root->dir_path + "\\.git\\logs\\refs\\heads";
        std::string refsheads_path = current_ft.root->dir_path + "\\.git\\refs\\heads";
        create_dir(logs_path);
        create_dir(logs_refs_path);
        create_dir(logs_refs_heads_path);
        create_dir(refsheads_path);

        parent_sha1_str = "0000000000000000000000000000000000000000";
    } else {
        std::string line;
        std::string last_line;
        while (std::getline(ifs, line)) {
            last_line = line;
        }
        ifs.close();
        parent_sha1_str = last_line.substr(41, 40);       // 40 digit (hex)
    }

    return EXIT_SUCCESS;
}

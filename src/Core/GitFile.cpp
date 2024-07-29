#include "GitFile.hpp"

int GitFile::cleate_bin_file()
{
    std::ofstream write_file;
    write_file.open(target_file_path, std::ios::out | std::ios::binary);

    if (write_file.fail()) {
        std::cerr << "couldn't open "<< target_file_path << std::endl;
        return EXIT_FAILURE;
    }

    const char *data = reinterpret_cast<char *>(bin_data.data());
    for (int i = 0; i < bin_data.size(); i++) {
        write_file.write(data + i, sizeof(char));
    }

    write_file.close();

    return EXIT_SUCCESS;
}

int GitFile::calc_sha1_str()
{
    sha1_str = calc_sha1_str_from_u8data(bin_data);
    return EXIT_SUCCESS;
}

int GitFile::cleate_obj()
{
    // file name
    calc_sha1_str();
    std::string sha1_file_name_pre = sha1_str.substr(0, 2);          // upper two digits
    std::string sha1_file_name = sha1_str.substr(2);

    // get current dir
    char current_dir_buf[256];
    GetCurrentDirectory(256, current_dir_buf);
    std::string current_dir_path(current_dir_buf);

    // cleate dir
    const char *dir_path_buf;
    std::string target_path = current_dir_path + "\\.git\\objects\\" + sha1_file_name_pre;
    dir_path_buf = target_path.c_str();

    if (!CreateDirectory(dir_path_buf, NULL)) {
        switch (GetLastError())
        {
        case ERROR_ALREADY_EXISTS:
            break;
        case ERROR_PATH_NOT_FOUND:
            std::cerr << "err: " << dir_path_buf << " path not found." << std::endl;
            return EXIT_FAILURE;
        default:
            std::cerr << "err: " << dir_path_buf << "unknown error." << std::endl;
            return EXIT_FAILURE;
        }
    }

    // cleate file
    std::string out_file_name = ".\\.git\\objects\\" + sha1_file_name_pre + "\\" + sha1_file_name;
    const char *Out = out_file_name.c_str();
    do_compress2(bin_data, Out);   // zlib compress

    return EXIT_SUCCESS;
}

int GitFile::write_text_file(std::ios_base::openmode mode)
{
    std::ofstream write_file;
    write_file.open(target_file_path, mode);

    if (write_file.fail()) {
        std::cerr << "couldn't open "<< target_file_path << std::endl;
        return EXIT_FAILURE;
    }

    write_file << text_data;

    write_file.close();

    return EXIT_SUCCESS;
}

int GitFile::get_bin_data()
{
    std::basic_ifstream<uint8_t> bifs(target_file_path, std::ios::in | std::ios::binary);
    if (bifs.fail()) {
        std::cerr << "couldn't open "<< target_file_path << std::endl;
        return EXIT_FAILURE;
    }

    bin_data = std::vector<uint8_t>(std::istreambuf_iterator<uint8_t>(bifs), std::istreambuf_iterator<uint8_t>());
    bifs.close();
    return EXIT_SUCCESS;
}

int GitFile::get_text_data()
{
    std::ifstream ifs;
    ifs.open(target_file_path, std::ios::in);

    if (ifs.fail()) {
        std::cerr << "couldn't open "<< target_file_path << std::endl;
        return EXIT_FAILURE;
    }

    text_data = std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

    ifs.close();
    return EXIT_SUCCESS;
}


int GitFile::get_obj_data()
{
    const char *In = target_file_path.c_str();
    do_decompress2(In, bin_data);   // zlib compress

    return EXIT_SUCCESS;
}

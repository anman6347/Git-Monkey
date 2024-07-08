#undef UNICODE

#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>

int main(int argc, char **argv)
{
    if (argc > 1) {
        std::cerr << "\"MiniGit init\" must not have args." << std::endl;
        return EXIT_FAILURE;
    }

    char current_dir[255];
    char dir_name_buf[255];
    GetCurrentDirectory(255, current_dir);     // Get current dir

    // initialize
    const char *create_dir_list[] = {
        "\\.git",
        "\\.git\\hooks",
        "\\.git\\info",
        "\\.git\\objects",
        "\\.git\\objects\\info",
        "\\.git\\objects\\pack",
        "\\.git\\refs",
        "\\.git\\refs\\heads",
        "\\.git\\refs\\tags",
    };

    // create .git and associated dirs
    for (int i = 0; i < sizeof(create_dir_list) / sizeof(char*); i++) {
        sprintf(dir_name_buf, "%s%s", current_dir, create_dir_list[i]);
        if (!CreateDirectory(dir_name_buf, NULL)) {
            switch (GetLastError())
            {
            case ERROR_ALREADY_EXISTS:
                std::cerr << "warning: " << create_dir_list[i] <<" is already exists. Ignored it." << std::endl;
                break;
            case ERROR_PATH_NOT_FOUND:
                std::cerr << "err: " << create_dir_list[i] << " path not found." << std::endl;
                return EXIT_FAILURE;
            default:
                std::cerr << "err: " << create_dir_list[i] << "unknown error." << std::endl;
                return EXIT_FAILURE;
            }
        }
    }

    // create config file
    std::ofstream writing_file;
    std::string filename = ".\\.git\\config";
    writing_file.open(filename, std::ios::out);
    std::string writing_text = "[core]\n\trepositoryformatversion = 0\n\tfilemode = false\n\tbare = false\n\tlogallrefupdates = true\n\tsymlinks = false\n\tignorecase = true";
    writing_file << writing_text << std::endl;
    writing_file.close();

    // create description file
    filename = ".\\.git\\description";
    writing_file.open(filename, std::ios::out);
    writing_text = "Unnamed repository; edit this file 'description' to name the repository.";
    writing_file << writing_text << std::endl;
    writing_file.close();

    // create HEAD file
    filename = ".\\.git\\HEAD";
    writing_file.open(filename, std::ios::out);
    writing_text = "ref: refs/heads/master";
    writing_file << writing_text << std::endl;
    writing_file.close();

    return 0;
}

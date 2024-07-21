#ifndef __COMMITOBJ_HPP
#define __COMMITOBJ_HPP

#include <iostream>
#include <stdint.h>
#include <chrono>
#include <string>
#include <vector>
#include "FileTree.hpp"
#include "Compress.hpp"

class CommitObj
{
public:
    FileTree current_ft;
    std::string author;
    std::string e_mail;
    std::string commit_message;
private:
    time_t current_time;
    std::string timezone;
    std::string parent_sha1;
public:
    CommitObj(FileTree ft,
              std::string &_author,
              std::string &_e_mail,
              std::string &_commit_message)
    : current_ft(ft), author(_author), e_mail(_e_mail), commit_message(_commit_message)
    {
        e_mail = "<" + e_mail + ">";
        timezone = "+0900";
    };
    int create_obj_file();
private:
    int get_and_Set_current_time();
    int read_parent_sha1();
};


#endif

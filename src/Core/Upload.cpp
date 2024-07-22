#undef UNICODE

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <json/json.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <Windows.h>


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


/**
 g++ -I .\curl-8.8.0_3-win64-mingw\include\ -I .\jsoncpp-1.9.0\include\ -I .\jsoncpp-1.9.0\build\include\json -L .\curl-8.8.0_3-win64-mingw\lib\ -L.\jsoncpp-1.9.0\build\src\lib_json\ -o push_repo main.cpp -lcurl -lssl -lcrypto -ljsoncpp -lws2_32 -lbcrypt
*/

size_t write_callback(void* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string base64_encode(const std::string& input) {
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Ignore newlines
    BIO_write(bio, input.data(), input.size());
    BIO_flush(bio);

    BUF_MEM* bufferPtr;
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string output(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    return output;
}

std::string create_github_repo(const std::string& token, const std::string& repo_name) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: MyGitHubApp/1.0");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: token " + token).c_str());

        Json::Value json_data;
        json_data["name"] = repo_name.c_str();
        json_data["private"] = false;
        Json::String data_str = Json::writeString(Json::StreamWriterBuilder(), json_data);

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.github.com/user/repos");

        // ssl 無効
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}



// curl -X PUT -H "Authorization: token <token>" "https://api.github.com/repos/anman6347/test_repo/contents/newfile" -d "{\"message\": \"Initial Commit\", \"content\": \"bXkgbmV3IGZpbGUgY29udGVudHM=\"}"

std::string upload_file_to_github(const std::string& token, const std::string& repo_name, const std::string& path, const std::string& content) {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if(curl) {
        struct curl_slist* headers = nullptr;
        headers = curl_slist_append(headers, "User-Agent: MyGitHubApp/1.0");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: token " + token).c_str());

        Json::Value json_data;
        json_data["message"] = "Initial commit"; // Commit message
        json_data["content"] = base64_encode(content).c_str(); // Base64 encoded file content
        Json::String data_str = Json::writeString(Json::StreamWriterBuilder(), json_data);
        std::string url = "https://api.github.com/repos/anman6347/" + repo_name + "/contents/" + path;
        //std::cout << data_str.c_str() << std::endl;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        // ssl 無効
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // 
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data_str.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    return readBuffer;
}

int main(int argc, char **argv) {

    // get token
    
    // set author, e_mail info
    char env_path[255];
    GetModuleFileName(NULL, env_path, 255);
    int index = 254;
    while (env_path[index] != '\\' || index < 0) index--;
    env_path[index] = '\0';
    sprintf(env_path, "%s%s", env_path, "\\.env");
    std::ifstream ifs;
    ifs.open(env_path, std::ios::in);
    if (ifs.fail()) {
        std::cerr << "couldn't open .env file." << std::endl;
        return EXIT_FAILURE;
    }

    std::string tmp;
    std::getline(ifs, tmp);
    std::getline(ifs, tmp);
    std::string token;
    std::getline(ifs, token);

    ifs.close();
    std::string repo_name = "test_repo_1";

    std::cout << "Creating GitHub repository..." << std::endl;
    std::string response = create_github_repo(token, repo_name);
    //std::cout << "Response: " << response << std::endl;

    // Add your local files to be uploaded here
    // Get current dir
    char current_dir[255];
    GetCurrentDirectory(255, current_dir);

    // get child files
    std::string current_dir_s(current_dir);
    std::vector<std::string> child_files = scan_directory(current_dir_s);


    std::vector<std::pair<std::string, std::string>> upload_files;
    for(std::string file_path : child_files) {
        std::string file_name = "";
        for(int i = file_path.size() - 1; i >= 0; i--) {
            if (file_path[i] != '\\') file_name = file_path[i] + file_name;
            else break;
        }
        std::string file_data = "";
        std::ifstream ifs;
        ifs.open(file_path, std::ios::in);
        std::vector<char> buffer_c{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
        ifs.close();
        for (int i = 0; i < buffer_c.size(); i++) file_data += buffer_c[i];
        upload_files.push_back({file_name, file_data});
    }

    // std::vector<std::pair<std::string, std::string>> files = {
    //     {"README.md", "# This is a README file"},
    //     {"main.cpp", "#include <iostream>\n\nint main() { std::cout << \"Hello, world!\" << std::endl; return 0; }"}
    // };

    for (const auto& file : upload_files) {
        //std::cout << "Uploading file " << file.first << "..." << std::endl;
        response = upload_file_to_github(token, repo_name, file.first, file.second);
        //std::cout << "Response: " << response << std::endl;
    }

    //std::cout << "All files uploaded successfully." << std::endl;

    return EXIT_SUCCESS;
}

#include "Crypt.hpp"

NTSTATUS CreateHash(LPCWSTR pszAlgId, std::vector<UCHAR> &input, std::vector<UCHAR> &output) {

    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    BCRYPT_ALG_HANDLE       hAlg = NULL;
    DWORD                   cbData = 0, cbHash = 0, cbHashObject = 0;

    PBYTE                   pbHashObject = NULL;
    PBYTE                   pbHash = NULL;
    PBYTE                   pbOutput = NULL;
    BCRYPT_HASH_HANDLE      hHash = NULL;

    do {
        //open an algorithm handle
        status = BCryptOpenAlgorithmProvider(&hAlg, pszAlgId, NULL, 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //calculate the size of the buffer to hold the hash object
        status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //allocate the hash object on the heap
        pbHashObject = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHashObject);
        if (NULL == pbHashObject) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        //calculate the length of the hash
        status = BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //allocate the hash buffer on the heap
        pbOutput = (PBYTE)HeapAlloc(GetProcessHeap(), 0, cbHash);
        if (NULL == pbOutput) {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        //create a hash
        status = BCryptCreateHash(hAlg, &hHash, pbHashObject, cbHashObject, NULL, 0, 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //hash some data
        status = BCryptHashData(hHash, input.data(), input.size(), 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //close the hash
        status = BCryptFinishHash(hHash, pbOutput, cbHash, 0);
        if (!NT_SUCCESS(status)) {
            break;
        }

        output.resize(cbHash);
        PBYTE p = pbOutput;
        for (size_t i = 0; i < cbHash; i++) {
            output.at(i) = *p;
            p++;
        }

        if (hAlg) {
            BCryptCloseAlgorithmProvider(hAlg, 0);
        }

        if (hHash) {
            BCryptDestroyHash(hHash);
        }

        if (pbHashObject) {
            HeapFree(GetProcessHeap(), 0, pbHashObject);
        }

        if (pbOutput) {
            HeapFree(GetProcessHeap(), 0, pbOutput);
        }

    } while (0);

    return status;

}

// calc sha1 and return it in str form  (not binary mode)
std::string calc_blob_sha1_str(std::string &file_path)
{
    std::string res;
    std::ifstream ifs;
    ifs.open(file_path, std::ios::in);
    std::vector<char> buffer_c{std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>()};
    ifs.close();
    std::vector<unsigned char> buffer;
    buffer.insert(buffer.end(), buffer_c.begin(), buffer_c.end());
    buffer.insert(buffer.begin(), static_cast<uint8_t>('\0'));
    std::string file_size_str = std::to_string(buffer_c.size());
    for (int i = file_size_str.size() - 1; i >= 0; i--) {
        buffer.insert(buffer.begin(), static_cast<uint8_t>(file_size_str[i]));
    }
    buffer.insert(buffer.begin(), static_cast<uint8_t>(' '));
    buffer.insert(buffer.begin(), static_cast<uint8_t>('b'));
    buffer.insert(buffer.begin(), static_cast<uint8_t>('o'));
    buffer.insert(buffer.begin(), static_cast<uint8_t>('l'));
    buffer.insert(buffer.begin(), static_cast<uint8_t>('b'));

    NTSTATUS status = STATUS_UNSUCCESSFUL;
    std::vector<UCHAR> hash1;
    status = CreateHash(BCRYPT_SHA1_ALGORITHM, buffer, hash1);
    if (!NT_SUCCESS(status)) {
        std::cout << "Error: " << status << std::endl;
        return res;
    }

    std::ostringstream ss;
    for (size_t i = 0; i < hash1.size(); i++) {
        ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (int)hash1.at(i);      // 1 byte => 2-digit hexadecimal number
    }

    res = ss.str();
    return res;
}


// calc sha-1 (str) from u8 data
std::string calc_sha1_str_from_u8data(std::vector<uint8_t> &raw_data)
{
    std::string res;
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    std::vector<UCHAR> hash1;
    status = CreateHash(BCRYPT_SHA1_ALGORITHM, raw_data, hash1);
    if (!NT_SUCCESS(status)) {
        std::cout << "Error: " << status << std::endl;
        return res;
    }

    std::ostringstream ss;
    for (size_t i = 0; i < hash1.size(); i++) {
        ss << std::setfill('0') << std::right << std::setw(2) << std::hex << (int)hash1.at(i);      // 1 byte => 2-digit hexadecimal number
    }

    res = ss.str();
    return res;
}


// calc sha-1 (big endian) from u8 data
int calc_sha1_from_u8data(uint8_t dst[], std::vector<uint8_t> &raw_data)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    std::vector<UCHAR> hash1;
    status = CreateHash(BCRYPT_SHA1_ALGORITHM, raw_data, hash1);
    if (!NT_SUCCESS(status)) {
        std::cout << "Error: " << status << std::endl;
        return EXIT_FAILURE;
    }

    for (int i = 19; i >= 0; i--) {
        dst[i] = hash1.at(19 - i);
    }

    return EXIT_SUCCESS;
}
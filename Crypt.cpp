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

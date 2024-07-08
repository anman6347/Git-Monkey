#ifndef __CRYPTO_HPP
#define __CRYPTO_HPP

#include <iostream>
#include <string>
#include <vector>
#include <windows.h>
#include <bcrypt.h>


#define NT_SUCCESS(Status)          (((NTSTATUS)(Status)) >= 0)
#define STATUS_UNSUCCESSFUL         ((NTSTATUS)0xC0000001L)


// calc sha1
NTSTATUS CreateHash(LPCWSTR pszAlgId, std::vector<UCHAR> &input, std::vector<UCHAR> &output);


#endif

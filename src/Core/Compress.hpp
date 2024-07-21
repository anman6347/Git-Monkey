#ifndef __COMPRESS_HPP
#define __COMPRESS_HPP

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <zlib.h>

#define INBUFSIZ   65536                 // 入力バッファサイズ（任意）
#define OUTBUFSIZ  65536                 // 出力バッファサイズ（任意）

// 圧縮
void do_compress(const char *in_file, const char *out_file);
// 展開（復元）
void do_decompress(char *in_file, char *out_file);
// 圧縮
void do_compress2(std::vector<unsigned char> &in, const char *out_file);

#endif

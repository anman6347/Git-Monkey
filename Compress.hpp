#include <cstdio>
#include <cstdlib>
#include <zlib.h>

#define INBUFSIZ   65536                 // 入力バッファサイズ（任意）
#define OUTBUFSIZ  65536                 // 出力バッファサイズ（任意）

// 圧縮
void do_compress(const char *in_file, const char *out_file){
        unsigned char inbuf[INBUFSIZ];           // 入力バッファ
        unsigned char outbuf[OUTBUFSIZ];         // 出力バッファ
        char err_msg[250];
        FILE *fin, *fout;       // 入力・出力ファイル
        z_stream z;             // ライブラリとやりとりするための構造体
        int count, flush, status;
        if (!(fin = fopen(in_file, "rb"))) return;
        if (!(fout = fopen(out_file, "wb"))) return;

        // すべてのメモリ管理をライブラリに任せる
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;

        // 初期化  第2引数は圧縮の度合。0～9 の範囲の整数で，0 は無圧縮, Z_DEFAULT_COMPRESSION (= 6) が標準
        if (deflateInit(&z, Z_DEFAULT_COMPRESSION) != Z_OK) {
                sprintf(err_msg, "圧縮初期化エラー（deflateInit関数）: %s\n", (z.msg) ? z.msg : "???");
                fclose(fin);
                fclose(fout);
                return;
        }

        z.avail_in = 0;                         // 入力バッファ中のデータのバイト数
        z.next_out = outbuf;            // 出力ポインタ
        z.avail_out = OUTBUFSIZ;        // 出力バッファのサイズ

        // 通常は deflate() の第2引数は Z_NO_FLUSH にして呼び出す
        flush = Z_NO_FLUSH;

        while (1) {
                if (z.avail_in == 0) {  // 入力が尽きれば
                        z.next_in = inbuf;      // 入力ポインタを入力バッファの先頭に
                        z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); // データを読み込む

                        // 入力が最後になったら deflate() の第2引数は Z_FINISH にする
                        if (z.avail_in < INBUFSIZ) flush = Z_FINISH;
                }
                status = deflate(&z, flush); // 圧縮する
                if (status == Z_STREAM_END) break; // 完了
                if (status != Z_OK) {   // エラー
                        sprintf(err_msg, "圧縮エラー（deflate関数）: %s\n", (z.msg) ? z.msg : "???");
                        fclose(fin);
                        fclose(fout);
                        return;
                }
                if (z.avail_out == 0) { // 出力バッファが尽きれば
                        // まとめて書き出す
                        if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                                fclose(fin);
                                fclose(fout);
                                return;
                        }
                        z.next_out = outbuf; // 出力バッファ残量を元に戻す
                        z.avail_out = OUTBUFSIZ; // 出力ポインタを元に戻す
                }
        }

        // 残りを吐き出す
        if ((count = OUTBUFSIZ - z.avail_out) != 0) {
                if (fwrite(outbuf, 1, count, fout) != count) {
                        fclose(fin);
                        fclose(fout);
                        return;
                }
        }

        // 後始末
        if (deflateEnd(&z) != Z_OK) {
                sprintf(err_msg, "圧縮完了エラー（deflateEnd関数）: %s\n", (z.msg) ? z.msg : "???");
                fclose(fin);
                fclose(fout);
                return;
        }
        fclose(fin);
        fclose(fout);
        return;
}
// 展開（復元）
void do_decompress(char *in_file, char *out_file){
        unsigned char inbuf[INBUFSIZ];           // 入力バッファ
        unsigned char outbuf[OUTBUFSIZ];         // 出力バッファ
        char err_msg[250];
        FILE *fin, *fout;       // 入力・出力ファイル
        int count, status;
        z_stream z;             // ライブラリとやりとりするための構造体
        if (!(fin = fopen(in_file, "rb"))) return;
        if (!(fout = fopen(out_file, "wb"))) return;

        // すべてのメモリ管理をライブラリに任せる
        z.zalloc = Z_NULL;
        z.zfree = Z_NULL;
        z.opaque = Z_NULL;

        // 初期化
        z.next_in = Z_NULL;
        z.avail_in = 0;
        if (inflateInit(&z) != Z_OK) {
                sprintf(err_msg, "展開初期化エラー（inflateInit関数）: %s\n", (z.msg) ? z.msg : "???");
                fclose(fin);
                fclose(fout);
                return;
        }

        z.next_out = outbuf;            // 出力ポインタ
        z.avail_out = OUTBUFSIZ;        // 出力バッファ残量
        status = Z_OK;

        while (status != Z_STREAM_END) {
                if (z.avail_in == 0) {  // 入力残量がゼロになれば
                        z.next_in = inbuf;      // 入力ポインタを元に戻す
                        z.avail_in = fread(inbuf, 1, INBUFSIZ, fin); // データを読む
                }
                status = inflate(&z, Z_NO_FLUSH); // 展開
                if (status == Z_STREAM_END) break; // 完了
                if (status != Z_OK) {   // エラー
                        sprintf(err_msg, "展開エラー（inflate関数）: %s\n", (z.msg) ? z.msg : "???");
                        fclose(fin);
                        fclose(fout);
                        return;
                }
                if (z.avail_out == 0) { // 出力バッファが尽きれば
                        // まとめて書き出す
                        if (fwrite(outbuf, 1, OUTBUFSIZ, fout) != OUTBUFSIZ) {
                                fclose(fin);
                                fclose(fout);
                                return;
                        }
                        z.next_out = outbuf; // 出力ポインタを元に戻す
                        z.avail_out = OUTBUFSIZ; // 出力バッファ残量を元に戻す
                }
        }

        // 残りを吐き出す
        if ((count = OUTBUFSIZ - z.avail_out) != 0) {
                if (fwrite(outbuf, 1, count, fout) != count) {
                        fclose(fin);
                        fclose(fout);
                        return;
                }
        }

        // 後始末
        if (inflateEnd(&z) != Z_OK) {
                sprintf(err_msg, "展開終了エラー（inflateEnd関数）: %s\n", (z.msg) ? z.msg : "???");
                fclose(fin);
                fclose(fout);
                return;
        }
        fclose(fin);
        fclose(fout);
        return;
}

#include <iostream>
#include <string>
#undef UNICODE
#include <Windows.h>

int main(int argc, char **argv) {

    if (argc <= 1) {
        std::cerr << "wrapper error" << std::endl;
    }

    if( !SetCurrentDirectory(argv[1])) {
        printf("SetCurrentDirectory failed (%d)\n", GetLastError());
        return EXIT_FAILURE;
    }

    // std::cout << argv[1] << std::endl;
    // Sleep(3000);

    char commandLine[256] = {' '};          // e.g. MiniGit add -A => commandLine = "Add -A    "
    int i = 2;
    int index = 0;
    while(argv[i]) {
        for(int j = 0; argv[i][j] != '\0'; j++) {
            commandLine[index] = argv[i][j];
            index++;
        }
        commandLine[index] = ' ';
        index++;
        i++;
    }

    HANDLE childProcess = NULL;
    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi = {};
    if(!CreateProcess(
            NULL,   // The first white space–delimited token of the command line specifies the module name.
                    // If the file name does not contain a directory path, the system searches for the executable file
                    // in the directory from which the application loaded.
            commandLine,
            NULL,   //プロセスのセキュリティー記述子
            NULL,   //スレッドのセキュリティー記述子
            FALSE,  //ハンドルを継承しない
            0,      //作成フラグ
            NULL,   //環境変数は引き継ぐ
            NULL,   //カレントディレクトリーは同じ
            &si,
            &pi)) {
        fprintf(stderr, "command executing error\n");
        return -1;
    }
    // 子プロセス起動成功
    childProcess = pi.hProcess;

    // 不要なスレッドハンドルをクローズする
    if(!CloseHandle(pi.hThread)) {
        //printError("CloseHandle(hThread)");
        return -1;
    }

    //printf("child processId=%d\n", pi.dwProcessId);

    // 子プロセスの終了待ち
    DWORD r = WaitForSingleObject(childProcess, INFINITE);
    switch(r) {
        case WAIT_FAILED:
            //printError("wait result=WAIT_FAILED");
            return -1;
        case WAIT_ABANDONED:
            //printf("wait result=WAIT_ABANDONED\n");
            return -1;
        case WAIT_OBJECT_0:  //正常終了
            //printf("wait result=WAIT_OBJECT_0\n");
            break;
        case WAIT_TIMEOUT:
            //printf("wait result=WAIT_TIMEOUT\n");
            return -1;
        default:
            //printf("wait result=%d\n", r);
            return -1;
    }

    // 子プロセスの終了コードを取得
    DWORD exitCode;
    GetExitCodeProcess(childProcess, &exitCode);

    return exitCode;
}

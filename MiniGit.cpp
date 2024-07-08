#undef UNICODE

#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <windows.h>


/* List of builtin commands, followed by their corresponding functions. */
const char* builtin_str[] = {
    "init",
    "add",
    "commit",
    "reset",
    "log",
    "status"
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}


HANDLE childProcess = NULL;

int cmd_launch(char **args) {
    char commandLine[256] = {' '};          // e.g. MiniGit add -A => commandLine = "Add -A    "
    int i = 1;
    int index = 0;
    while(args[i]) {
        for(int j = 0; args[i][j] != '\0'; j++) {
            commandLine[index] = args[i][j];
            index++;
        }
        commandLine[index] = ' ';
        index++;
        i++;
    }
    commandLine[0] = toupper(commandLine[0]);


    // e.g. "add" -> "Add.exe"
    //char cmd[64];
    //sprintf(cmd, "%s%s", args[1], ".exe\0");
    //sprintf(cmd, "%s%s", args[1], "\0");
    //cmd[0] = toupper(cmd[0]);
    //std::cout << cmd << std::endl;

    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi = {};
    if(!CreateProcess(
           NULL,   // PATH から commandLine にあるコマンドを探す
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
    if(!GetExitCodeProcess(childProcess, &exitCode)) {
        //printError("GetExitCodeProcess");
        return -1;
    }
    //printf("exitCode=%d/%x\n", exitCode, exitCode);

    return EXIT_SUCCESS;
}

int cmd_execute(int argc, char** args) {
    if(argc <= 1) {
        // An empty command was entered.
        return EXIT_FAILURE;
    }

    for(int i = 0; i < num_builtins(); i++) {
        if(strcmp(args[1], builtin_str[i]) == 0) {
            return cmd_launch(args);
        }
    }

    std::cerr <<"\"" << args[1] << "\"" << " is not supported" << std::endl;
    return EXIT_FAILURE;
}


int main(int argc, char **argv) {
    return cmd_execute(argc, argv);
}

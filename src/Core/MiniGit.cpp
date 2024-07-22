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
    "status",
    "upload",
};

// return the number of built-in commands
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

    //std::cout << commandLine << std::endl;
    //Sleep(3000);

    /* Get full path of the currently executing executable
    char MiniGitPath[256] = {'\0'};
    GetModuleFileName(
        NULL,                                           // Get full path of the currently executing executable
        MiniGitPath,                                    // buffer
        sizeof(MiniGitPath) / sizeof(MiniGitPath[0])    // buffer size
    );
    std::cout << MiniGitPath << std::endl;
    */

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

char nargv[256];
int main(int argc, char **argv) {

    // Re-add double quotation marks
    int i = 0, j = 0;
    bool need_dq = false;
    while (argv[i]) {
        for (j = 0; argv[i][j] != '\0'; j++) {
            if (argv[i][j] == ' ') {
                need_dq = true;
            }
        }

        if (need_dq) {
            //char nargv[256];
            nargv[0] = '"';
            strcpy(nargv + 1, argv[i]);
            nargv[j + 1] = '"';
            argv[i] = nargv;
        }
        need_dq = false;
        i++;
    }
    return cmd_execute(argc, argv);
}

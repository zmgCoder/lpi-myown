#define _XOPEN_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
    用例：
        echo $USER | tee file.txt # 输出本机登录用户名到标准输出并写入文件file.txt
    默认情况下，tee将会覆盖传递的文件内容。传递-a参数会使得tee追加内容到文件中
    支持的命令行参数：
        -a  追加文件内容
        -h  展示帮助信息并退出

*/


void helpAndLeave(const char* progname, int status);
void failure(const char *fCall);

int main(int argc, char* argv[])
{
    const int BUF_SIZE = 1024;
    const int MAX_OUT_FILES = 128;
    
    int opt;
    bool append = false;

    int fd, flags;
    int fds[MAX_OUT_FILES];   // 需要打开的文件描述符数组
    mode_t mode;
    char buf[BUF_SIZE + 1];   
    ssize_t numRead;

    while ( (opt = getopt(argc, argv, "+a")) != -1 )
    {
        switch (opt)
        {
        case 'a':  append = true;  break;
        case '?': helpAndLeave(argv[0], EXIT_FAILURE);  break;
        case 'h': helpAndLeave(argv[0], EXIT_SUCCESS);  break;
            
        }
    }
    if (optind >= argc)
        helpAndLeave(argv[0], EXIT_FAILURE);
    
    // stdin重定向
    flags = O_CREAT | O_WRONLY;
    mode = S_IRUSR | S_IWUSR;  // rw-------

    if (append)
        flags |= O_APPEND;
    else
        flags |= O_TRUNC;
    
    int numFiles = 0;
    for (int i = optind; i < argc; ++i)
    {
        fds[i - optind] = fd = open(argv[i], flags, mode);
        if (fd == -1)
            failure("open");
        ++numFiles;
    }

    while ( (numRead = read(STDIN_FILENO, buf, BUF_SIZE)) > 0 )
    {
        if ( write(STDOUT_FILENO, buf, numRead) != numRead )
            failure("write");

        for ( int i = 0; i < numFiles; ++i )
        {
            if ( write(fds[i], buf, numRead) != numRead )
                failure("write");
        }

        if ( numRead == -1 )
            failure("read");
    }
    for ( int i = 0; i < numFiles; ++i )
    {
        if ( close(fds[i]) == -1 )
            failure("close");
    }

    return 0;
}

void
helpAndLeave(const char* progname, int status)
{
    fprintf(stderr, "Usage: %s [-a] <file1> <file2> ... <fileN>\n", progname);
    exit(status);
}

void 
failure(const char *fCall)
{
    perror(fCall);
    exit(EXIT_FAILURE);
}
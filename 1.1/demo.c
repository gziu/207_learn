#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

extern int read_line(int, void *, int);

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Please input file name!\n");
        exit(1);
    }

    int fd;
    int to_fd;
    char buffer[1024];

    if ((fd = open(argv[1], O_RDONLY | O_CREAT , 0655)) < 0)
    {
        perror("open file error!");
        exit(1);
    }


    if ((to_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0655)) < 0)
    {
        perror("open file error!");
        exit(1);
    }
#if 0
    for (int i = 0; i < 3; i++)
    {
        memset(buffer, 0, sizeof(buffer));
        printf("Please input data:\n");
        scanf("%s", buffer);

        if (write(fd, buffer, strlen(buffer)) < 0)
        {
            perror("write data error!");
            exit(1);
        }

        write(fd, "\n", 1);
    }
#endif
#if 0
    lseek(fd, 0, SEEK_SET);

    memset(buffer, 0, sizeof(buffer));

    while (read_line(fd, buffer, sizeof(buffer) - 1) != 0) //遇到\n或者缓冲区最大值时，停止读取数据
    {
        printf("%s\n", buffer);
        memset(buffer, 0, sizeof(buffer));
    }
    
#endif

    while(read_line(fd, buffer, sizeof(buffer) - 1) != 0)
    {
        write(to_fd, buffer, strlen(buffer));
        //write(to_fd, "\n", 1);
        memset(buffer, 0, sizeof(buffer));
    }

    close(fd);

    return 0;
}
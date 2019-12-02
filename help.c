#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#define FIFO "fifo.txt"


int main()
{
	mknod(FIFO, S_IFIFO | 0666, 0);
	int fd = open(FIFO, O_WRONLY);
	char buf[20];
	strcpy(buf, "*in.txt*out.txt*");
	printf("Trying to connect...\n");
	write(fd, buf, 20);
	mknod("in.txt", S_IFIFO | 0666, 0);
	int fd_in = open("in.txt", O_WRONLY);
	mknod("out.txt", S_IFIFO | 0666, 0);
	int fd_out = open("out.txt", O_RDONLY);
	char msg[20];
	read(fd_out, msg, 20);
	if(strcmp(msg, "No free sockets") ==0)
	{
		printf("No free sockets\n");
		return 0;
	}
	else if (strcmp(msg, "Connected") == 0)
		printf("Connected\n");

	while(1)
	{
		char request[20];
		scanf("%s", request);
		if(strcmp(request, "file1")==0)
		{
			char send_req[20];
			strcpy(send_req, "*file1*");
			write(fd_in, send_req, 20);
			char fd_receive[20];
			read(fd_out, fd_receive, 20);
			mknod(fd_receive, S_IFIFO|0666, 0);
		        int read_fd = open(fd_receive, O_RDONLY);
		        char info[1000];
		        printf("Waiting...\n");
		        int size = read(read_fd, info, sizeof(info)-1);
		        info[size] = '\0';
		        printf("Got: %s\n", info);
		}
		else if(strcmp(request, "file2")==0)
                {
                        char send_req[20];
                        strcpy(send_req, "*file2*");
                        write(fd_in, send_req, 20);
                        char fd_receive[20];
                        read(fd_out, fd_receive, 20);
                        mknod(fd_receive, S_IFIFO|0666, 0);
                        int read_fd = open(fd_receive, O_RDONLY);
                        char info[1000];
                        printf("Waiting...\n");
                        int size = read(read_fd, info, sizeof(info)-1);
                        info[size] = '\0';
                        printf("Got: %s\n", info);
                }
		else if(strcmp(request, "file3")==0)
                {
                        char send_req[20];
                        strcpy(send_req, "*file3*");
                        write(fd_in, send_req, 20);
                        char fd_receive[20];
                        read(fd_out, fd_receive, 20);
                        mknod(fd_receive, S_IFIFO|0666, 0);
                        int read_fd = open(fd_receive, O_RDONLY);
                        char info[1000];
                        printf("Waiting...\n");
                        int size = read(read_fd, info, sizeof(info)-1);
                        info[size] = '\0';
                        printf("Got: %s\n", info);
                }
		else if(strcmp(request, "exit") ==0)
		{
			char send_req[20];
                        strcpy(send_req, "*exit*");
                        write(fd_in, send_req, 20);
			return 0;
		}

	}
}

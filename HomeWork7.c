#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/select.h>
#include <poll.h>
#include <string.h>

#define th_sz 5
#define Server_FIFO "fifo.txt"
#define cl_sz 2
#define file1name "file1.txt"
#define file2name "file2.txt"
#define file3name "file3.txt"
pthread_t Threads_Id[th_sz];


typedef struct
{
	int client_server_fd;
	int server_client_fd;
} communication;

typedef struct
{
	int client_numb;
	int request;
} Req;

communication* arr_com;
int* free_to_commun;    //свободные места для клиентов
struct pollfd clients_fds[cl_sz]; //дискрипторы клиентов
Req arr_req[cl_sz];
int Clients_Requests;


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;


void *Transition(void* args)
{
	int cl_numb;
	int req;
	while(1)
	{
		pthread_mutex_lock(&mutex);
		if(Clients_Requests > 0)
		{
			cl_numb = arr_req[0].client_numb;
			req = arr_req[0].request;
			for(int i=0; i<Clients_Requests; i++)
				arr_req[i] = arr_req[i+1];
			Clients_Requests--;
			pthread_mutex_unlock(&mutex);
			char FNM[10];
			switch(req)
			{
				case 1: strcpy(FNM, file1name);break;
				case 2: strcpy(FNM, file2name);break;
				case 3: strcpy(FNM, file3name);break;
			}
			int file_descriptor = open(FNM, O_RDONLY);
			char send[1000];
			read(file_descriptor, send, sizeof(send));
			printf("%s\n", send);
			char numb[5] = itoa(cl_numb);
			printf("%s\n", numb);
			sleep(20);

		}
		else
		{
			pthread_mutex_unlock(&mutex);
			sleep(1);
		}
	}

}


int main()
{
	Clients_Requests=0;
	char buf[100];
	for(int i=0; i<th_sz; i++)
		pthread_create(&Threads_Id[i], NULL, Transition, NULL);
	mknod(Server_FIFO, S_IFIFO | 0666, 0);
	int server_fd = open(Server_FIFO, O_RDONLY);
	
	arr_com = (communication*)malloc(sizeof(communication) * cl_sz);
	free_to_commun = (int*)malloc(sizeof(int) * cl_sz);
	for (int i=0; i<cl_sz; i++)
	{
		arr_req[i].client_numb = -1;
		free_to_commun[i] = 0;
	}



	struct pollfd Main[1];
        int timeout = 0;
        Main[0].fd = server_fd;
        Main[0].events = 0 | POLLIN;


	while(1)
	{
		int ret_Main = poll(Main, 1, timeout);
		if(ret_Main >0)
                {
			read(server_fd, buf, sizeof(buf));
			int end = 0;
			int i=1;
			printf("#1\n");
			while(!end)
			{
				int k=0;
				char forward_fd_name[100];
				while(buf[i]!='*')
					forward_fd_name[k++] = buf[i++];
				i++;
				forward_fd_name[k]='\0';
				k=0;
				char backward_fd_name[100];
				while(buf[i]!='*')
					backward_fd_name[k++] = buf[i++];
				backward_fd_name[k] = '\0';
				i++;
				
				
				printf("%s, %s\n", forward_fd_name, backward_fd_name);
				communication new_client;
				mknod(forward_fd_name, S_IFIFO | 0666, 0);
			        new_client.client_server_fd = open(forward_fd_name, O_RDONLY);
				mknod(backward_fd_name, S_IFIFO | 0666, 0);
				new_client.server_client_fd = open(backward_fd_name, O_WRONLY);
				int found = 0;
				for(int j=0; j<cl_sz; j++)
				{
					if(free_to_commun[j] == 0)
					{
						printf("%d\n", j);
						found = 1;
						free_to_commun[j] = 1;
						arr_com[j] = new_client;
						clients_fds[j].fd = new_client.client_server_fd;
						clients_fds[j].events = 0|POLLIN;
						break;
					}
				}
				if(!found)
				{
					printf("NO!\n");
					char response_buf[100];
					strcpy(response_buf, "No free sckets");
					write(new_client.server_client_fd, response_buf, strlen(response_buf)+1);
					close(new_client.client_server_fd);
					close(new_client.server_client_fd);
				}

				

				if(strlen(buf) == i)
					end = 1;
				else
					i++;
			}
                }
	        int ret_Clients = poll(clients_fds, cl_sz, timeout);
		if (ret_Clients > 0)
		{
			for(int cl_numb=0; cl_numb < cl_sz; cl_numb++)	
			{
				if(clients_fds[cl_numb].revents & POLLIN)
				{
					char Message[100];
					read(clients_fds[cl_numb].fd, Message, 100);
					printf("%s, %d\n", Message, strlen(Message));
					int n = 1;
						char token[100];	
						while(Message[n] != '*')
						{
							printf("GG\n");
							token[n-1] = Message[n];
							n++;
						}
						token[n-1] = '\0';
						if(strcmp(token, "exit") == 0)
						{
							free_to_commun[cl_numb] = 0;
							close(arr_com[cl_numb].client_server_fd);
							close(arr_com[cl_numb].server_client_fd);
							clients_fds[cl_numb].fd=0;
						}
						else if(strcmp(token, "file1"))
						{
							pthread_mutex_lock(&mutex);
							arr_req[Clients_Requests].client_numb = cl_numb;
							arr_req[Clients_Requests].request = 1;
							Clients_Requests++;
							pthread_mutex_unlock(&mutex);
						}
						else if(strcmp(token, "file2"))
                                                {
                                                        pthread_mutex_lock(&mutex);
                                                        arr_req[Clients_Requests].client_numb = cl_numb;
                                                        arr_req[Clients_Requests].request = 2;
                                                        Clients_Requests++;
                                                        pthread_mutex_unlock(&mutex);
                                                }
						else if(strcmp(token, "file3"))
                                                {
                                                        pthread_mutex_lock(&mutex);
                                                        arr_req[Clients_Requests].client_numb = cl_numb;
                                                        arr_req[Clients_Requests].request = 3;
                                                        Clients_Requests++;
                                                        pthread_mutex_unlock(&mutex);
                                                }

					}
				
			}
		}
	}
}
